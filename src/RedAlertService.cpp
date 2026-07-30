#include "RedAlertService.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <cstring>

#include "Config.h"
#include "RedAlertPolicy.h"
#include "RedAlertProvider.h"
#include "SettingsService.h"
#include "TimeService.h"
#include "TrustedCertificates.h"
#include "WeatherService.h"

namespace
{
    // Community research credits:
    // https://github.com/dmatik/oref-alerts-proxy-ms
    // https://github.com/moshepinhasi/Red_alert_monitor
    // HALO's implementation is independent; official Home Front Command
    // alerts and instructions always take precedence.
    SemaphoreHandle_t mutex = nullptr;
    RedAlertData current;
    uint32_t nextAttemptAt = 0;
    uint32_t simulationUntil = 0;
    uint32_t requestHeapFloor = 0;
    bool lowHeapSuspended = false;

    void copyText(char* destination, size_t size, const char* source)
    {
        snprintf(destination, size, "%s", source == nullptr ? "" : source);
    }

    void recordRequestCompletion(bool success)
    {
        const uint32_t now = millis();
        current.lastRequestDurationMs = current.lastRequestStartedMs == 0 ? 0 : now - current.lastRequestStartedMs;
        current.lastRequestFreeHeap = ESP.getFreeHeap();
        current.lastRequestMinimumFreeHeap = requestHeapFloor == 0
            ? current.lastRequestFreeHeap
            : requestHeapFloor;
        if (success)
        {
            ++current.successCount;
        }
        else
        {
            ++current.failureCount;
        }
    }

    void observeRequestHeap()
    {
        const uint32_t freeHeap = ESP.getFreeHeap();
        if (requestHeapFloor == 0 || freeHeap < requestHeapFloor)
        {
            requestHeapFloor = freeHeap;
        }
    }

    void finishFailure(const char* reason)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        current.updating = false;
        current.stale = true;
        current.active = false;
        if (current.consecutiveFailures < UINT8_MAX) ++current.consecutiveFailures;
        recordRequestCompletion(false);
        copyText(current.error, sizeof(current.error), reason);
        nextAttemptAt = RedAlertPolicy::nextAttemptAfterFailure(
            millis(), Config::RED_ALERT_FAILURE_BACKOFF_MS);
        xSemaphoreGive(mutex);
        Serial.printf("RED ALERT ERROR: %s\r\n", reason);
    }

    void suspendForLowHeap()
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        current.updating = false;
        current.active = false;
        current.stale = true;
        lowHeapSuspended = true;
        copyText(current.error, sizeof(current.error), "live polling suspended: low heap");
        xSemaphoreGive(mutex);
        Serial.printf("RED ALERT: polling suspended at %lu free heap\r\n", static_cast<unsigned long>(ESP.getFreeHeap()));
    }

    void performRequest()
    {
        WiFiClientSecure relayClient;
        observeRequestHeap();
        relayClient.setCACert(TrustedCertificates::DEFAULT_PROVIDER_ROOTS);
        HTTPClient http;
        observeRequestHeap();
        http.setConnectTimeout(Config::RED_ALERT_CONNECT_TIMEOUT_MS);
        http.setTimeout(Config::RED_ALERT_RESPONSE_TIMEOUT_MS);
        const char* relayUrl = SettingsService::redAlert().relayUrl;
        if (!http.begin(relayClient, relayUrl)) { finishFailure("relay HTTPS setup failed"); return; }
        observeRequestHeap();
        // Always close this community-relay connection after a complete
        // response. A long-lived TLS session was observed to stall Wi-Fi on
        // the target ESP32; the heap guard limits per-request churn instead.
        http.useHTTP10(true);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        http.addHeader("Referer", "https://www.oref.org.il/");
        http.addHeader("User-Agent", "HALO-CST/1.0");
        http.addHeader("X-Requested-With", "XMLHttpRequest");
        http.addHeader("Accept", "application/json");
        const int status = http.GET();
        observeRequestHeap();
        if (status != HTTP_CODE_OK)
        {
            char reason[64];
            snprintf(reason, sizeof(reason), "alert source HTTP %d", status);
            http.end(); finishFailure(reason); return;
        }
        String payload = http.getString();
        observeRequestHeap();
        http.end();
        if (payload.length() >= 3 && static_cast<uint8_t>(payload[0]) == 0xEFU &&
            static_cast<uint8_t>(payload[1]) == 0xBBU && static_cast<uint8_t>(payload[2]) == 0xBFU)
        {
            payload.remove(0, 3);
        }
        if (payload.length() > Config::RED_ALERT_MAX_RESPONSE_BYTES) { finishFailure("response too large"); return; }
        if (payload.isEmpty())
        {
            xSemaphoreTake(mutex, portMAX_DELAY);
            current.updating = false; current.stale = false; current.active = false;
            current.id[0] = current.title[0] = current.areas[0] = current.error[0] = '\0';
            current.consecutiveFailures = 0;
            recordRequestCompletion(true);
            current.lastSuccessfulUpdateMs = millis();
            nextAttemptAt = RedAlertPolicy::nextAttemptAfterSuccess(
                millis(), Config::RED_ALERT_POLL_INTERVAL_MS);
            xSemaphoreGive(mutex); return;
        }
        RedAlertReading reading;
        if (RedAlertProvider::parseResponse(payload.c_str(), payload.length(), SettingsService::redAlert().locations, reading) != RedAlertParseResult::OK)
        {
            finishFailure("invalid alert response"); return;
        }
        xSemaphoreTake(mutex, portMAX_DELAY);
        current.updating = false;
        current.stale = false;
        current.lastSuccessfulUpdateMs = millis();
        current.consecutiveFailures = 0;
        recordRequestCompletion(true);
        current.active = reading.sourceAlert && reading.matchesSelection;
        if (current.active)
        {
            copyText(current.id, sizeof(current.id), reading.id);
            copyText(current.title, sizeof(current.title), reading.title);
            copyText(current.areas, sizeof(current.areas), reading.areas);
        }
        else { current.id[0] = current.title[0] = current.areas[0] = '\0'; }
        current.error[0] = '\0';
        nextAttemptAt = RedAlertPolicy::nextAttemptAfterSuccess(
            millis(), Config::RED_ALERT_POLL_INTERVAL_MS);
        xSemaphoreGive(mutex);
    }

    void requestTask(void*)
    {
        // The HTTPS client and response String must leave their C++ scopes
        // before this task is deleted. Calling vTaskDelete() directly from
        // performRequest() bypasses their destructors and leaks heap on every
        // otherwise successful poll.
        performRequest();
        vTaskDelete(nullptr);
    }
}

