#include "TimeService.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "Config.h"
#include "Oled.h"

namespace
{
    WiFiManager wifiManager;
    tm currentLocalTime = {};
    time_t lastRenderedSecond = 0;
    bool synchronized = false;
    bool changed = false;
    bool ntpConfigured = false;
    wl_status_t previousWifiStatus = WL_IDLE_STATUS;
    constexpr time_t MINIMUM_VALID_EPOCH = 1704067200; // 2024-01-01 UTC

    void configureNtp()
    {
        configTzTime(
            Config::ISRAEL_TIMEZONE,
            Config::NTP_SERVER_PRIMARY,
            Config::NTP_SERVER_SECONDARY);
        ntpConfigured = true;
        Serial.printf("[TIME] NTP configured for Israel; IP: %s\r\n", WiFi.localIP().toString().c_str());
    }
}

void TimeService::begin()
{
    if (!Config::ENABLE_WIFI)
    {
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setConfigPortalTimeout(Config::WIFI_PORTAL_TIMEOUT_SECONDS);
    wifiManager.setConnectTimeout(15);
    wifiManager.setAPCallback([](WiFiManager*) {
        Oled::status("WiFi Setup", Config::WIFI_PORTAL_NAME);
        Serial.println("[WIFI] Configuration portal started at 192.168.4.1");
    });
    Oled::status("Connecting WiFi", Config::WIFI_PORTAL_NAME);
    wifiManager.autoConnect(Config::WIFI_PORTAL_NAME, Config::WIFI_PORTAL_PASSWORD);
    previousWifiStatus = WiFi.status();
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
        Serial.println("[TIME] Israel local time synchronized");
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
