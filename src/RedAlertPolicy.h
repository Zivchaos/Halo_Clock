#pragma once

#include <stdint.h>

// Pure scheduling rules for the optional Red Alert visual aid. Keeping these
// independent of Wi-Fi and HTTP makes the safety decisions testable without
// contacting a provider or touching the physical device.
namespace RedAlertPolicy
{
    constexpr bool isDue(uint32_t now, uint32_t scheduledAt)
    {
        return static_cast<int32_t>(now - scheduledAt) >= 0;
    }

    constexpr bool shouldStartRequest(
        bool enabled,
        bool relayConfigured,
        bool simulationActive,
        bool lowHeapSuspended,
        bool requestInProgress,
        uint32_t now,
        uint32_t nextAttemptAt)
    {
        return enabled && relayConfigured && !simulationActive &&
            !lowHeapSuspended && !requestInProgress && isDue(now, nextAttemptAt);
    }

    constexpr bool shouldSuspendForLowHeap(uint32_t freeHeap, uint32_t minimumFreeHeap)
    {
        return freeHeap < minimumFreeHeap;
    }

    constexpr bool shouldDeferForMinuteTransition(
        bool timeSynchronized,
        uint8_t second,
        uint8_t guardSeconds)
    {
        return timeSynchronized && guardSeconds > 0U &&
            (second < guardSeconds || second >= 60U - guardSeconds);
    }

    constexpr uint32_t nextAttemptAfterSuccess(uint32_t now, uint32_t pollInterval)
    {
        return now + pollInterval;
    }

    constexpr uint32_t nextAttemptAfterFailure(uint32_t now, uint32_t retryDelay)
    {
        return now + retryDelay;
    }
}
