#include "WeatherService.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "Config.h"
#include "DiagnosticsService.h"
#include "SettingsService.h"
#include "WeatherProvider.h"

namespace
{
    SemaphoreHandle_t stateMutex = nullptr;
    WeatherData current;
    uint32_t nextAttemptAt = 0;
    uint32_t lastManualRefreshAt = 0;
    bool manualRefreshSeen = false;
    bool manualRefreshPending = false;
    bool displayUpdatePending = false;
    bool wifiStateKnown = false;
    bool previouslyConnected = false;
    constexpr time_t MINIMUM_VALID_EPOCH = 1704067200; // 2024-01-01 UTC

    void lockState()
    {
        if (stateMutex != nullptr)
        {
            xSemaphoreTake(stateMutex, portMAX_DELAY);
        }
    }

    void unlockState()
    {
        if (stateMutex != nullptr)
        {
            xSemaphoreGive(stateMutex);
        }
    }

    void copyText(char* destination, size_t size, const char* source)
    {
        if (size == 0)
        {
            return;
        }
        snprintf(destination, size, "%s", source == nullptr ? "unknown" : source);
    }

    void finishFailure(const char* reason)
    {
        DiagnosticsService::recordWeatherFailure(reason);
        const uint32_t now = millis();
        bool usingCache = false;
        lockState();
        WeatherPolicy::applyFailure(current, reason);
        nextAttemptAt = now + Config::WEATHER_FAILURE_BACKOFF_MS;
        usingCache = current.valid;
        displayUpdatePending = true;
        unlockState();

        Serial.printf("WEATHER ERROR: %s\r\n", reason);
        if (usingCache)
        {
            Serial.println("WEATHER: USING CACHE");
        }
    }

    void finishSuccess(const WeatherReading& reading)
    {
        DiagnosticsService::recordWeatherSuccess();
        const uint32_t now = millis();
        const time_t epoch = time(nullptr);
        lockState();
        current.valid = true;
        current.stale = false;
        current.apparentTemperatureAvailable = reading.apparentTemperatureAvailable;
        current.updating = false;
        current.temperature = reading.temperature;
        current.apparentTemperature = reading.apparentTemperature;
        current.humidity = reading.humidity;
        current.windSpeed = reading.windSpeed;
        current.conditionCode = reading.conditionCode;
        current.lastSuccessfulUpdateMs = now;
        current.lastSuccessfulUpdateEpoch = epoch >= MINIMUM_VALID_EPOCH ? epoch : 0;
        copyText(current.condition, sizeof(current.condition), reading.condition);
        current.error[0] = '\0';
        nextAttemptAt = now + Config::WEATHER_REFRESH_INTERVAL_MS;
        displayUpdatePending = true;
        unlockState();
        Serial.println("WEATHER: UPDATED");
    }

    void performWeatherRequest()
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            finishFailure("offline");
            return;
        }

        char url[512];
        const WeatherLocationSettings location = SettingsService::weatherLocation();
        if (!WeatherProvider::buildRequestUrl(url, sizeof(url), location.latitude, location.longitude))
        {
            finishFailure("request URL too long");
            return;
        }

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;
        http.setConnectTimeout(Config::WEATHER_CONNECT_TIMEOUT_MS);
        http.setTimeout(Config::WEATHER_RESPONSE_TIMEOUT_MS);
        if (!http.begin(client, url))
        {
            finishFailure("HTTPS setup failed");
            return;
        }

        const int statusCode = http.GET();
        if (statusCode != HTTP_CODE_OK)
        {
            char reason[64];
            if (statusCode < 0)
            {
                const String description = HTTPClient::errorToString(statusCode);
                snprintf(reason, sizeof(reason), "request failed: %s", description.c_str());
            }
            else
            {
                snprintf(reason, sizeof(reason), "HTTP %d", statusCode);
            }
            http.end();
            finishFailure(reason);
            return;
        }

        const int responseSize = http.getSize();
        if (responseSize > static_cast<int>(Config::WEATHER_MAX_RESPONSE_BYTES))
        {
            http.end();
            finishFailure("response too large");
            return;
        }

        const String payload = http.getString();
        http.end();
        if (payload.isEmpty() || payload.length() > Config::WEATHER_MAX_RESPONSE_BYTES)
        {
            finishFailure("empty or oversized response");
            return;
        }

        WeatherReading reading;
        const WeatherParseResult parseResult = WeatherProvider::parseResponse(
            payload.c_str(),
            payload.length(),
            reading);
        if (parseResult != WeatherParseResult::OK)
        {
            finishFailure(WeatherProvider::parseError(parseResult));
            return;
        }

        finishSuccess(reading);
    }

    void weatherTask(void*)
    {
        // Keep resource-owning C++ objects inside a normal function scope so
        // their destructors run before the FreeRTOS task deletes itself.
        performWeatherRequest();
        vTaskDelete(nullptr);
    }

    bool startRequest()
    {
        const uint32_t now = millis();
        lockState();
        if (current.updating)
        {
            unlockState();
            return false;
        }
        current.updating = true;
        current.lastAttemptMs = now;
        manualRefreshPending = false;
        unlockState();

        Serial.println("WEATHER: REQUEST");
        DiagnosticsService::recordWeatherRequest();
        if (xTaskCreate(
                weatherTask,
                "halo-weather",
                Config::WEATHER_TASK_STACK_SIZE,
                nullptr,
                1,
                nullptr) != pdPASS)
        {
            finishFailure("task start failed");
            return false;
        }
        return true;
    }
}

