#pragma once

#include <Arduino.h>

enum class DisplayMode : uint8_t
{
    CLASSIC = 0,
    MINIMAL,
    NIGHT,
    CUSTOM
};

namespace DisplayModes
{
    constexpr uint8_t COUNT = 4;
    constexpr const char* NAMES[COUNT] = {"CLASSIC", "MINIMAL", "NIGHT", "CUSTOM"};

    constexpr bool isValid(uint8_t value)
    {
        return value < COUNT;
    }

    constexpr const char* name(DisplayMode mode)
    {
        const uint8_t index = static_cast<uint8_t>(mode);
        return isValid(index) ? NAMES[index] : NAMES[0];
    }

    constexpr DisplayMode next(DisplayMode mode)
    {
        return static_cast<DisplayMode>((static_cast<uint8_t>(mode) + 1) % COUNT);
    }
}
