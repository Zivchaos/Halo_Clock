#pragma once

#include <Arduino.h>
#include <cstdio>
#include <time.h>

struct WeatherData
{
    bool valid = false;
    bool stale = false;
    bool apparentTemperatureAvailable = false;
    bool updating = false;
    float temperature = 0.0F;
    float apparentTemperature = 0.0F;
    uint8_t humidity = 0;
    float windSpeed = 0.0F;
    int16_t conditionCode = -1;
    uint32_t lastSuccessfulUpdateMs = 0;
    uint32_t lastAttemptMs = 0;
    time_t lastSuccessfulUpdateEpoch = 0;
    char condition[24] = "Unavailable";
    char error[64] = "waiting for Wi-Fi";
};

enum class WeatherRefreshResult : uint8_t
{
    ACCEPTED,
    RATE_LIMITED,
    BUSY,
    OFFLINE
};

namespace WeatherPolicy
{
    constexpr bool isStale(bool valid, uint32_t now, uint32_t updatedAt, uint32_t staleAfter)
    {
        return valid && now - updatedAt >= staleAfter;
    }

    constexpr bool isDue(uint32_t now, uint32_t dueAt)
    {
        return static_cast<int32_t>(now - dueAt) >= 0;
    }

    constexpr bool shouldStartRequest(
        bool connected,
        bool updating,
        bool manualPending,
        uint32_t now,
        uint32_t dueAt)
    {
        return connected && !updating && (manualPending || isDue(now, dueAt));
    }

    constexpr time_t estimateUpdateEpoch(
        time_t currentEpoch,
        uint32_t now,
        uint32_t updatedAt)
    {
        const time_t ageSeconds = static_cast<time_t>((now - updatedAt) / 1000UL);
        return currentEpoch > ageSeconds ? currentEpoch - ageSeconds : currentEpoch;
    }

    inline void applyFailure(WeatherData& data, const char* reason)
    {
        data.updating = false;
        std::snprintf(
            data.error,
            sizeof(data.error),
            "%s",
            reason == nullptr ? "unknown" : reason);
    }
}

class WeatherService
{
public:
    static void begin();
    static void update();
    static WeatherData snapshot();
    static WeatherRefreshResult requestRefresh();
    static bool locationChanged();
    static bool consumeDisplayUpdate();
};
