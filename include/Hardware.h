#pragma once

#include <Arduino.h>

namespace Hardware
{
    constexpr uint8_t LED_PIN = 18;      
    constexpr uint8_t LED_COUNT = 60;
    constexpr uint8_t LED_BRIGHTNESS = 40;

    constexpr uint8_t OLED_SDA = 21;
    constexpr uint8_t OLED_SCL = 22;
    constexpr uint8_t OLED_ADDRESS = 0x3C;

    constexpr uint8_t BUTTON_PIN = 0;
}