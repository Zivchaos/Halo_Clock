#pragma once

#include <Arduino.h>

#include "DisplayMode.h"

class Halo
{
public:

    static void begin();

    static void update();

    static bool setBrightness(uint8_t brightness);
    static bool setDisplayMode(DisplayMode mode);

};
