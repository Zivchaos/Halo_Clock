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

    constexpr uint8_t LED_BRIGHTNESS_LEVELS[] = {10, 25, 40, 80};
    constexpr size_t LED_BRIGHTNESS_LEVEL_COUNT =
        sizeof(LED_BRIGHTNESS_LEVELS) / sizeof(LED_BRIGHTNESS_LEVELS[0]);
    constexpr uint8_t NIGHT_LED_BRIGHTNESS = 3;
    constexpr uint8_t NIGHT_OLED_CONTRAST = 32;
    constexpr uint32_t MODE_NOTICE_DURATION_MS = 1000;

    constexpr bool isSupportedBrightness(uint8_t level)
    {
        for (size_t index = 0; index < LED_BRIGHTNESS_LEVEL_COUNT; ++index)
        {
            if (LED_BRIGHTNESS_LEVELS[index] == level)
            {
                return true;
            }
        }
        return false;
    }

    constexpr bool AUTO_NIGHT_ENABLED = true;
    constexpr uint8_t AUTO_NIGHT_START_HOUR = 20;
    constexpr uint8_t AUTO_NIGHT_START_MINUTE = 0;
    constexpr uint8_t AUTO_NIGHT_END_HOUR = 6;
    constexpr uint8_t AUTO_NIGHT_END_MINUTE = 0;

    // OTA is intentionally unauthenticated and must only be used on a trusted local network.
    constexpr const char* OTA_HOSTNAME = "REDACTED";
    constexpr uint8_t OTA_LED_BRIGHTNESS = 40;
    constexpr uint32_t OTA_READY_NOTICE_MS = 1500;
    constexpr uint32_t OTA_RESULT_NOTICE_MS = 3000;

    constexpr uint16_t WEB_PORT = 80;
    constexpr uint32_t WEB_REBOOT_DELAY_MS = 500;

    constexpr const char* SETTINGS_NAMESPACE = "REDACTED";
    constexpr const char* SETTINGS_BRIGHTNESS_KEY = "brightness";
    constexpr const char* SETTINGS_DISPLAY_MODE_KEY = "displayMode";
    constexpr const char* SETTINGS_AUTO_NIGHT_ENABLED_KEY = "autoEnabled";
    constexpr const char* SETTINGS_AUTO_NIGHT_START_HOUR_KEY = "autoStartH";
    constexpr const char* SETTINGS_AUTO_NIGHT_START_MINUTE_KEY = "autoStartM";
    constexpr const char* SETTINGS_AUTO_NIGHT_END_HOUR_KEY = "autoEndH";
    constexpr const char* SETTINGS_AUTO_NIGHT_END_MINUTE_KEY = "autoEndM";

    // LED ring calibration. Logical position 0 is 12 o'clock; positions then
    // advance around the clock face in the selected direction.
    constexpr uint8_t LED_ZERO_OFFSET = 0;
    constexpr bool LED_CLOCKWISE = true;
    constexpr bool ENABLE_RING_CALIBRATION = false;

    constexpr const char* WIFI_PORTAL_NAME = "HALO Clock Setup";
    constexpr const char* WIFI_PORTAL_PASSWORD = "REDACTED";
    constexpr uint16_t WIFI_PORTAL_TIMEOUT_SECONDS = 300;

    constexpr const char* NTP_SERVER_PRIMARY = "pool.ntp.org";
    constexpr const char* NTP_SERVER_SECONDARY = "time.nist.gov";
    constexpr const char* ISRAEL_TIMEZONE = "IST-2IDT,M3.4.4/26,M10.5.0";
}
