#include "LedRing.h"

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "Hardware.h"

static Adafruit_NeoPixel strip(
    Hardware::LED_COUNT,
    Hardware::LED_PIN,
    NEO_GRB + NEO_KHZ800);

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

    // Blue sweep
    for (int i = 0; i < Hardware::LED_COUNT; i++)
    {
        strip.setPixelColor(i, 0, 0, 255);
        strip.show();
        delay(15);
    }

    delay(150);

    // Fade out
    for (int b = Hardware::LED_BRIGHTNESS; b >= 0; b--)
    {
        strip.setBrightness(b);
        strip.show();
        delay(12);
    }

    strip.clear();
    for (int i = 0; i < 60; i += 5)
    {
    strip.setPixelColor(i, 30, 30, 30);
    }
strip.show();
}

void LedRing::drawClock(
    uint8_t hour,
    uint8_t minute,
    uint8_t second)
{
    strip.clear();

    //
    // Hour ticks
    //
    for (int i = 0; i < 60; i += 5)
    {
        strip.setPixelColor(i, 25, 25, 25);
    }

    //
    // Minutes
    //
    for (int i = 0; i < minute; i++)
    {
        strip.setPixelColor(i, 80, 80, 0);
    }

    //
    // Hours
    //
    int h = (hour % 12) * 5;

    strip.setPixelColor((h + 59) % 60, 120, 40, 0);
    strip.setPixelColor(h,               255, 80, 0);
    strip.setPixelColor((h + 1) % 60,    120, 40, 0);

    //
    // Seconds
    //
    strip.setPixelColor(second, 0, 120, 255);

    strip.show();
}
