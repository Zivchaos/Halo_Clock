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

    // OTA is disabled by default and requires an administrator password when enabled.
    constexpr uint8_t OTA_LED_BRIGHTNESS = 40;
    constexpr uint32_t OTA_READY_NOTICE_MS = 1500;
    constexpr uint32_t OTA_RESULT_NOTICE_MS = 3000;

    constexpr uint16_t WEB_PORT = 80;
    constexpr uint32_t WEB_REBOOT_DELAY_MS = 500;
    // Reboot if the main application loop stops servicing the clock. This
    // covers a stalled network stack as well as other unexpected loop hangs.
    constexpr uint32_t WATCHDOG_TIMEOUT_SECONDS = 30;

    // Current weather is provided by Open-Meteo without an API key. These
    // checked-in coordinates are the fresh-install default near Greenwich,
    // UK. The Web UI can persist installation-specific WGS84 coordinates.
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

    // Optional Israeli Home Front Command companion. This is a visual aid
    // only; official alerts and instructions always take precedence.
    constexpr const char* RED_ALERT_API_URL = "https://www.oref.org.il/WarningMessages/alert/alerts.json";
    constexpr const char* RED_ALERT_DEFAULT_RELAY_URL = "https://api.tzevaadom.co.il/notifications";
    constexpr bool RED_ALERT_DEFAULT_ENABLED = false;
    // Community HTTPS relays are polled conservatively so they remain a
    // secondary visual aid without competing with clock rendering or weather.
    constexpr uint32_t RED_ALERT_POLL_INTERVAL_MS = 15000;
    constexpr uint32_t RED_ALERT_FAILURE_BACKOFF_MS = 30000;
    // Community providers behind CDN/TLS endpoints can take longer than the
    // local weather service to complete a first connection.
    constexpr uint32_t RED_ALERT_CONNECT_TIMEOUT_MS = 15000;
    constexpr uint32_t RED_ALERT_RESPONSE_TIMEOUT_MS = 15000;
    // Keep TLS setup away from the clock's full redraw at the minute boundary.
    constexpr uint8_t RED_ALERT_MINUTE_TRANSITION_GUARD_SECONDS = 3;
    constexpr size_t RED_ALERT_MAX_RESPONSE_BYTES = 4096;
    constexpr uint32_t RED_ALERT_TASK_STACK_SIZE = 8192;
    // Keep TLS work off the Arduino loop core, which owns display rendering.
    constexpr BaseType_t RED_ALERT_TASK_CORE = 0;
    // Stop live polling before TLS allocation pressure can affect rendering.
    // A reboot resets this guard; the saved Red Alert setting remains unchanged.
    constexpr uint32_t RED_ALERT_MIN_FREE_HEAP_BYTES = 160UL * 1024UL;
    constexpr bool RED_ALERT_LIVE_POLLING_ENABLED = true;

    // Compatibility contract: keep the legacy namespace so branded upgrades
    // retain brightness, display mode, and automatic NIGHT settings.
    constexpr const char* SETTINGS_NAMESPACE = "REDACTED";
    constexpr const char* SETTINGS_BRIGHTNESS_KEY = "brightness";
    constexpr const char* SETTINGS_DISPLAY_MODE_KEY = "displayMode";
    constexpr const char* SETTINGS_AUTO_NIGHT_ENABLED_KEY = "autoEnabled";
    constexpr const char* SETTINGS_AUTO_NIGHT_START_HOUR_KEY = "autoStartH";
    constexpr const char* SETTINGS_AUTO_NIGHT_START_MINUTE_KEY = "autoStartM";
    constexpr const char* SETTINGS_AUTO_NIGHT_END_HOUR_KEY = "autoEndH";
    constexpr const char* SETTINGS_AUTO_NIGHT_END_MINUTE_KEY = "autoEndM";
    constexpr const char* SETTINGS_NETWORK_KEY = "networkCfg";
    constexpr const char* SETTINGS_STATIC_FAILURE_COUNT_KEY = "netFail";
    constexpr const char* SETTINGS_RING_ZERO_KEY = "ringZero";
    constexpr const char* SETTINGS_RING_CLOCKWISE_KEY = "ringCw";
    constexpr const char* SETTINGS_WEATHER_LOCATION_KEY = "weatherLoc";
    constexpr const char* SETTINGS_CUSTOM_COLORS_KEY = "customLed";
    constexpr const char* SETTINGS_RED_ALERT_KEY = "redAlert";
    constexpr const char* SETTINGS_HARDWARE_KEY = "hardware";

    constexpr uint8_t STATIC_NETWORK_FAILURE_LIMIT = 2;
    constexpr uint32_t STATIC_NETWORK_VALIDATION_TIMEOUT_MS = 45UL * 1000UL;
    constexpr uint32_t STATIC_NETWORK_CONFIRMATION_MS = 15UL * 1000UL;
    constexpr uint8_t STATIC_NETWORK_MAX_PRECONFIRM_DROPS = 3;
    constexpr uint32_t NETWORK_RECOVERY_HOLD_MS = 10UL * 1000UL;
    constexpr uint32_t NETWORK_RECOVERY_BOOT_WINDOW_MS = 30UL * 1000UL;
    constexpr uint32_t NETWORK_REBOOT_DELAY_MS = 750;

    // Fresh-install LED ring calibration defaults. Logical position 0 is
    // 12 o'clock; the Web UI can persist offset and direction changes.
    constexpr uint8_t LED_ZERO_OFFSET = 0;
    constexpr bool LED_CLOCKWISE = true;
    constexpr bool ENABLE_RING_CALIBRATION = false;

    constexpr uint16_t WIFI_PORTAL_TIMEOUT_SECONDS = 300;
    // Recover from a station interface that remains disconnected despite
    // ESP32 auto-reconnect. These do not erase Wi-Fi credentials.
    constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 15UL * 1000UL;
    constexpr uint32_t WIFI_STACK_RESET_AFTER_MS = 60UL * 1000UL;
    constexpr uint32_t WIFI_REBOOT_AFTER_MS = 3UL * 60UL * 1000UL;

    constexpr const char* NTP_SERVER_PRIMARY = "pool.ntp.org";
    constexpr const char* NTP_SERVER_SECONDARY = "time.nist.gov";
    // POSIX timezone default retained for existing installations. Public
    // builders should configure this for their location before flashing.
    constexpr const char* LOCAL_TIMEZONE = "IST-2IDT,M3.4.4/26,M10.5.0";
}
