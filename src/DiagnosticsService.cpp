#include "DiagnosticsService.h"

#include <esp_ota_ops.h>
#include <esp_system.h>

#include "Version.h"

namespace
{
    portMUX_TYPE stateMux = portMUX_INITIALIZER_UNLOCKED;
    DiagnosticsData state;
    bool wifiStateKnown = false;
    bool everConnected = false;
    uint32_t lastTimeSyncAt = 0;

    void copyText(char* destination, size_t size, const char* source)
    {
        if (size == 0)
        {
            return;
        }
        snprintf(destination, size, "%s", source == nullptr ? "unknown" : source);
    }

    const char* resetReasonName(esp_reset_reason_t reason)
    {
        switch (reason)
        {
            case ESP_RST_POWERON: return "POWER_ON";
            case ESP_RST_EXT: return "EXTERNAL";
            case ESP_RST_SW: return "SOFTWARE";
            case ESP_RST_PANIC: return "PANIC";
            case ESP_RST_INT_WDT: return "INTERRUPT_WDT";
            case ESP_RST_TASK_WDT: return "TASK_WDT";
            case ESP_RST_WDT: return "OTHER_WDT";
            case ESP_RST_DEEPSLEEP: return "DEEP_SLEEP";
            case ESP_RST_BROWNOUT: return "BROWNOUT";
            case ESP_RST_SDIO: return "SDIO";
            case ESP_RST_UNKNOWN:
            default: return "UNKNOWN";
        }
    }
}

void DiagnosticsService::begin()
{
    const esp_partition_t* runningPartition = esp_ota_get_running_partition();

    portENTER_CRITICAL(&stateMux);
    state = DiagnosticsData{};
    copyText(state.resetReason, sizeof(state.resetReason), resetReasonName(esp_reset_reason()));
    copyText(
        state.runningPartition,
        sizeof(state.runningPartition),
        runningPartition == nullptr ? "unknown" : runningPartition->label);
    copyText(state.firmwareVersion, sizeof(state.firmwareVersion), Product::FIRMWARE_VERSION);
    copyText(state.buildDate, sizeof(state.buildDate), Product::BUILD_DATE);
    copyText(state.buildTime, sizeof(state.buildTime), Product::BUILD_TIME);
    wifiStateKnown = false;
    everConnected = false;
    lastTimeSyncAt = 0;
    portEXIT_CRITICAL(&stateMux);

    const DiagnosticsData boot = snapshot();
    Serial.printf("DIAGNOSTICS RESET REASON: %s\r\n", boot.resetReason);
    Serial.printf("DIAGNOSTICS RUNNING PARTITION: %s\r\n", boot.runningPartition);
    Serial.printf("DIAGNOSTICS FIRMWARE: %s\r\n", boot.firmwareVersion);
    Serial.printf("DIAGNOSTICS FREE HEAP: %lu\r\n", static_cast<unsigned long>(boot.freeHeap));
    Serial.printf(
        "DIAGNOSTICS MINIMUM FREE HEAP: %lu\r\n",
        static_cast<unsigned long>(boot.minimumFreeHeap));
}

void DiagnosticsService::update(
    bool connected,
    int32_t rssi,
    bool synchronized,
    bool otaReady,
    bool otaUpdating)
{
    portENTER_CRITICAL(&stateMux);
    if (!wifiStateKnown)
    {
        wifiStateKnown = true;
        everConnected = connected;
    }
    else if (connected && !state.wifiConnected)
    {
        if (everConnected)
        {
            ++state.wifiReconnectCount;
        }
        everConnected = true;
    }

    state.wifiConnected = connected;
    state.wifiRssi = connected ? rssi : 0;
    state.ntpSynchronized = synchronized;
    state.otaReady = otaReady;
    state.otaUpdating = otaUpdating;
    portEXIT_CRITICAL(&stateMux);
}

void DiagnosticsService::recordTimeSync()
{
    portENTER_CRITICAL(&stateMux);
    lastTimeSyncAt = millis();
    state.ntpSynchronized = true;
    portEXIT_CRITICAL(&stateMux);
}

void DiagnosticsService::recordWeatherRequest()
{
    portENTER_CRITICAL(&stateMux);
    ++state.weatherRequestCount;
    portEXIT_CRITICAL(&stateMux);
}

void DiagnosticsService::recordWeatherSuccess()
{
    portENTER_CRITICAL(&stateMux);
    ++state.weatherSuccessCount;
    portEXIT_CRITICAL(&stateMux);
}

void DiagnosticsService::recordWeatherFailure(const char* reason)
{
    portENTER_CRITICAL(&stateMux);
    ++state.weatherFailureCount;
    copyText(state.lastWeatherError, sizeof(state.lastWeatherError), reason);
    portEXIT_CRITICAL(&stateMux);
}

DiagnosticsData DiagnosticsService::snapshot()
{
    const uint32_t now = millis();
    portENTER_CRITICAL(&stateMux);
    DiagnosticsData result = state;
    const uint32_t synchronizedAt = lastTimeSyncAt;
    portEXIT_CRITICAL(&stateMux);

    result.uptimeSeconds = now / 1000UL;
    result.freeHeap = ESP.getFreeHeap();
    result.minimumFreeHeap = ESP.getMinFreeHeap();
    result.timeSyncAgeValid = synchronizedAt != 0;
    result.lastTimeSyncAgeSeconds = synchronizedAt == 0
        ? 0
        : (now - synchronizedAt) / 1000UL;
    return result;
}
