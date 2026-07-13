#pragma once

#include <Arduino.h>

#include "DisplayMode.h"

class LedRing
{
public:
    static void begin(uint8_t brightness);
    static void setDisplayMode(DisplayMode mode);
    static void bootAnimation();
    static void showCalibrationTest();
    static void showOtaProgress(uint8_t percent);
    static void drawClock(uint8_t hour, uint8_t minute, uint8_t second, DisplayMode mode);
    static uint8_t cycleBrightness();
};
