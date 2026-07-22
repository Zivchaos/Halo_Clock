#include "WebService.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebServer.h>
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
    bool rebootScheduled = false;
    uint32_t rebootRequestedAt = 0;

    const char* boolText(bool value)
    {
        return value ? "true" : "false";
    }

    void sendJson(int statusCode, const char* body)
    {
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

    void handleFavicon()
    {
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

        const AutoNightSettings& autoNight = SettingsService::autoNight();
        const WeatherData weather = WeatherService::snapshot();
        const RedAlertData redAlert = RedAlertService::snapshot();
        char startText[8];
        char endText[8];
        snprintf(startText, sizeof(startText), "%02u:%02u", autoNight.startHour, autoNight.startMinute);
        snprintf(endText, sizeof(endText), "%02u:%02u", autoNight.endHour, autoNight.endMinute);
        const String ipAddress = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "0.0.0.0";

        char temperatureText[16] = "null";
        char apparentTemperatureText[16] = "null";
        char humidityText[8] = "null";
        char windSpeedText[16] = "null";
        if (weather.valid)
        {
            snprintf(temperatureText, sizeof(temperatureText), "%.1f", static_cast<double>(weather.temperature));
            snprintf(humidityText, sizeof(humidityText), "%u", weather.humidity);
            snprintf(windSpeedText, sizeof(windSpeedText), "%.1f", static_cast<double>(weather.windSpeed));
            if (weather.apparentTemperatureAvailable)
            {
                snprintf(
                    apparentTemperatureText,
                    sizeof(apparentTemperatureText),
                    "%.1f",
                    static_cast<double>(weather.apparentTemperature));
            }
        }

        const NetworkStatus network = NetworkService::snapshot();
        char body[2600];
        snprintf(
            body,
            sizeof(body),
            "{\"time\":\"%s\",\"wifiConnected\":%s,\"ip\":\"%s\",\"rssi\":%ld,"
            "\"uptimeSeconds\":%lu,\"freeHeap\":%lu,\"minimumFreeHeap\":%lu,"
            "\"firmwareVersion\":\"%s\",\"resetReason\":\"%s\",\"runningPartition\":\"%s\","
            "\"wifiReconnectCount\":%lu,\"ntpSynchronized\":%s,\"timeSyncAgeValid\":%s,"
            "\"lastTimeSyncAgeSeconds\":%lu,\"weatherRequestCount\":%lu,"
            "\"weatherSuccessCount\":%lu,\"weatherFailureCount\":%lu,"
            "\"selectedMode\":\"%s\",\"effectiveMode\":\"%s\","
            "\"brightness\":%u,\"autoNightEnabled\":%s,\"autoNightActive\":%s,"
            "\"autoNightStart\":\"%s\",\"autoNightEnd\":\"%s\",\"manualOverride\":%s,"
            "\"otaReady\":%s,\"otaUpdating\":%s,"
            "\"networkConfiguredMode\":\"%s\",\"networkEffectiveMode\":\"%s\","
            "\"dhcpFallbackActive\":%s,\"staticConnectionFailureCount\":%u,"
            "\"weatherAvailable\":%s,\"weatherStale\":%s,\"temperature\":%s,"
            "\"apparentTemperature\":%s,\"condition\":\"%s\",\"humidity\":%s,"
            "\"windSpeed\":%s,\"weatherLastUpdate\":%lu,\"weatherError\":\"%s\","
            "\"redAlertEnabled\":%s,\"redAlertActive\":%s,\"redAlertStale\":%s,"
            "\"redAlertUpdating\":%s,\"redAlertAreas\":\"%s\",\"redAlertError\":\"%s\","
            "\"redAlertHasSuccessfulCheck\":%s,\"redAlertLastCheckAgeSeconds\":%lu,\"redAlertFailureCount\":%u}",
            timeText,
            boolText(WiFi.status() == WL_CONNECTED),
            ipAddress.c_str(),
            static_cast<long>(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0),
            static_cast<unsigned long>(diagnostics.uptimeSeconds),
            static_cast<unsigned long>(diagnostics.freeHeap),
            static_cast<unsigned long>(diagnostics.minimumFreeHeap),
            diagnostics.firmwareVersion,
            diagnostics.resetReason,
            diagnostics.runningPartition,
            static_cast<unsigned long>(diagnostics.wifiReconnectCount),
            boolText(diagnostics.ntpSynchronized),
            boolText(diagnostics.timeSyncAgeValid),
            static_cast<unsigned long>(diagnostics.lastTimeSyncAgeSeconds),
            static_cast<unsigned long>(diagnostics.weatherRequestCount),
            static_cast<unsigned long>(diagnostics.weatherSuccessCount),
            static_cast<unsigned long>(diagnostics.weatherFailureCount),
            DisplayModes::name(SettingsService::displayMode()),
            DisplayModes::name(Clock::effectiveMode()),
            SettingsService::brightness(),
            boolText(autoNight.enabled),
            boolText(AutoNightService::isActive()),
            startText,
            endText,
            boolText(AutoNightService::isManualOverride()),
            boolText(OtaService::isReady()),
            boolText(OtaService::isUpdating()),
            NetworkConfig::modeName(network.configuredMode),
            NetworkConfig::modeName(network.effectiveMode),
            boolText(network.dhcpFallbackActive),
            network.staticConnectionFailureCount,
            boolText(weather.valid),
            boolText(weather.stale),
            temperatureText,
            apparentTemperatureText,
            weather.condition,
            humidityText,
            windSpeedText,
            static_cast<unsigned long>(weather.lastSuccessfulUpdateEpoch),
            weather.error,
            boolText(redAlert.enabled), boolText(redAlert.active), boolText(redAlert.stale),
            boolText(redAlert.updating), redAlert.areas, redAlert.error,
            boolText(redAlert.lastSuccessfulUpdateMs != 0),
            static_cast<unsigned long>(redAlert.lastSuccessfulUpdateMs == 0 ? 0 : (millis() - redAlert.lastSuccessfulUpdateMs) / 1000UL),
            redAlert.consecutiveFailures);
        sendJson(200, body);
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
        server.on("/api/network", HTTP_GET, handleNetworkGet);
        server.on("/api/network", HTTP_POST, handleNetworkSave);
        server.on("/api/network/reset", HTTP_POST, handleNetworkReset);
        server.on("/api/mode", HTTP_POST, handleMode);
        server.on("/api/brightness", HTTP_POST, handleBrightness);
        server.on("/api/auto-night", HTTP_POST, handleAutoNight);
        server.on("/api/weather/refresh", HTTP_POST, handleWeatherRefresh);
        server.on("/api/weather/location", HTTP_POST, handleWeatherLocation);
        server.on("/api/calibration", HTTP_POST, handleCalibration);
        server.on("/api/custom-colors", HTTP_POST, handleCustomColors);
        server.on("/api/red-alert", HTTP_POST, handleRedAlertSettings);
        server.on("/api/red-alert/simulate", HTTP_POST, handleRedAlertSimulation);
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
