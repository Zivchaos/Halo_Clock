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
    constexpr uint8_t OTA_LED_BRIGHTNESS = 40;
    constexpr uint32_t OTA_READY_NOTICE_MS = 1500;
    constexpr uint32_t OTA_RESULT_NOTICE_MS = 3000;

    constexpr uint16_t WEB_PORT = 80;
    constexpr uint32_t WEB_REBOOT_DELAY_MS = 500;

    // Current weather is provided by Open-Meteo without an API key. These
    // checked-in coordinates are a deliberate public example near Greenwich,
    // UK. Change them to the installation's WGS84 latitude and longitude.
    constexpr const char* WEATHER_API_URL = "https://api.open-meteo.com/v1/forecast";
    constexpr float WEATHER_LATITUDE = 51.4769F;
    constexpr float WEATHER_LONGITUDE = 0.0005F;
    constexpr const char* WEATHER_TEMPERATURE_UNIT = "celsius";
    constexpr const char* WEATHER_WIND_SPEED_UNIT = "kmh";
    constexpr uint32_t WEATHER_REFRESH_INTERVAL_MS = 15UL * 60UL * 1000UL;
    constexpr uint32_t WEATHER_FAILURE_BACKOFF_MS = 2UL * 60UL * 1000UL;
    constexpr uint32_t WEATHER_STALE_AFTER_MS = 45UL * 60UL * 1000UL;
    constexpr uint32_t WEATHER_MANUAL_REFRESH_LIMIT_MS = 30UL * 1000UL;
    constexpr uint32_t WEATHER_STARTUP_DELAY_MS = 5UL * 1000UL;
    constexpr uint32_t WEATHER_CONNECT_TIMEOUT_MS = 8000;
    constexpr uint32_t WEATHER_RESPONSE_TIMEOUT_MS = 8000;
    constexpr size_t WEATHER_MAX_RESPONSE_BYTES = 4096;
    constexpr uint32_t WEATHER_OLED_DURATION_MS = 5000;
    constexpr uint32_t WEATHER_TASK_STACK_SIZE = 8192;
    constexpr bool ENABLE_WEATHER_OLED = true;

    // Compatibility contract: keep the legacy namespace so branded upgrades
    // retain brightness, display mode, and automatic NIGHT settings.
    constexpr const char* SETTINGS_NAMESPACE = "halo-clock";
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

    constexpr uint16_t WIFI_PORTAL_TIMEOUT_SECONDS = 300;

    constexpr const char* NTP_SERVER_PRIMARY = "pool.ntp.org";
    constexpr const char* NTP_SERVER_SECONDARY = "time.nist.gov";
    // POSIX timezone default retained for existing installations. Public
    // builders should configure this for their location before flashing.
    constexpr const char* LOCAL_TIMEZONE = "IST-2IDT,M3.4.4/26,M10.5.0";
}
