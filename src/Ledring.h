#pragma once

#include <Arduino.h>

#include "DisplayMode.h"
#include "SettingsService.h"

class LedRing
{
public:
    static void begin(uint8_t brightness);
    static void setDisplayMode(DisplayMode mode);
    static void bootAnimation();
    static void showCalibrationTest();
    static void showOtaProgress(uint8_t percent);
    static void showRedAlert(bool phase);
    static void drawClock(uint8_t hour, uint8_t minute, uint8_t second, DisplayMode mode);
    static bool setBrightness(uint8_t brightness);
    static uint8_t cycleBrightness();
    static void setRingCalibration(const RingCalibrationSettings& settings);
    static void setCustomColors(const CustomColorSettings& settings);
};
