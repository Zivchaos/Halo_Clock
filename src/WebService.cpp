#include "WebService.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <cmath>
#include <cstdlib>

#include "AutoNightService.h"
#include "Clock.h"
#include "Config.h"
#include "DiagnosticsService.h"
#include "Halo.h"
#include "Hardware.h"
#include "NetworkConfig.h"
#include "NetworkService.h"
#include "OtaService.h"
#include "RedAlertService.h"
#include "SettingsService.h"
#include "TimeService.h"
#include "Version.h"
#include "WeatherService.h"
#include "WebUiAssets.h"

namespace
{
    WebServer server(Config::WEB_PORT);
    bool started = false;
    bool routesConfigured = false;
    bool wifiWasConnected = false;
    bool mdnsStarted = false;
    bool rebootScheduled = false;
    uint32_t rebootRequestedAt = 0;

    void sendSecurityHeaders()
    {
        server.sendHeader("X-Content-Type-Options", "nosniff");
        server.sendHeader("X-Frame-Options", "DENY");
        server.sendHeader("Referrer-Policy", "no-referrer");
        server.sendHeader("Content-Security-Policy", "frame-ancestors 'none'; base-uri 'none'");
    }

    void sendJson(int statusCode, const char* body)
    {
        sendSecurityHeaders();
        server.sendHeader("Cache-Control", "no-store");
        server.send(statusCode, "application/json", body);
    }

    void sendError(int statusCode, const char* reason)
    {
        char body[160];
        snprintf(body, sizeof(body), "{\"ok\":false,\"error\":\"%s\"}", reason);
        sendJson(statusCode, body);
        Serial.printf("WEB ERROR: %s\r\n", reason);
    }

    String addressText(const IPv4Address& address)
    {
        char text[16];
        NetworkConfig::formatIPv4(address, text, sizeof(text));
        return String(text);
    }

    bool parseRequiredAddress(const char* name, IPv4Address& address)
    {
        return server.hasArg(name) && NetworkConfig::parseIPv4(server.arg(name).c_str(), address);
    }

    bool parseByteArgument(const char* name, uint8_t maximum, uint8_t& result)
    {
        if (!server.hasArg(name))
        {
            return false;
        }

        const String value = server.arg(name);
        if (value.isEmpty())
        {
            return false;
        }

        uint16_t parsed = 0;
        for (size_t index = 0; index < value.length(); ++index)
        {
            const char character = value[index];
            if (character < '0' || character > '9')
            {
                return false;
            }
            parsed = static_cast<uint16_t>(parsed * 10U + static_cast<uint8_t>(character - '0'));
            if (parsed > maximum)
            {
                return false;
            }
        }
        result = static_cast<uint8_t>(parsed);
        return true;
    }

    bool changesAllowed()
    {
        if (!server.hasHeader("X-Halo-Request") || server.header("X-Halo-Request") != "1")
        {
            sendError(403, "trusted HALO request required");
            return false;
        }
        if (server.hasHeader("Origin"))
        {
            const String origin = server.header("Origin");
            const String hostname = String("http://") + Product::HOSTNAME + ".local";
            const String ip = String("http://") + WiFi.localIP().toString();
            if (origin != hostname && origin != ip && origin != "http://192.168.4.1")
            {
                sendError(403, "untrusted request origin");
                return false;
            }
        }
        if (OtaService::isUpdating())
        {
            sendError(503, "OTA update in progress");
            return false;
        }
        return true;
    }

    void handleRoot()
    {
        Serial.println("WEB REQUEST: GET /");
        server.sendHeader("Cache-Control", "no-store");
        server.send_P(200, "text/html; charset=utf-8", WebUiAssets::INDEX_HTML);
    }

    bool parseFloatArgument(const char* name, float minimum, float maximum, float& result)
    {
        if (!server.hasArg(name)) return false;
        const String value = server.arg(name);
        if (value.isEmpty()) return false;
        char* end = nullptr;
        const float parsed = std::strtof(value.c_str(), &end);
        if (end == value.c_str() || *end != '\0' || !std::isfinite(parsed) || parsed < minimum || parsed > maximum)
        {
            return false;
        }
        result = parsed;
        return true;
    }

