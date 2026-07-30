#include "TimeService.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <esp_sntp.h>

#include "Config.h"
#include "DiagnosticsService.h"
#include "Oled.h"
#include "Version.h"

namespace
{
    WiFiManager wifiManager;
    tm currentLocalTime = {};
    time_t lastRenderedSecond = 0;
    bool synchronized = false;
    bool changed = false;
    bool ntpConfigured = false;
    wl_status_t previousWifiStatus = WL_IDLE_STATUS;
    uint32_t wifiDisconnectedAt = 0;
    uint32_t lastReconnectAttemptAt = 0;
    bool wifiStackReset = false;
    constexpr time_t MINIMUM_VALID_EPOCH = 1704067200; // 2024-01-01 UTC

    void configureNtp()
    {
        sntp_set_time_sync_notification_cb([](timeval*) {
            DiagnosticsService::recordTimeSync();
        });
        configTzTime(
            Config::LOCAL_TIMEZONE,
            Config::NTP_SERVER_PRIMARY,
            Config::NTP_SERVER_SECONDARY);
        ntpConfigured = true;
        Serial.printf("[TIME] NTP configured for local timezone; IP: %s\r\n", WiFi.localIP().toString().c_str());
    }
}

void TimeService::begin()
{
    if (!Config::ENABLE_WIFI)
    {
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(Product::HOSTNAME);
    WiFi.setAutoReconnect(true);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setConfigPortalTimeout(Config::WIFI_PORTAL_TIMEOUT_SECONDS);
    wifiManager.setConnectTimeout(15);
    wifiManager.setAPCallback([](WiFiManager*) {
        Oled::status("WiFi Setup", Product::SETUP_AP_NAME);
        Serial.println("[WIFI] Configuration portal started at 192.168.4.1");
    });
    Oled::status("Connecting WiFi", Product::SETUP_AP_NAME);
    wifiManager.autoConnect(Product::SETUP_AP_NAME);
    previousWifiStatus = WiFi.status();
    wifiDisconnectedAt = previousWifiStatus == WL_CONNECTED ? 0 : millis();
    lastReconnectAttemptAt = 0;
    wifiStackReset = false;
}

void TimeService::update()
{
    changed = false;

    if (!Config::ENABLE_WIFI)
    {
        return;
    }

    wifiManager.process();
    const wl_status_t status = WiFi.status();

    if (status != previousWifiStatus)
    {
        if (status == WL_CONNECTED)
        {
            Serial.printf("[WIFI] Connected: %s\r\n", WiFi.localIP().toString().c_str());
        }
        else if (previousWifiStatus == WL_CONNECTED)
        {
            Serial.println("[WIFI] Connection lost; reconnecting");
        }
        previousWifiStatus = status;
    }

    if (status == WL_CONNECTED)
    {
        wifiDisconnectedAt = 0;
        wifiStackReset = false;
    }
    else
    {
        const uint32_t nowMs = millis();
        if (wifiDisconnectedAt == 0)
        {
            wifiDisconnectedAt = nowMs;
            lastReconnectAttemptAt = 0;
        }

        if (lastReconnectAttemptAt == 0 || nowMs - lastReconnectAttemptAt >= Config::WIFI_RECONNECT_INTERVAL_MS)
        {
            lastReconnectAttemptAt = nowMs;
            Serial.printf("[WIFI] Reconnect attempt; status %d\r\n", static_cast<int>(status));
            WiFi.reconnect();
        }

        const uint32_t disconnectedFor = nowMs - wifiDisconnectedAt;
        if (!wifiStackReset && disconnectedFor >= Config::WIFI_STACK_RESET_AFTER_MS)
        {
            wifiStackReset = true;
            Serial.println("[WIFI] Resetting station interface after prolonged disconnect");
            WiFi.disconnect(false, false);
            WiFi.mode(WIFI_OFF);
            WiFi.mode(WIFI_STA);
            WiFi.setHostname(Product::HOSTNAME);
            WiFi.reconnect();
        }
        else if (disconnectedFor >= Config::WIFI_REBOOT_AFTER_MS)
        {
            Serial.println("[WIFI] Rebooting after unrecoverable disconnect");
            ESP.restart();
        }
    }

    if (status == WL_CONNECTED && !ntpConfigured)
    {
        configureNtp();
    }

    const time_t now = time(nullptr);
    if (now < MINIMUM_VALID_EPOCH || now == lastRenderedSecond)
    {
        return;
    }

    if (localtime_r(&now, &currentLocalTime) == nullptr)
    {
        return;
    }

    lastRenderedSecond = now;
    changed = true;
    if (!synchronized)
    {
        synchronized = true;
        Serial.println("[TIME] Local time synchronized");
    }
}

bool TimeService::isSynchronized()
{
    return synchronized;
}

bool TimeService::hasChanged()
{
    return changed;
}

const tm& TimeService::localTime()
{
    return currentLocalTime;
}
