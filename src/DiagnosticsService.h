#pragma once

#include <Arduino.h>

struct DiagnosticsData
{
    uint32_t uptimeSeconds = 0;
    uint32_t freeHeap = 0;
    uint32_t minimumFreeHeap = 0;
    char resetReason[24] = "UNKNOWN";
    uint32_t wifiReconnectCount = 0;
    int32_t wifiRssi = 0;
    bool wifiConnected = false;
    bool ntpSynchronized = false;
    bool timeSyncAgeValid = false;
    uint32_t lastTimeSyncAgeSeconds = 0;
    uint32_t weatherRequestCount = 0;
    uint32_t weatherSuccessCount = 0;
    uint32_t weatherFailureCount = 0;
    char lastWeatherError[64] = "";
    bool otaReady = false;
    bool otaUpdating = false;
    char runningPartition[16] = "unknown";
    char firmwareVersion[24] = "unknown";
    char buildDate[16] = "unknown";
    char buildTime[16] = "unknown";
};

class DiagnosticsService
{
public:
    static void begin();
    static void update(
        bool wifiConnected,
        int32_t wifiRssi,
        bool ntpSynchronized,
        bool otaReady,
        bool otaUpdating);
    static void recordTimeSync();
    static void recordWeatherRequest();
    static void recordWeatherSuccess();
    static void recordWeatherFailure(const char* reason);
    static DiagnosticsData snapshot();
};