void RedAlertService::begin()
{
    if (mutex == nullptr) mutex = xSemaphoreCreateMutex();
    xSemaphoreTake(mutex, portMAX_DELAY);
    current = RedAlertData{};
    current.enabled = SettingsService::redAlert().enabled;
    copyText(current.error, sizeof(current.error), current.enabled ? "waiting for Wi-Fi" : "disabled");
    nextAttemptAt = millis();
    lowHeapSuspended = false;
    xSemaphoreGive(mutex);
}

void RedAlertService::update()
{
    const RedAlertSettings& settings = SettingsService::redAlert();
    const bool enabled = settings.enabled;
    xSemaphoreTake(mutex, portMAX_DELAY);
    current.enabled = enabled;
    if constexpr (!Config::RED_ALERT_LIVE_POLLING_ENABLED)
    {
        if (simulationUntil == 0)
        {
            current.active = false;
            current.stale = false;
            current.updating = false;
            copyText(current.error, sizeof(current.error), enabled ? "live polling paused for stability" : "disabled");
        }
        xSemaphoreGive(mutex);
        return;
    }
    if ((!enabled || settings.relayUrl[0] == '\0') && simulationUntil == 0)
    {
        current.active = false;
        current.stale = false;
        current.updating = false;
        copyText(current.error, sizeof(current.error), enabled ? "relay URL not configured" : "disabled");
    }
    if (simulationUntil != 0 && static_cast<int32_t>(millis() - simulationUntil) >= 0)
    {
        simulationUntil = 0;
        current.active = false;
    }
    const bool due = RedAlertPolicy::shouldStartRequest(
        enabled,
        settings.relayUrl[0] != '\0',
        simulationUntil != 0,
        lowHeapSuspended,
        current.updating,
        millis(),
        nextAttemptAt);
    if (due) current.updating = true;
    xSemaphoreGive(mutex);
    if (!due) return;
    if (RedAlertPolicy::shouldDeferForMinuteTransition(
            TimeService::isSynchronized(),
            static_cast<uint8_t>(TimeService::localTime().tm_sec),
            Config::RED_ALERT_MINUTE_TRANSITION_GUARD_SECONDS))
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        current.updating = false;
        nextAttemptAt = millis() + 1000UL;
        xSemaphoreGive(mutex);
        return;
    }
    xSemaphoreTake(mutex, portMAX_DELAY);
    ++current.requestCount;
    current.lastRequestStartedMs = millis();
    requestHeapFloor = ESP.getFreeHeap();
    xSemaphoreGive(mutex);
    if (WiFi.status() != WL_CONNECTED) { finishFailure("offline"); return; }
    if (WeatherService::snapshot().updating)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        current.updating = false;
        nextAttemptAt = millis() + 1000UL;
        xSemaphoreGive(mutex);
        return;
    }
    if (RedAlertPolicy::shouldSuspendForLowHeap(
            ESP.getFreeHeap(), Config::RED_ALERT_MIN_FREE_HEAP_BYTES))
    {
        suspendForLowHeap();
        return;
    }
    if (xTaskCreatePinnedToCore(
            requestTask,
            "halo-alert",
            Config::RED_ALERT_TASK_STACK_SIZE,
            nullptr,
            1,
            nullptr,
            Config::RED_ALERT_TASK_CORE) != pdPASS)
    {
        finishFailure("task start failed");
    }
}

RedAlertData RedAlertService::snapshot()
{
    xSemaphoreTake(mutex, portMAX_DELAY); const RedAlertData copy = current; xSemaphoreGive(mutex); return copy;
}

bool RedAlertService::settingsChanged()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    current.active = false;
    current.stale = false;
    lowHeapSuspended = false;
    nextAttemptAt = millis();
    xSemaphoreGive(mutex);
    return true;
}

bool RedAlertService::simulate()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    current.active = true; current.stale = false; copyText(current.id, sizeof(current.id), "simulation");
    copyText(current.title, sizeof(current.title), "TEST ALERT"); copyText(current.areas, sizeof(current.areas), "Simulation only");
    simulationUntil = millis() + 5000;
    xSemaphoreGive(mutex); return true;
}
