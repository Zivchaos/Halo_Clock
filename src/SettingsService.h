#pragma once

#include <Arduino.h>

#include "DisplayMode.h"

class SettingsService
{
public:
    static void begin();
    static uint8_t brightness();
    static void saveBrightness(uint8_t level);
    static DisplayMode displayMode();
    static void saveDisplayMode(DisplayMode mode);
};
