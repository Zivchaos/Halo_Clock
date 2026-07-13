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

    // LED ring calibration. Logical position 0 is 12 o'clock; positions then
    // advance around the clock face in the selected direction.
    constexpr uint8_t LED_ZERO_OFFSET = 0;
    constexpr bool LED_CLOCKWISE = true;
    constexpr bool ENABLE_LED_CALIBRATION_TEST = false;

    constexpr const char* WIFI_PORTAL_NAME = "HALO Clock Setup";
    constexpr const char* WIFI_PORTAL_PASSWORD = "halo-clock";
    constexpr uint16_t WIFI_PORTAL_TIMEOUT_SECONDS = 300;

    constexpr const char* NTP_SERVER_PRIMARY = "pool.ntp.org";
    constexpr const char* NTP_SERVER_SECONDARY = "time.nist.gov";
    constexpr const char* ISRAEL_TIMEZONE = "IST-2IDT,M3.4.4/26,M10.5.0";
}