    bool parseColorArgument(const char* name, RgbColor& result)
    {
        if (!server.hasArg(name)) return false;
        const String value = server.arg(name);
        if (value.length() != 7 || value[0] != '#') return false;
        char* end = nullptr;
        const unsigned long parsed = std::strtoul(value.c_str() + 1, &end, 16);
        if (end != value.c_str() + 7 || *end != '\0') return false;
        for (size_t index = 1; index < 7; ++index)
        {
            const char c = value[index];
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) return false;
        }
        result = {
            static_cast<uint8_t>((parsed >> 16) & 0xFFU),
            static_cast<uint8_t>((parsed >> 8) & 0xFFU),
            static_cast<uint8_t>(parsed & 0xFFU)};
        return true;
    }

    bool parseRedAlertSettings(RedAlertSettings& result)
    {
        if (!server.hasArg("enabled") || !server.hasArg("locations") || !server.hasArg("relayUrl")) return false;
        const String enabled = server.arg("enabled");
        const String locations = server.arg("locations");
        const String relayUrl = server.arg("relayUrl");
        if ((enabled != "true" && enabled != "false") || locations.length() >= sizeof(result.locations) ||
            relayUrl.length() >= sizeof(result.relayUrl) || (enabled == "true" && (locations.isEmpty() || relayUrl.isEmpty()))) return false;
        result.enabled = enabled == "true";
        snprintf(result.locations, sizeof(result.locations), "%s", locations.c_str());
        snprintf(result.relayUrl, sizeof(result.relayUrl), "%s", relayUrl.c_str());
        return true;
    }

    void colorText(const RgbColor& color, char destination[8])
    {
        snprintf(destination, 8, "#%02X%02X%02X", color.red, color.green, color.blue);
    }

    bool parseBackupColor(JsonVariant value, RgbColor& result)
    {
        if (!value.is<const char*>()) return false;
        const char* text = value.as<const char*>();
        if (text == nullptr || strlen(text) != 7 || text[0] != '#') return false;
        char* end = nullptr;
        const unsigned long parsed = std::strtoul(text + 1, &end, 16);
        if (end != text + 7 || *end != '\0') return false;
        for (size_t index = 1; index < 7; ++index)
        {
            const char c = text[index];
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) return false;
        }
        result = {static_cast<uint8_t>((parsed >> 16) & 0xFFU), static_cast<uint8_t>((parsed >> 8) & 0xFFU), static_cast<uint8_t>(parsed & 0xFFU)};
        return true;
    }

    void handleSettingsExport()
    {
        JsonDocument document;
        document["format"] = "halo-cst-settings";
        document["version"] = 1;
        document["brightness"] = SettingsService::brightness();
        document["displayMode"] = DisplayModes::name(SettingsService::displayMode());
        const AutoNightSettings& autoNight = SettingsService::autoNight();
        JsonObject autoNightJson = document["autoNight"].to<JsonObject>();
        autoNightJson["enabled"] = autoNight.enabled;
        autoNightJson["startHour"] = autoNight.startHour;
        autoNightJson["startMinute"] = autoNight.startMinute;
        autoNightJson["endHour"] = autoNight.endHour;
        autoNightJson["endMinute"] = autoNight.endMinute;
        const RingCalibrationSettings& calibration = SettingsService::ringCalibration();
        JsonObject calibrationJson = document["calibration"].to<JsonObject>();
        calibrationJson["zeroOffset"] = calibration.zeroOffset;
        calibrationJson["clockwise"] = calibration.clockwise;
        const WeatherLocationSettings& location = SettingsService::weatherLocation();
        JsonObject locationJson = document["weatherLocation"].to<JsonObject>();
        locationJson["latitude"] = location.latitude;
        locationJson["longitude"] = location.longitude;
        const CustomColorSettings& colors = SettingsService::customColors();
        JsonObject colorsJson = document["customColors"].to<JsonObject>();
        char color[8];
        colorText(colors.hourTicks, color); colorsJson["hourTicks"] = color;
        colorText(colors.minuteProgress, color); colorsJson["minuteProgress"] = color;
        colorText(colors.hourCenter, color); colorsJson["hourCenter"] = color;
        colorText(colors.hourSides, color); colorsJson["hourSides"] = color;
        colorText(colors.minuteMarker, color); colorsJson["minuteMarker"] = color;
        colorText(colors.secondMarker, color); colorsJson["secondMarker"] = color;
        const RedAlertSettings& redAlert = SettingsService::redAlert();
        JsonObject redAlertJson = document["redAlert"].to<JsonObject>();
        redAlertJson["enabled"] = redAlert.enabled;
        redAlertJson["locations"] = redAlert.locations;
        redAlertJson["relayUrl"] = redAlert.relayUrl;
        String body;
        body.reserve(1600);
        serializeJson(document, body);
        sendSecurityHeaders();
        server.sendHeader("Cache-Control", "no-store");
        server.sendHeader("Content-Disposition", "attachment; filename=halo-cst-settings.json");
        server.send(200, "application/json", body);
    }

    void handleSettingsImport()
    {
        if (!changesAllowed()) return;
        if (!server.hasArg("backup") || server.arg("backup").length() > 1800) { sendError(400, "invalid settings backup"); return; }
        JsonDocument document;
        if (deserializeJson(document, server.arg("backup"))) { sendError(400, "settings backup is not valid JSON"); return; }
        JsonObject root = document.as<JsonObject>();
        if (root["format"] != "halo-cst-settings" || root["version"] != 1) { sendError(400, "unsupported settings backup"); return; }
        if (!root["brightness"].is<uint8_t>() || !Config::isSupportedBrightness(root["brightness"].as<uint8_t>())) { sendError(400, "invalid backup brightness"); return; }
        const char* modeName = root["displayMode"] | "";
        DisplayMode mode;
        if (strcmp(modeName, "CLASSIC") == 0) mode = DisplayMode::CLASSIC;
        else if (strcmp(modeName, "MINIMAL") == 0) mode = DisplayMode::MINIMAL;
        else if (strcmp(modeName, "NIGHT") == 0) mode = DisplayMode::NIGHT;
        else if (strcmp(modeName, "CUSTOM") == 0) mode = DisplayMode::CUSTOM;
        else { sendError(400, "invalid backup display mode"); return; }
        JsonObject autoNightJson = root["autoNight"].as<JsonObject>();
        JsonObject calibrationJson = root["calibration"].as<JsonObject>();
        JsonObject locationJson = root["weatherLocation"].as<JsonObject>();
        JsonObject colorsJson = root["customColors"].as<JsonObject>();
        JsonObject redAlertJson = root["redAlert"].as<JsonObject>();
        AutoNightSettings autoNight = {autoNightJson["enabled"] | false, static_cast<uint8_t>(autoNightJson["startHour"] | 255), static_cast<uint8_t>(autoNightJson["startMinute"] | 255), static_cast<uint8_t>(autoNightJson["endHour"] | 255), static_cast<uint8_t>(autoNightJson["endMinute"] | 255)};
        RingCalibrationSettings calibration = {static_cast<uint8_t>(calibrationJson["zeroOffset"] | 255), calibrationJson["clockwise"] | false};
        WeatherLocationSettings location = {locationJson["latitude"] | NAN, locationJson["longitude"] | NAN};
        CustomColorSettings colors = {};
        RedAlertSettings redAlert = {};
        const char* locations = redAlertJson["locations"] | "";
        const char* relayUrl = redAlertJson["relayUrl"] | "";
        if (autoNightJson.isNull() || calibrationJson.isNull() || locationJson.isNull() || colorsJson.isNull() || redAlertJson.isNull() ||
            !autoNightJson["enabled"].is<bool>() || !autoNightJson["startHour"].is<uint8_t>() || !autoNightJson["startMinute"].is<uint8_t>() || !autoNightJson["endHour"].is<uint8_t>() || !autoNightJson["endMinute"].is<uint8_t>() ||
            !calibrationJson["zeroOffset"].is<uint8_t>() || !calibrationJson["clockwise"].is<bool>() || !locationJson["latitude"].is<float>() || !locationJson["longitude"].is<float>() ||
            !redAlertJson["enabled"].is<bool>() || !redAlertJson["locations"].is<const char*>() || !redAlertJson["relayUrl"].is<const char*>() ||
            autoNight.startHour > 23 || autoNight.startMinute > 59 || autoNight.endHour > 23 || autoNight.endMinute > 59 ||
            calibration.zeroOffset >= Hardware::LED_COUNT || !std::isfinite(location.latitude) || !std::isfinite(location.longitude) || location.latitude < -90 || location.latitude > 90 || location.longitude < -180 || location.longitude > 180 ||
            strlen(locations) >= sizeof(redAlert.locations) || strlen(relayUrl) >= sizeof(redAlert.relayUrl) ||
            ((redAlertJson["enabled"] | false) && (locations[0] == '\0' || relayUrl[0] == '\0')) ||
            !parseBackupColor(colorsJson["hourTicks"], colors.hourTicks) || !parseBackupColor(colorsJson["minuteProgress"], colors.minuteProgress) || !parseBackupColor(colorsJson["hourCenter"], colors.hourCenter) || !parseBackupColor(colorsJson["hourSides"], colors.hourSides) || !parseBackupColor(colorsJson["minuteMarker"], colors.minuteMarker) || !parseBackupColor(colorsJson["secondMarker"], colors.secondMarker))
        { sendError(400, "settings backup contains invalid values"); return; }
        redAlert.enabled = redAlertJson["enabled"] | false;
        snprintf(redAlert.locations, sizeof(redAlert.locations), "%s", locations);
        snprintf(redAlert.relayUrl, sizeof(redAlert.relayUrl), "%s", relayUrl);
        // Apply after a controlled reboot rather than changing several live
        // rendering and service states from a single WebServer request.
        SettingsService::saveBrightness(root["brightness"].as<uint8_t>());
        SettingsService::saveDisplayMode(mode);
        if (!SettingsService::saveAutoNight(autoNight) || !SettingsService::saveRingCalibration(calibration) ||
            !SettingsService::saveWeatherLocation(location) || !SettingsService::saveCustomColors(colors) ||
            !SettingsService::saveRedAlert(redAlert))
        {
            sendError(500, "settings backup could not be saved"); return;
        }
        sendJson(200, "{\"ok\":true,\"rebootScheduled\":true}");
        rebootRequestedAt = millis();
        rebootScheduled = true;
    }

    void handleFavicon()
    {
        sendSecurityHeaders();
        server.sendHeader("Cache-Control", "public, max-age=86400");
        server.send_P(200, "image/svg+xml", WebUiAssets::FAVICON_SVG);
    }

    void handleStatus()
    {
        const DiagnosticsData diagnostics = DiagnosticsService::snapshot();
        char timeText[9] = "--:--:--";
        if (TimeService::isSynchronized())
        {
            strftime(timeText, sizeof(timeText), "%H:%M:%S", &TimeService::localTime());
        }
        char dateText[24] = "Time unavailable";
        if (TimeService::isSynchronized())
        {
            strftime(dateText, sizeof(dateText), "%a, %b %d", &TimeService::localTime());
        }

        const AutoNightSettings& autoNight = SettingsService::autoNight();
        const WeatherData weather = WeatherService::snapshot();
        const RedAlertData redAlert = RedAlertService::snapshot();
        const NetworkStatus network = NetworkService::snapshot();
        JsonDocument document;
        document["time"] = timeText;
        document["date"] = dateText;
        document["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        document["ip"] = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "0.0.0.0";
        document["rssi"] = WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0;
        document["uptimeSeconds"] = diagnostics.uptimeSeconds;
        document["freeHeap"] = diagnostics.freeHeap;
        document["minimumFreeHeap"] = diagnostics.minimumFreeHeap;
        document["firmwareVersion"] = diagnostics.firmwareVersion;
        document["resetReason"] = diagnostics.resetReason;
        document["runningPartition"] = diagnostics.runningPartition;
        document["wifiReconnectCount"] = diagnostics.wifiReconnectCount;
        document["ntpSynchronized"] = diagnostics.ntpSynchronized;
        document["timeSyncAgeValid"] = diagnostics.timeSyncAgeValid;
        document["lastTimeSyncAgeSeconds"] = diagnostics.lastTimeSyncAgeSeconds;
        document["weatherRequestCount"] = diagnostics.weatherRequestCount;
        document["weatherSuccessCount"] = diagnostics.weatherSuccessCount;
        document["weatherFailureCount"] = diagnostics.weatherFailureCount;
        document["selectedMode"] = DisplayModes::name(SettingsService::displayMode());
        document["effectiveMode"] = DisplayModes::name(Clock::effectiveMode());
        document["brightness"] = SettingsService::brightness();
        document["brightnessMaximum"] = Config::LED_BRIGHTNESS_LEVELS[Config::LED_BRIGHTNESS_LEVEL_COUNT - 1];
        document["autoNightEnabled"] = autoNight.enabled;
        document["autoNightActive"] = AutoNightService::isActive();
        char schedule[8]; snprintf(schedule, sizeof(schedule), "%02u:%02u", autoNight.startHour, autoNight.startMinute); document["autoNightStart"] = schedule;
        snprintf(schedule, sizeof(schedule), "%02u:%02u", autoNight.endHour, autoNight.endMinute); document["autoNightEnd"] = schedule;
        document["manualOverride"] = AutoNightService::isManualOverride();
        document["otaReady"] = OtaService::isReady(); document["otaUpdating"] = OtaService::isUpdating();
        document["networkConfiguredMode"] = NetworkConfig::modeName(network.configuredMode); document["networkEffectiveMode"] = NetworkConfig::modeName(network.effectiveMode);
        document["dhcpFallbackActive"] = network.dhcpFallbackActive; document["staticConnectionFailureCount"] = network.staticConnectionFailureCount;
        document["weatherAvailable"] = weather.valid; document["weatherStale"] = weather.stale;
        if (weather.valid) { document["temperature"] = weather.temperature; document["humidity"] = weather.humidity; document["windSpeed"] = weather.windSpeed; if (weather.apparentTemperatureAvailable) document["apparentTemperature"] = weather.apparentTemperature; else document["apparentTemperature"] = nullptr; } else { document["temperature"] = nullptr; document["humidity"] = nullptr; document["windSpeed"] = nullptr; document["apparentTemperature"] = nullptr; }
        document["condition"] = weather.condition; document["weatherLastUpdate"] = weather.lastSuccessfulUpdateEpoch; document["weatherError"] = weather.error;
        document["redAlertEnabled"] = redAlert.enabled; document["redAlertActive"] = redAlert.active; document["redAlertStale"] = redAlert.stale; document["redAlertUpdating"] = redAlert.updating; document["redAlertAreas"] = redAlert.areas; document["redAlertError"] = redAlert.error;
        document["redAlertHasSuccessfulCheck"] = redAlert.lastSuccessfulUpdateMs != 0; document["redAlertLastCheckAgeSeconds"] = redAlert.lastSuccessfulUpdateMs == 0 ? 0 : (millis() - redAlert.lastSuccessfulUpdateMs) / 1000UL; document["redAlertFailureCount"] = redAlert.consecutiveFailures;
        document["redAlertRequestCount"] = redAlert.requestCount; document["redAlertSuccessCount"] = redAlert.successCount; document["redAlertTotalFailureCount"] = redAlert.failureCount; document["redAlertLastRequestDurationMs"] = redAlert.lastRequestDurationMs; document["redAlertLastRequestFreeHeap"] = redAlert.lastRequestFreeHeap; document["redAlertLastRequestMinimumFreeHeap"] = redAlert.lastRequestMinimumFreeHeap;
        String body; body.reserve(1800);
        if (serializeJson(document, body) == 0) { sendError(500, "status serialization failed"); return; }
        sendJson(200, body.c_str());
    }

    void handleDiagnostics()
    {
        const DiagnosticsData diagnostics = DiagnosticsService::snapshot();
        JsonDocument document;

        JsonObject system = document["system"].to<JsonObject>();
        system["uptimeSeconds"] = diagnostics.uptimeSeconds;
        system["freeHeap"] = diagnostics.freeHeap;
        system["minimumFreeHeap"] = diagnostics.minimumFreeHeap;
        system["resetReason"] = diagnostics.resetReason;

        JsonObject wifi = document["wifi"].to<JsonObject>();
        wifi["connected"] = diagnostics.wifiConnected;
        wifi["reconnectCount"] = diagnostics.wifiReconnectCount;
        wifi["rssi"] = diagnostics.wifiRssi;

        const NetworkStatus networkStatus = NetworkService::snapshot();
        JsonObject network = document["network"].to<JsonObject>();
        network["configuredMode"] = NetworkConfig::modeName(networkStatus.configuredMode);
        network["effectiveMode"] = NetworkConfig::modeName(networkStatus.effectiveMode);
        network["activeIp"] = addressText(networkStatus.activeIp);
        network["gateway"] = addressText(networkStatus.gateway);
        network["subnet"] = addressText(networkStatus.subnet);
        network["primaryDns"] = addressText(networkStatus.primaryDns);
        network["secondaryDns"] = addressText(networkStatus.secondaryDns);
        network["dhcpFallbackActive"] = networkStatus.dhcpFallbackActive;
        network["staticConnectionFailureCount"] = networkStatus.staticConnectionFailureCount;

        JsonObject time = document["time"].to<JsonObject>();
        time["ntpSynchronized"] = diagnostics.ntpSynchronized;
        time["syncAgeValid"] = diagnostics.timeSyncAgeValid;
        time["lastSuccessfulSyncAgeSeconds"] = diagnostics.lastTimeSyncAgeSeconds;

        JsonObject weather = document["weather"].to<JsonObject>();
        weather["requestCount"] = diagnostics.weatherRequestCount;
        weather["successCount"] = diagnostics.weatherSuccessCount;
        weather["failureCount"] = diagnostics.weatherFailureCount;
        weather["lastError"] = diagnostics.lastWeatherError;

        JsonObject ota = document["ota"].to<JsonObject>();
        ota["ready"] = diagnostics.otaReady;
        ota["updating"] = diagnostics.otaUpdating;
        ota["runningPartition"] = diagnostics.runningPartition;

        JsonObject firmware = document["firmware"].to<JsonObject>();
        firmware["version"] = diagnostics.firmwareVersion;
        firmware["buildDate"] = diagnostics.buildDate;
        firmware["buildTime"] = diagnostics.buildTime;

        String body;
        body.reserve(1024);
        serializeJson(document, body);
        sendSecurityHeaders();
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json", body);
    }

    void handleWeatherRefresh()
    {
        Serial.println("WEB REQUEST: POST /api/weather/refresh");
        if (!changesAllowed()) return;

        switch (WeatherService::requestRefresh())
        {
            case WeatherRefreshResult::ACCEPTED:
                sendJson(202, "{\"ok\":true}");
                return;
            case WeatherRefreshResult::RATE_LIMITED:
                sendError(429, "weather refresh rate limited");
                return;
            case WeatherRefreshResult::BUSY:
                sendError(409, "weather refresh already in progress");
                return;
            case WeatherRefreshResult::OFFLINE:
                sendError(503, "weather refresh unavailable while offline");
                return;
        }
    }

    void handleCustomizationGet()
    {
        const RingCalibrationSettings& calibration = SettingsService::ringCalibration();
        const WeatherLocationSettings& location = SettingsService::weatherLocation();
        const CustomColorSettings& colors = SettingsService::customColors();
        JsonDocument document;
        JsonObject calibrationJson = document["calibration"].to<JsonObject>();
        calibrationJson["zeroOffset"] = calibration.zeroOffset;
        calibrationJson["clockwise"] = calibration.clockwise;
        calibrationJson["active"] = Halo::isRingCalibrationActive();
        JsonObject locationJson = document["weatherLocation"].to<JsonObject>();
        locationJson["latitude"] = location.latitude;
        locationJson["longitude"] = location.longitude;
        JsonObject colorsJson = document["customColors"].to<JsonObject>();
        char colorValue[8];
        colorText(colors.hourTicks, colorValue); colorsJson["hourTicks"] = colorValue;
        colorText(colors.minuteProgress, colorValue); colorsJson["minuteProgress"] = colorValue;
        colorText(colors.hourCenter, colorValue); colorsJson["hourCenter"] = colorValue;
        colorText(colors.hourSides, colorValue); colorsJson["hourSides"] = colorValue;
        colorText(colors.minuteMarker, colorValue); colorsJson["minuteMarker"] = colorValue;
        colorText(colors.secondMarker, colorValue); colorsJson["secondMarker"] = colorValue;
        const RedAlertSettings& redAlert = SettingsService::redAlert();
        JsonObject redAlertJson = document["redAlert"].to<JsonObject>();
        redAlertJson["enabled"] = redAlert.enabled;
        redAlertJson["locations"] = redAlert.locations;
        redAlertJson["relayUrl"] = redAlert.relayUrl;
        String body;
        body.reserve(480);
        serializeJson(document, body);
        sendSecurityHeaders();
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json", body);
    }

    void handleCalibration()
    {
        Serial.println("WEB REQUEST: POST /api/calibration");
        if (!changesAllowed()) return;
        uint8_t zeroOffset = 0;
        if (!parseByteArgument("zeroOffset", Hardware::LED_COUNT - 1U, zeroOffset) ||
            !server.hasArg("clockwise") || !server.hasArg("active"))
        {
            sendError(400, "invalid calibration settings");
            return;
        }
        const String clockwiseValue = server.arg("clockwise");
        const String activeValue = server.arg("active");
        if ((clockwiseValue != "true" && clockwiseValue != "false") ||
            (activeValue != "true" && activeValue != "false"))
        {
            sendError(400, "invalid calibration settings");
            return;
        }
        const RingCalibrationSettings settings = {zeroOffset, clockwiseValue == "true"};
        if (!Halo::setRingCalibration(settings, activeValue == "true"))
        {
            sendError(500, "calibration settings could not be saved");
            return;
        }
        sendJson(200, "{\"ok\":true}");
    }

    void handleWeatherLocation()
    {
        Serial.println("WEB REQUEST: POST /api/weather/location");
        if (!changesAllowed()) return;
        WeatherLocationSettings settings = {};
        if (!parseFloatArgument("latitude", -90.0F, 90.0F, settings.latitude) ||
            !parseFloatArgument("longitude", -180.0F, 180.0F, settings.longitude))
        {
            sendError(400, "invalid weather coordinates");
            return;
        }
        if (WeatherService::snapshot().updating)
        {
            sendError(409, "weather request already in progress");
            return;
        }
        if (!SettingsService::saveWeatherLocation(settings) || !WeatherService::locationChanged())
        {
            sendError(500, "weather location could not be applied");
            return;
        }
        sendJson(200, "{\"ok\":true,\"refreshScheduled\":true}");
    }

    void handleCustomColors()
    {
        Serial.println("WEB REQUEST: POST /api/custom-colors");
        if (!changesAllowed()) return;
        CustomColorSettings settings = {};
        if (!parseColorArgument("hourTicks", settings.hourTicks) ||
            !parseColorArgument("minuteProgress", settings.minuteProgress) ||
            !parseColorArgument("hourCenter", settings.hourCenter) ||
            !parseColorArgument("hourSides", settings.hourSides) ||
            !parseColorArgument("minuteMarker", settings.minuteMarker) ||
            !parseColorArgument("secondMarker", settings.secondMarker))
        {
            sendError(400, "invalid custom color");
            return;
        }
        if (!Halo::setCustomColors(settings))
        {
            sendError(500, "custom colors could not be saved");
            return;
        }
        sendJson(200, "{\"ok\":true}");
    }

    void handleRedAlertSettings()
    {
        Serial.println("WEB REQUEST: POST /api/red-alert");
        if (!changesAllowed()) return;
        RedAlertSettings settings = {};
        if (!server.hasArg("enabled") || !server.hasArg("locations") || !server.hasArg("relayUrl")) { sendError(400, "missing Red Alert settings"); return; }
        if (!parseRedAlertSettings(settings))
        {
            const String locations = server.arg("locations");
            const String relayUrl = server.arg("relayUrl");
            sendError(400, locations.length() >= sizeof(settings.locations)
                ? "too many selected alert areas" : (relayUrl.length() >= sizeof(settings.relayUrl)
                    ? "relay URL is too long" : "select areas and configure a relay URL before enabling"));
            return;
        }
        if (!SettingsService::saveRedAlert(settings) || !RedAlertService::settingsChanged())
        {
            sendError(500, "Red Alert settings could not be saved"); return;
        }
        sendJson(200, "{\"ok\":true}");
    }

    void handleRedAlertSimulation()
    {
        Serial.println("WEB REQUEST: POST /api/red-alert/simulate");
        if (!changesAllowed()) return;
        if (!RedAlertService::simulate()) { sendError(409, "enable Red Alert mode before simulating"); return; }
        sendJson(200, "{\"ok\":true}");
    }

    void handleOtaSettings()
    {
        if (!changesAllowed()) return;
        if (!server.hasArg("enabled")) { sendError(400, "missing OTA state"); return; }
        const bool enable = server.arg("enabled") == "true";
        if (!enable)
        {
            if (!OtaService::disableForSession()) { sendError(409, "OTA update in progress"); return; }
            sendJson(200, "{\"ok\":true}"); return;
        }
        if (!server.hasArg("password") || (!OtaService::isConfigured() && (!server.hasArg("passwordConfirm") || server.arg("password") != server.arg("passwordConfirm"))) || !OtaService::enableForSession(server.arg("password").c_str()))
        {
            sendError(403, OtaService::isConfigured() ? "invalid OTA admin password" : "enter and confirm a new OTA admin password"); return;
        }
        sendJson(200, "{\"ok\":true}");
    }

    void handleMode()
    {
        Serial.println("WEB REQUEST: POST /api/mode");
        if (!changesAllowed()) return;
        if (!server.hasArg("mode"))
        {
            sendError(400, "missing mode");
            return;
        }

        const String value = server.arg("mode");
        DisplayMode mode;
        if (value == "CLASSIC") mode = DisplayMode::CLASSIC;
        else if (value == "MINIMAL") mode = DisplayMode::MINIMAL;
        else if (value == "NIGHT") mode = DisplayMode::NIGHT;
        else if (value == "CUSTOM") mode = DisplayMode::CUSTOM;
        else
        {
            sendError(400, "invalid mode");
            return;
        }

        Halo::setDisplayMode(mode);
        Serial.printf("WEB SET MODE: %s\r\n", DisplayModes::name(mode));
        sendJson(200, "{\"ok\":true}");
    }

    void handleBrightness()
    {
        Serial.println("WEB REQUEST: POST /api/brightness");
        if (!changesAllowed()) return;
        uint8_t brightness = 0;
        if (!parseByteArgument("value", 255, brightness) || !Config::isSupportedBrightness(brightness))
        {
            sendError(400, "invalid brightness");
            return;
        }

        Halo::setBrightness(brightness);
        Serial.printf("WEB SET BRIGHTNESS: %u\r\n", brightness);
        sendJson(200, "{\"ok\":true}");
    }

    void handleAutoNight()
    {
        Serial.println("WEB REQUEST: POST /api/auto-night");
        if (!changesAllowed()) return;
        if (!server.hasArg("enabled"))
        {
            sendError(400, "missing automatic NIGHT state");
            return;
        }

        const String enabledValue = server.arg("enabled");
        if (enabledValue != "true" && enabledValue != "false")
        {
            sendError(400, "invalid automatic NIGHT state");
            return;
        }

        AutoNightSettings settings = {};
        settings.enabled = enabledValue == "true";
        if (!parseByteArgument("startHour", 23, settings.startHour) ||
            !parseByteArgument("startMinute", 59, settings.startMinute) ||
            !parseByteArgument("endHour", 23, settings.endHour) ||
            !parseByteArgument("endMinute", 59, settings.endMinute))
        {
            sendError(400, "invalid automatic NIGHT schedule");
            return;
        }

        const AutoNightSettings previous = SettingsService::autoNight();
        if (!SettingsService::saveAutoNight(settings))
        {
            sendError(500, "automatic NIGHT settings could not be saved");
            return;
        }

        const bool changed = settings.enabled != previous.enabled ||
            settings.startHour != previous.startHour || settings.startMinute != previous.startMinute ||
            settings.endHour != previous.endHour || settings.endMinute != previous.endMinute;
        if (changed)
        {
            AutoNightService::settingsChanged();
        }
        Serial.printf("WEB AUTO NIGHT: %s\r\n", settings.enabled ? "ENABLED" : "DISABLED");
        Serial.printf(
            "WEB AUTO NIGHT SCHEDULE: %02u:%02u-%02u:%02u\r\n",
            settings.startHour,
            settings.startMinute,
            settings.endHour,
            settings.endMinute);
        sendJson(200, "{\"ok\":true}");
    }

    void handleNetworkGet()
    {
        Serial.println("WEB REQUEST: GET /api/network");
        const NetworkSettings& configured = SettingsService::network();
        const NetworkStatus active = NetworkService::snapshot();
        JsonDocument document;

        JsonObject configuredJson = document["configured"].to<JsonObject>();
        configuredJson["mode"] = NetworkConfig::modeName(configured.mode);
        configuredJson["staticIp"] = addressText(configured.staticIp);
        configuredJson["gateway"] = addressText(configured.gateway);
        configuredJson["subnet"] = addressText(configured.subnet);
        configuredJson["primaryDns"] = addressText(configured.primaryDns);
        configuredJson["secondaryDns"] = addressText(configured.secondaryDns);
        configuredJson["secondaryDnsConfigured"] = configured.secondaryDnsConfigured;

        JsonObject effectiveJson = document["effective"].to<JsonObject>();
        effectiveJson["mode"] = NetworkConfig::modeName(active.effectiveMode);
        effectiveJson["dhcpFallbackActive"] = active.dhcpFallbackActive;
        effectiveJson["staticConnectionFailureCount"] = active.staticConnectionFailureCount;

        JsonObject activeJson = document["active"].to<JsonObject>();
        activeJson["ip"] = addressText(active.activeIp);
        activeJson["gateway"] = addressText(active.gateway);
        activeJson["subnet"] = addressText(active.subnet);
        activeJson["primaryDns"] = addressText(active.primaryDns);
        activeJson["secondaryDns"] = addressText(active.secondaryDns);

        String body;
        body.reserve(640);
        serializeJson(document, body);
        sendSecurityHeaders();
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json", body);
    }

    void handleNetworkSave()
    {
        Serial.println("WEB REQUEST: POST /api/network");
        if (!changesAllowed()) return;
        if (!server.hasArg("confirm") || server.arg("confirm") != "true")
        {
            sendError(400, "network change confirmation required");
            return;
        }
        if (!server.hasArg("mode"))
        {
            sendError(400, "missing network mode");
            return;
        }

        NetworkSettings settings = NetworkConfig::defaults();
        const String mode = server.arg("mode");
        if (mode == "DHCP")
        {
            settings.mode = NetworkMode::DHCP;
        }
        else if (mode == "STATIC")
        {
            settings.mode = NetworkMode::STATIC;
            if (!parseRequiredAddress("ip", settings.staticIp) ||
                !parseRequiredAddress("gateway", settings.gateway) ||
                !parseRequiredAddress("subnet", settings.subnet) ||
                !parseRequiredAddress("primaryDns", settings.primaryDns))
            {
                sendError(400, "missing or malformed static IPv4 field");
                return;
            }
            if (server.hasArg("secondaryDns") && !server.arg("secondaryDns").isEmpty())
            {
                settings.secondaryDnsConfigured = true;
                if (!NetworkConfig::parseIPv4(server.arg("secondaryDns").c_str(), settings.secondaryDns))
                {
                    sendError(400, "malformed secondary DNS address");
                    return;
                }
            }
        }
        else
        {
            sendError(400, "invalid network mode");
            return;
        }

        const NetworkValidationResult validation = NetworkConfig::validate(settings);
        if (!validation.valid)
        {
            sendError(400, validation.error);
            return;
        }
        if (!SettingsService::saveNetwork(settings))
        {
            sendError(500, "network settings could not be saved");
            return;
        }

        sendJson(200, "{\"ok\":true,\"rebootScheduled\":true}");
        rebootRequestedAt = millis();
        rebootScheduled = true;
        Serial.printf("WEB NETWORK APPLY: %s\r\n", NetworkConfig::modeName(settings.mode));
    }

    void handleHardwareGet()
    {
        Serial.println("WEB REQUEST: GET /api/hardware");
        const HardwareSettings& hardware = SettingsService::hardware();
        JsonDocument document;
        JsonObject pins = document["pins"].to<JsonObject>();
        pins["ledData"] = hardware.ledData;
        pins["oledSda"] = hardware.oledSda;
        pins["oledScl"] = hardware.oledScl;
        pins["button"] = hardware.button;
        JsonArray allowed = document["allowedPins"].to<JsonArray>();
        for (uint8_t pin = 0; pin < 40; ++pin)
        {
            if (SettingsService::isHardwarePinAllowed(pin)) allowed.add(pin);
        }
        String body;
        body.reserve(240);
        serializeJson(document, body);
        sendSecurityHeaders();
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json", body);
    }

    void handleHardwareSave()
    {
        Serial.println("WEB REQUEST: POST /api/hardware");
        if (!changesAllowed()) return;
        if (!server.hasArg("confirm") || server.arg("confirm") != "true")
        {
            sendError(400, "hardware change confirmation required");
            return;
        }

        HardwareSettings settings = {};
        if (!parseByteArgument("ledData", 39, settings.ledData) ||
            !parseByteArgument("oledSda", 39, settings.oledSda) ||
            !parseByteArgument("oledScl", 39, settings.oledScl) ||
            !parseByteArgument("button", 39, settings.button) ||
            !SettingsService::isHardwareSettingsValid(settings))
        {
            sendError(400, "invalid or conflicting hardware pins");
            return;
        }
        if (!SettingsService::saveHardware(settings))
        {
            sendError(500, "hardware settings could not be saved");
            return;
        }

        sendJson(200, "{\"ok\":true,\"rebootScheduled\":true}");
        rebootRequestedAt = millis();
        rebootScheduled = true;
        Serial.println("WEB HARDWARE APPLY: REBOOT SCHEDULED");
    }

    void handleNetworkReset()
    {
        Serial.println("WEB REQUEST: POST /api/network/reset");
        if (!changesAllowed()) return;
        if (!server.hasArg("confirm") || server.arg("confirm") != "true")
        {
            sendError(400, "network reset confirmation required");
            return;
        }
        if (!SettingsService::resetNetworkToDhcp())
        {
            sendError(500, "network settings could not be reset");
            return;
        }

        sendJson(200, "{\"ok\":true,\"rebootScheduled\":true}");
        rebootRequestedAt = millis();
        rebootScheduled = true;
        Serial.println("WEB NETWORK RESET: DHCP");
    }

    void handleReboot()
    {
        Serial.println("WEB REQUEST: POST /api/reboot");
        if (!changesAllowed()) return;
        if (!server.hasArg("confirm") || server.arg("confirm") != "true")
        {
            sendError(400, "reboot confirmation required");
            return;
        }

        sendJson(200, "{\"ok\":true}");
        rebootRequestedAt = millis();
        rebootScheduled = true;
        Serial.println("WEB REBOOT REQUESTED");
    }

    void configureRoutes()
    {
        server.on("/", HTTP_GET, handleRoot);
        server.on("/favicon.svg", HTTP_GET, handleFavicon);
        server.on("/api/status", HTTP_GET, handleStatus);
        server.on("/api/diagnostics", HTTP_GET, handleDiagnostics);
        server.on("/api/customization", HTTP_GET, handleCustomizationGet);
        server.on("/api/settings/export", HTTP_GET, handleSettingsExport);
        server.on("/api/settings/import", HTTP_POST, handleSettingsImport);
        server.on("/api/network", HTTP_GET, handleNetworkGet);
        server.on("/api/network", HTTP_POST, handleNetworkSave);
        server.on("/api/network/reset", HTTP_POST, handleNetworkReset);
        server.on("/api/hardware", HTTP_GET, handleHardwareGet);
        server.on("/api/hardware", HTTP_POST, handleHardwareSave);
        server.on("/api/mode", HTTP_POST, handleMode);
        server.on("/api/brightness", HTTP_POST, handleBrightness);
        server.on("/api/auto-night", HTTP_POST, handleAutoNight);
        server.on("/api/weather/refresh", HTTP_POST, handleWeatherRefresh);
        server.on("/api/weather/location", HTTP_POST, handleWeatherLocation);
        server.on("/api/calibration", HTTP_POST, handleCalibration);
        server.on("/api/custom-colors", HTTP_POST, handleCustomColors);
        server.on("/api/red-alert", HTTP_POST, handleRedAlertSettings);
        server.on("/api/red-alert/simulate", HTTP_POST, handleRedAlertSimulation);
        server.on("/api/ota", HTTP_POST, handleOtaSettings);
        server.on("/api/reboot", HTTP_POST, handleReboot);
        server.onNotFound([]() {
            Serial.printf("WEB REQUEST: %s %s\r\n", server.method() == HTTP_POST ? "POST" : "GET", server.uri().c_str());
            sendError(404, "not found");
        });
    }
}

