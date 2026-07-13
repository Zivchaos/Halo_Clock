#pragma once

#include <Arduino.h>

namespace Config
{
    constexpr uint32_t SERIAL_BAUD = 115200;

    constexpr bool SERIAL_LOGGING = true;
    constexpr bool SHOW_SPLASH = true;
    constexpr bool ENABLE_WIFI = true;
    constexpr bool ENABLE_OLED = true;
    constexpr bool ENABLE_LED_RING = true;

    constexpr const char* WIFI_PORTAL_NAME = "HALO Clock Setup";
    constexpr const char* WIFI_PORTAL_PASSWORD = "halo-clock";
    constexpr uint16_t WIFI_PORTAL_TIMEOUT_SECONDS = 300;

    constexpr const char* NTP_SERVER_PRIMARY = "pool.ntp.org";
    constexpr const char* NTP_SERVER_SECONDARY = "time.nist.gov";
    constexpr const char* ISRAEL_TIMEZONE = "IST-2IDT,M3.4.4/26,M10.5.0";
}
