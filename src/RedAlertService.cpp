#include "RedAlertService.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <cstring>

#include "Config.h"
#include "RedAlertProvider.h"
#include "SettingsService.h"

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

    void copyText(char* destination, size_t size, const char* source)
    {
        snprintf(destination, size, "%s", source == nullptr ? "" : source);
    }

    void finishFailure(const char* reason)
    {
        xSemaphoreTake(mutex, portMAX_DELAY);
        current.updating = false;
        current.stale = true;
        current.active = false;
        if (current.consecutiveFailures < UINT8_MAX) ++current.consecutiveFailures;
        copyText(current.error, sizeof(current.error), reason);
        nextAttemptAt = millis() + Config::RED_ALERT_FAILURE_BACKOFF_MS;
        xSemaphoreGive(mutex);
        Serial.printf("RED ALERT ERROR: %s\r\n", reason);
    }

    void requestTask(void*)
    {
        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;
        http.setConnectTimeout(Config::RED_ALERT_CONNECT_TIMEOUT_MS);
        http.setTimeout(Config::RED_ALERT_RESPONSE_TIMEOUT_MS);
        const char* relayUrl = SettingsService::redAlert().relayUrl;
        if (!http.begin(client, relayUrl)) { finishFailure("relay HTTPS setup failed"); vTaskDelete(nullptr); return; }
        // Do not leave a persistent CDN connection open.  This makes small
        // responses such as Tzeva Adom's `[]` deterministic on the ESP32.
        http.useHTTP10(true);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        http.addHeader("Referer", "https://www.oref.org.il/");
        http.addHeader("User-Agent", "HALO-CST/1.0");
        http.addHeader("X-Requested-With", "XMLHttpRequest");
        http.addHeader("Accept", "application/json");
        const int status = http.GET();
        if (status != HTTP_CODE_OK)
        {
            char reason[64];
            snprintf(reason, sizeof(reason), "alert source HTTP %d", status);
            http.end(); finishFailure(reason); vTaskDelete(nullptr); return;
        }
        String payload = http.getString();
        http.end();
        if (payload.length() >= 3 && static_cast<uint8_t>(payload[0]) == 0xEFU &&
            static_cast<uint8_t>(payload[1]) == 0xBBU && static_cast<uint8_t>(payload[2]) == 0xBFU)
        {
            payload.remove(0, 3);
        }
        if (payload.length() > Config::RED_ALERT_MAX_RESPONSE_BYTES) { finishFailure("response too large"); vTaskDelete(nullptr); return; }
        if (payload.isEmpty())
        {
            xSemaphoreTake(mutex, portMAX_DELAY);
            current.updating = false; current.stale = false; current.active = false;
            current.id[0] = current.title[0] = current.areas[0] = current.error[0] = '\0';
            current.consecutiveFailures = 0;
            current.lastSuccessfulUpdateMs = millis(); nextAttemptAt = millis() + Config::RED_ALERT_POLL_INTERVAL_MS;
            xSemaphoreGive(mutex); vTaskDelete(nullptr); return;
        }
        RedAlertReading reading;
        if (RedAlertProvider::parseResponse(payload.c_str(), payload.length(), SettingsService::redAlert().locations, reading) != RedAlertParseResult::OK)
        {
            finishFailure("invalid alert response"); vTaskDelete(nullptr); return;
        }
        xSemaphoreTake(mutex, portMAX_DELAY);
        current.updating = false;
        current.stale = false;
        current.lastSuccessfulUpdateMs = millis();
        current.consecutiveFailures = 0;
        current.active = reading.sourceAlert && reading.matchesSelection;
        if (current.active)
        {
            copyText(current.id, sizeof(current.id), reading.id);
            copyText(current.title, sizeof(current.title), reading.title);
            copyText(current.areas, sizeof(current.areas), reading.areas);
        }
        else { current.id[0] = current.title[0] = current.areas[0] = '\0'; }
        current.error[0] = '\0';
        nextAttemptAt = millis() + Config::RED_ALERT_POLL_INTERVAL_MS;
        xSemaphoreGive(mutex);
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
    xSemaphoreGive(mutex);
}

void RedAlertService::update()
{
    const RedAlertSettings& settings = SettingsService::redAlert();
    const bool enabled = settings.enabled;
    xSemaphoreTake(mutex, portMAX_DELAY);
    current.enabled = enabled;
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
    const bool due = enabled && settings.relayUrl[0] != '\0' && simulationUntil == 0 && !current.updating && static_cast<int32_t>(millis() - nextAttemptAt) >= 0;
    if (due) current.updating = true;
    xSemaphoreGive(mutex);
    if (!due) return;
    if (WiFi.status() != WL_CONNECTED) { finishFailure("offline"); return; }
    if (xTaskCreate(requestTask, "halo-alert", Config::RED_ALERT_TASK_STACK_SIZE, nullptr, 1, nullptr) != pdPASS) finishFailure("task start failed");
}

RedAlertData RedAlertService::snapshot()
{
    xSemaphoreTake(mutex, portMAX_DELAY); const RedAlertData copy = current; xSemaphoreGive(mutex); return copy;
}

bool RedAlertService::settingsChanged()
{
    xSemaphoreTake(mutex, portMAX_DELAY); current.active = false; current.stale = false; nextAttemptAt = millis(); xSemaphoreGive(mutex); return true;
}

bool RedAlertService::simulate()
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    current.active = true; current.stale = false; copyText(current.id, sizeof(current.id), "simulation");
    copyText(current.title, sizeof(current.title), "TEST ALERT"); copyText(current.areas, sizeof(current.areas), "Simulation only");
    simulationUntil = millis() + 5000;
    xSemaphoreGive(mutex); return true;
}