void WebService::begin()
{
    started = false;
    routesConfigured = false;
    mdnsStarted = false;
    const char* headers[] = {"Origin", "X-Halo-Request"};
    server.collectHeaders(headers, 2);
    wifiWasConnected = WiFi.status() == WL_CONNECTED;
    rebootScheduled = false;
}

void WebService::update()
{
    const bool wifiConnected = WiFi.status() == WL_CONNECTED;
    if (!wifiConnected && started)
    {
        server.stop();
        started = false;
        Serial.println("WEB UI: STOPPED (WIFI LOST)");
    }
    if (!wifiConnected && mdnsStarted)
    {
        MDNS.end();
        mdnsStarted = false;
    }

    if (wifiConnected && !wifiWasConnected)
    {
        Serial.println("WEB UI: WIFI RESTORED; RESTARTING SERVER");
    }
    wifiWasConnected = wifiConnected;

    if (!started && wifiConnected)
    {
        if (!routesConfigured)
        {
            configureRoutes();
            routesConfigured = true;
        }
        server.begin();
        started = true;
        if (MDNS.begin(Product::HOSTNAME))
        {
            MDNS.addService("http", "tcp", Config::WEB_PORT);
            mdnsStarted = true;
            Serial.printf("MDNS READY: %s.local\r\n", Product::HOSTNAME);
        }
        else
        {
            Serial.println("MDNS ERROR: START FAILED");
        }
        Serial.println("WEB UI: STARTED");
        Serial.printf("WEB UI: http://%s.local/\r\n", Product::HOSTNAME);
    }

    if (started && wifiConnected)
    {
        server.handleClient();
    }

    if (rebootScheduled && millis() - rebootRequestedAt >= Config::WEB_REBOOT_DELAY_MS)
    {
        rebootScheduled = false;
        ESP.restart();
    }
}
