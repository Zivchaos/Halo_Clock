#pragma once

#include <Arduino.h>

#include "AppTypes.h"

struct HaloSettings
{
    uint8_t dayBrightness = 40;
    uint8_t nightBrightness = 6;
    uint8_t nightStartHour = 22;
    uint8_t dayStartHour = 7;
    uint8_t hourWidth = 3;
    uint8_t ledOffset = 0;
    bool clockwise = true;
    bool showTicks = true;
    bool showSeconds = true;
    bool weatherEnabled = true;
    DisplayMode displayMode = DisplayMode::CLASSIC;
    ThemeId theme = ThemeId::Classic;
    String timezone = "IST-2IDT,M3.4.4/26,M10.5.0";
    float latitude = 32.0853F;
    float longitude = 34.7818F;
};

class Settings
{
public:
    static void begin();
    static const HaloSettings& get();
    static HaloSettings& edit();
    static void save();
    static void reset();
};
