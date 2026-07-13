#pragma once

#include <Arduino.h>

class OtaService
{
public:
    static void begin();
    static void update();
    static bool isUpdating();
    static bool isDisplayReserved();
    static uint8_t progressPercent();
};
