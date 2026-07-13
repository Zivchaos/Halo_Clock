#pragma once

#include <Arduino.h>

class LedRing
{
public:
    static void begin();
    static void bootAnimation();
    static void showCalibrationTest();
    static void drawClock(uint8_t hour, uint8_t minute, uint8_t second);
};
