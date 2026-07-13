#include "Ledring.h"

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "Hardware.h"

namespace
{
    Adafruit_NeoPixel strip(Hardware::LED_COUNT, Hardware::LED_PIN, NEO_GRB + NEO_KHZ800);
}

void LedRing::begin()
{
    strip.begin();
    strip.setBrightness(Hardware::LED_BRIGHTNESS);
    strip.clear();
    strip.show();
}

void LedRing::bootAnimation()
{
    strip.clear();
    strip.setBrightness(Hardware::LED_BRIGHTNESS);
    for (uint8_t index = 0; index < Hardware::LED_COUNT; ++index)
    {
        strip.setPixelColor(index, 0, 0, 180);
        strip.show();
        delay(12);
    }
    for (int16_t brightness = Hardware::LED_BRIGHTNESS; brightness >= 0; --brightness)
    {
        strip.setBrightness(brightness);
        strip.show();
        delay(8);
    }
    strip.clear();
    strip.setBrightness(Hardware::LED_BRIGHTNESS);
    strip.show();
}

void LedRing::drawClock(uint8_t hour, uint8_t minute, uint8_t second)
{
    strip.clear();

    for (uint8_t tick = 0; tick < Hardware::LED_COUNT; tick += 5)
    {
        strip.setPixelColor(tick, 18, 18, 18);
    }

    for (uint8_t index = 0; index < minute; ++index)
    {
        strip.setPixelColor(index, 80, 80, 0);
    }

    const uint8_t hourPosition = ((hour % 12) * 5 + minute / 12) % Hardware::LED_COUNT;
    strip.setPixelColor((hourPosition + Hardware::LED_COUNT - 1) % Hardware::LED_COUNT, 80, 25, 0);
    strip.setPixelColor(hourPosition, 255, 80, 0);
    strip.setPixelColor((hourPosition + 1) % Hardware::LED_COUNT, 80, 25, 0);

    strip.setPixelColor(second, 0, 120, 255);
    strip.show();
}
