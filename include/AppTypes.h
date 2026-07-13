#pragma once

#include <Arduino.h>

enum class DisplayMode : uint8_t
{
    Progress = 0,
    Analog,
    Minimal,
    Ambient,
    Rainbow,
    Count
};

enum class ThemeId : uint8_t
{
    Classic = 0,
    Nordic,
    Fire,
    Matrix,
    Cyberpunk,
    Sunset,
    Count
};

enum class OledPage : uint8_t
{
    Clock = 0,
    Weather,
    Network,
    System,
    Count
};

enum class NotificationType : uint8_t
{
    Info = 0,
    Success,
    Warning,
    Error
};

struct RgbColor
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

inline const char* displayModeName(DisplayMode mode)
{
    switch (mode)
    {
        case DisplayMode::Progress: return "Progress";
        case DisplayMode::Analog: return "Analog";
        case DisplayMode::Minimal: return "Minimal";
        case DisplayMode::Ambient: return "Ambient";
        case DisplayMode::Rainbow: return "Rainbow";
        default: return "Unknown";
    }
}

inline const char* themeName(ThemeId theme)
{
    switch (theme)
    {
        case ThemeId::Classic: return "Classic";
        case ThemeId::Nordic: return "Nordic";
        case ThemeId::Fire: return "Fire";
        case ThemeId::Matrix: return "Matrix";
        case ThemeId::Cyberpunk: return "Cyberpunk";
        case ThemeId::Sunset: return "Sunset";
        default: return "Unknown";
    }
}
