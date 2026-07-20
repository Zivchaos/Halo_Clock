#pragma once

#include <Arduino.h>

#include "DisplayMode.h"
#include "SettingsService.h"

class Halo
{
public:

    static void begin();

    static void update();

    static bool setBrightness(uint8_t brightness);
    static bool setDisplayMode(DisplayMode mode);
    static bool setRingCalibration(const RingCalibrationSettings& settings, bool testActive);
    static bool setCustomColors(const CustomColorSettings& settings);
    static bool isRingCalibrationActive();

};
