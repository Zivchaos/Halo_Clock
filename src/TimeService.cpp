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

    constexpr time_t MINIMUM_VALID_EPOCH = 1704067200; // 2024-01-01 UTC
}

void TimeService::begin()
{
    if (!Config::ENABLE_WIFI)
    {
        return;
    }

    WiFi.mode(WIFI_STA);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setConfigPortalTimeout(Config::WIFI_PORTAL_TIMEOUT_SECONDS);
    wifiManager.setConnectTimeout(15);
    wifiManager.autoConnect(Config::WIFI_PORTAL_NAME);

    Oled::status("Connecting WiFi", Config::WIFI_PORTAL_NAME);
}

void TimeService::update()
{
    changed = false;

    if (!Config::ENABLE_WIFI)
    {
        return;
    }

    wifiManager.process();

    if (WiFi.status() == WL_CONNECTED && !ntpConfigured)
    {
        configTzTime(
            Config::ISRAEL_TIMEZONE,
            Config::NTP_SERVER_PRIMARY,
            Config::NTP_SERVER_SECONDARY);
        ntpConfigured = true;
        Serial.print("[TIME] NTP configured; IP: ");
        Serial.println(WiFi.localIP());
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
    synchronized = true;
    changed = true;
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