void WeatherService::begin()
{
    if (stateMutex == nullptr)
    {
        stateMutex = xSemaphoreCreateMutex();
    }

    lockState();
    current = WeatherData{};
    nextAttemptAt = millis() + Config::WEATHER_STARTUP_DELAY_MS;
    lastManualRefreshAt = 0;
    manualRefreshSeen = false;
    manualRefreshPending = false;
    displayUpdatePending = false;
    wifiStateKnown = false;
    previouslyConnected = false;
    unlockState();
}

void WeatherService::update()
{
    const uint32_t now = millis();
    const time_t epoch = time(nullptr);
    const bool connected = WiFi.status() == WL_CONNECTED;
    bool logOffline = false;
    bool logStale = false;
    bool shouldStart = false;

    lockState();
    if (!wifiStateKnown || connected != previouslyConnected)
    {
        wifiStateKnown = true;
        previouslyConnected = connected;
        if (!connected)
        {
            copyText(current.error, sizeof(current.error), "offline");
            logOffline = true;
        }
        else
        {
            current.error[0] = '\0';
            nextAttemptAt = now + Config::WEATHER_STARTUP_DELAY_MS;
        }
    }

    if (WeatherPolicy::isStale(
            current.valid,
            now,
            current.lastSuccessfulUpdateMs,
            Config::WEATHER_STALE_AFTER_MS) && !current.stale)
    {
        current.stale = true;
        displayUpdatePending = true;
        logStale = true;
    }

    if (current.valid &&
        current.lastSuccessfulUpdateEpoch == 0 &&
        epoch >= MINIMUM_VALID_EPOCH)
    {
        current.lastSuccessfulUpdateEpoch = WeatherPolicy::estimateUpdateEpoch(
            epoch,
            now,
            current.lastSuccessfulUpdateMs);
    }

    shouldStart = WeatherPolicy::shouldStartRequest(
        connected,
        current.updating,
        manualRefreshPending,
        now,
        nextAttemptAt);
    unlockState();

    if (logOffline)
    {
        Serial.println("WEATHER ERROR: offline");
    }
    if (logStale)
    {
        Serial.println("WEATHER: STALE");
    }
    if (shouldStart)
    {
        startRequest();
    }
}

WeatherData WeatherService::snapshot()
{
    lockState();
    const WeatherData result = current;
    unlockState();
    return result;
}

WeatherRefreshResult WeatherService::requestRefresh()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        return WeatherRefreshResult::OFFLINE;
    }

    const uint32_t now = millis();
    lockState();
    if (manualRefreshSeen &&
        now - lastManualRefreshAt < Config::WEATHER_MANUAL_REFRESH_LIMIT_MS)
    {
        unlockState();
        return WeatherRefreshResult::RATE_LIMITED;
    }
    if (current.updating || manualRefreshPending)
    {
        unlockState();
        return WeatherRefreshResult::BUSY;
    }

    manualRefreshSeen = true;
    lastManualRefreshAt = now;
    manualRefreshPending = true;
    unlockState();
    return WeatherRefreshResult::ACCEPTED;
}

bool WeatherService::locationChanged()
{
    lockState();
    if (current.updating)
    {
        unlockState();
        return false;
    }
    manualRefreshPending = true;
    nextAttemptAt = millis();
    unlockState();
    return true;
}

bool WeatherService::consumeDisplayUpdate()
{
    lockState();
    const bool pending = displayUpdatePending;
    displayUpdatePending = false;
    unlockState();
    return pending;
}
