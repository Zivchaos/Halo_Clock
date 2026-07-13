#include "WebPortal.h"

#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WebServer.h>

#include "Notifications.h"
#include "Settings.h"
#include "TimeService.h"
#include "Version.h"
#include "Weather.h"
#include "WebAssets.h"

namespace
{
    WebServer server(80);
    bool started = false;

    void sendJson(const JsonDocument& document)
    {
        String payload;
        serializeJson(document, payload);
        server.send(200, "application/json", payload);
    }

    void handleStatus()
    {
        JsonDocument document;
        const tm& local = TimeService::localTime();
        char timeText[9] = "--:--:--";
        char dateText[24] = "Synchronizing";
        if (TimeService::isSynchronized())
        {
            strftime(timeText, sizeof(timeText), "%H:%M:%S", &local);
            strftime(dateText, sizeof(dateText), "%A, %d %B", &local);
        }
        document["time"] = timeText;
        document["date"] = dateText;
        document["uptime"] = millis() / 1000UL;
        document["heap"] = ESP.getFreeHeap();
        document["wifi"]["connected"] = TimeService::isWifiConnected();
        document["wifi"]["portal"] = TimeService::isPortalActive();
        document["wifi"]["ip"] = TimeService::ipAddress();
        document["wifi"]["rssi"] = TimeService::signalStrength();
        const WeatherData& weather = Weather::data();
        document["weather"]["valid"] = weather.valid;
        document["weather"]["temperature"] = weather.temperature;
        document["weather"]["humidity"] = weather.humidity;
        document["weather"]["description"] = Weather::description(weather.weatherCode);
        sendJson(document);
    }

    void handleGetSettings()
    {
        const HaloSettings& settings = Settings::get();
        JsonDocument document;
        document["theme"] = static_cast<uint8_t>(settings.theme);
        document["mode"] = static_cast<uint8_t>(settings.displayMode);
        document["dayBrightness"] = settings.dayBrightness;
        document["nightBrightness"] = settings.nightBrightness;
        document["nightStartHour"] = settings.nightStartHour;
        document["dayStartHour"] = settings.dayStartHour;
        document["hourWidth"] = settings.hourWidth;
        document["ledOffset"] = settings.ledOffset;
        document["clockwise"] = settings.clockwise;
        document["showTicks"] = settings.showTicks;
        document["showSeconds"] = settings.showSeconds;
        document["weatherEnabled"] = settings.weatherEnabled;
        document["latitude"] = settings.latitude;
        document["longitude"] = settings.longitude;
        document["timezone"] = settings.timezone;
        document["firmware"] = Version::FIRMWARE;
        sendJson(document);
    }

    void handleSaveSettings()
    {
        JsonDocument document;
        if (deserializeJson(document, server.arg("plain")))
        {
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        HaloSettings& settings = Settings::edit();
        settings.theme = static_cast<ThemeId>(document["theme"] | static_cast<uint8_t>(settings.theme));
        settings.displayMode = static_cast<DisplayMode>(document["mode"] | static_cast<uint8_t>(settings.displayMode));
        settings.dayBrightness = document["dayBrightness"] | settings.dayBrightness;
        settings.nightBrightness = document["nightBrightness"] | settings.nightBrightness;
        settings.nightStartHour = document["nightStartHour"] | settings.nightStartHour;
        settings.dayStartHour = document["dayStartHour"] | settings.dayStartHour;
        settings.hourWidth = document["hourWidth"] | settings.hourWidth;
        settings.ledOffset = document["ledOffset"] | settings.ledOffset;
        settings.clockwise = document["clockwise"] | settings.clockwise;
        settings.showTicks = document["showTicks"] | settings.showTicks;
        settings.showSeconds = document["showSeconds"] | settings.showSeconds;
        settings.weatherEnabled = document["weatherEnabled"] | settings.weatherEnabled;
        settings.latitude = document["latitude"] | settings.latitude;
        settings.longitude = document["longitude"] | settings.longitude;
        settings.timezone = document["timezone"] | settings.timezone;
        Settings::save();
        TimeService::reconfigureTime();
        Weather::refresh();
        Notifications::post(NotificationType::Success, "Settings saved", themeName(settings.theme), 1800);
        server.send(200, "application/json", "{\"ok\":true}");
    }

    void configureRoutes()
    {
        server.on("/", HTTP_GET, [] { server.send_P(200, "text/html", WebAssets::INDEX_HTML); });
        server.on("/logo.svg", HTTP_GET, [] { server.send_P(200, "image/svg+xml", WebAssets::LOGO_SVG); });
        server.on("/api/status", HTTP_GET, handleStatus);
        server.on("/api/settings", HTTP_GET, handleGetSettings);
        server.on("/api/settings", HTTP_POST, handleSaveSettings);
        server.on("/api/weather/refresh", HTTP_POST, [] {
            Weather::refresh();
            server.send(202, "application/json", "{\"ok\":true}");
        });
        server.on("/api/reboot", HTTP_POST, [] {
            server.send(200, "application/json", "{\"ok\":true}");
            delay(200);
            ESP.restart();
        });
        server.on("/api/wifi/reset", HTTP_POST, [] {
            server.send(200, "application/json", "{\"ok\":true}");
            TimeService::resetWifi();
        });
        server.on("/update", HTTP_GET, [] { server.send_P(200, "text/html", WebAssets::UPDATE_HTML); });
        server.on("/update", HTTP_POST,
            [] {
                const bool failed = Update.hasError();
                server.send(200, "text/plain", failed ? "Update failed. Return to HALO and try again." : "Update complete. HALO is restarting...");
                if (!failed)
                {
                    delay(300);
                    ESP.restart();
                }
            },
            [] {
                HTTPUpload& upload = server.upload();
                if (upload.status == UPLOAD_FILE_START)
                {
                    Notifications::post(NotificationType::Info, "Firmware update", "Installing...", 60000);
                    Update.begin(UPDATE_SIZE_UNKNOWN);
                }
                else if (upload.status == UPLOAD_FILE_WRITE)
                {
                    Update.write(upload.buf, upload.currentSize);
                }
                else if (upload.status == UPLOAD_FILE_END)
                {
                    Update.end(true);
                }
            });
        server.onNotFound([] { server.send(404, "application/json", "{\"error\":\"Not found\"}"); });
    }
}

void WebPortal::begin()
{
    configureRoutes();
}

void WebPortal::update()
{
    if (!TimeService::isWifiConnected())
    {
        return;
    }

    if (!started)
    {
        MDNS.begin("halo-clock");
        MDNS.addService("http", "tcp", 80);
        server.begin();
        started = true;
        Serial.println("[WEB] Dashboard: http://halo-clock.local");
        Notifications::post(NotificationType::Info, "Dashboard ready", "halo-clock.local", 2200);
    }
    server.handleClient();
}
