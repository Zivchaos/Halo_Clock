#pragma once

#include <time.h>

#include "DisplayMode.h"

namespace AutoNightSchedule
{
    constexpr bool isValidTime(int hour, int minute)
    {
        return hour >= 0 && hour < 24 && minute >= 0 && minute < 60;
    }

    constexpr bool isActive(
        int currentHour,
        int currentMinute,
        int startHour,
        int startMinute,
        int endHour,
        int endMinute)
    {
        if (!isValidTime(currentHour, currentMinute) ||
            !isValidTime(startHour, startMinute) ||
            !isValidTime(endHour, endMinute))
        {
            return false;
        }

        const int current = currentHour * 60 + currentMinute;
        const int start = startHour * 60 + startMinute;
        const int end = endHour * 60 + endMinute;

        // Equal boundaries explicitly represent a disabled/empty schedule.
        if (start == end)
        {
            return false;
        }

        if (start < end)
        {
            return current >= start && current < end;
        }

        return current >= start || current < end;
    }
}

enum class AutoNightEvent
{
    NONE,
    ACTIVATED,
    DEACTIVATED,
    TIME_INVALID
};

class AutoNightService
{
public:
    static void begin();
    static AutoNightEvent update(bool timeValid, const tm& localTime);
    static bool activateManualOverride();
    static DisplayMode effectiveMode(DisplayMode selectedMode);
};
