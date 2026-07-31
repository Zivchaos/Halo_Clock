#pragma once

#include <Arduino.h>

namespace Hardware
{
    constexpr uint8_t LED_PIN = 18;
    constexpr uint8_t LED_COUNT = 60;
    constexpr uint8_t LED_BRIGHTNESS = 40;

#ifndef HALO_OLED_SDA
    constexpr uint8_t OLED_SDA = 21;
#else
    constexpr uint8_t OLED_SDA = HALO_OLED_SDA;
#endif

#ifndef HALO_OLED_SCL
    constexpr uint8_t OLED_SCL = 22;
#else
    constexpr uint8_t OLED_SCL = HALO_OLED_SCL;
#endif
    constexpr uint8_t OLED_ADDRESS = 0x3C;

    // Button connects between GPIO27 and GND. INPUT_PULLUP is used.
    constexpr uint8_t BUTTON_PIN = 27;
}
