#pragma once

#include <Arduino.h>

namespace Config
{
    constexpr uint32_t SERIAL_BAUD = 115200;

    constexpr bool SERIAL_LOGGING = true;
    constexpr bool SHOW_SPLASH = true;
    constexpr bool ENABLE_WIFI = true;
    constexpr bool ENABLE_OLED = true;
    constexpr bool ENABLE_LED_RING = true;
}