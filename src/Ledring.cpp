#include "Ledring.h"

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "Config.h"
#include "Hardware.h"

namespace
{
    Adafruit_NeoPixel strip(Hardware::LED_COUNT, Hardware::LED_PIN, NEO_GRB + NEO_KHZ800);
    size_t brightnessIndex = 0;
    uint8_t selectedBrightness = Hardware::LED_BRIGHTNESS;
    DisplayMode activeDisplayMode = DisplayMode::CLASSIC;

    static_assert(Config::LED_ZERO_OFFSET < Hardware::LED_COUNT, "LED_ZERO_OFFSET must select an existing LED");

    uint8_t logicalToPhysicalIndex(int16_t logicalIndex)
    {
        int16_t normalized = logicalIndex % Hardware::LED_COUNT;
        if (normalized < 0)
        {
            normalized += Hardware::LED_COUNT;
        }

        if (!Config::LED_CLOCKWISE)
        {
            normalized = (Hardware::LED_COUNT - normalized) % Hardware::LED_COUNT;
        }

        return (Config::LED_ZERO_OFFSET + normalized) % Hardware::LED_COUNT;
    }

    void setLogicalPixel(int16_t logicalIndex, uint32_t color)
    {
        strip.setPixelColor(logicalToPhysicalIndex(logicalIndex), color);
    }

    size_t brightnessLevelIndex(uint8_t brightness)
    {
        for (size_t index = 0; index < Config::LED_BRIGHTNESS_LEVEL_COUNT; ++index)
        {
            if (Config::LED_BRIGHTNESS_LEVELS[index] == brightness)
            {
                return index;
            }
        }

        return 0;
    }
}

void LedRing::setDisplayMode(DisplayMode mode)
{
    activeDisplayMode = mode;
    strip.setBrightness(
        mode == DisplayMode::NIGHT ? Config::NIGHT_LED_BRIGHTNESS : selectedBrightness);
    strip.show();
}

void LedRing::begin(uint8_t brightness)
{
    selectedBrightness = brightness;
    brightnessIndex = brightnessLevelIndex(brightness);
    strip.begin();
    strip.setBrightness(
        activeDisplayMode == DisplayMode::NIGHT ? Config::NIGHT_LED_BRIGHTNESS : selectedBrightness);
    strip.clear();
    strip.show();
}

uint8_t LedRing::cycleBrightness()
{
    brightnessIndex = (brightnessIndex + 1) % Config::LED_BRIGHTNESS_LEVEL_COUNT;
    selectedBrightness = Config::LED_BRIGHTNESS_LEVELS[brightnessIndex];
    strip.setBrightness(
        activeDisplayMode == DisplayMode::NIGHT ? Config::NIGHT_LED_BRIGHTNESS : selectedBrightness);
    strip.show();
    Serial.printf("LED BRIGHTNESS %u\n", selectedBrightness);
    return selectedBrightness;
}

void LedRing::bootAnimation()
{
    strip.clear();
    strip.setBrightness(selectedBrightness);
    for (uint8_t index = 0; index < Hardware::LED_COUNT; ++index)
    {
        strip.setPixelColor(index, 0, 0, 180);
        strip.show();
        delay(12);
    }
    for (int16_t brightness = selectedBrightness; brightness >= 0; --brightness)
    {
        strip.setBrightness(brightness);
        strip.show();
        delay(8);
    }
    strip.clear();
    strip.setBrightness(selectedBrightness);
    strip.show();
}

void LedRing::showCalibrationTest()
{
    strip.clear();
    strip.setBrightness(selectedBrightness);

    setLogicalPixel(0, strip.Color(255, 255, 255));
    setLogicalPixel(15, strip.Color(255, 0, 0));
    setLogicalPixel(30, strip.Color(0, 255, 0));
    setLogicalPixel(45, strip.Color(0, 0, 255));
    strip.show();

    Serial.println("[CAL] LED ring calibration test enabled");
    Serial.printf("[CAL] logical  0 (12 o'clock, white) -> physical %u\n", logicalToPhysicalIndex(0));
    Serial.printf("[CAL] logical 15 ( 3 o'clock, red)   -> physical %u\n", logicalToPhysicalIndex(15));
    Serial.printf("[CAL] logical 30 ( 6 o'clock, green) -> physical %u\n", logicalToPhysicalIndex(30));
    Serial.printf("[CAL] logical 45 ( 9 o'clock, blue)  -> physical %u\n", logicalToPhysicalIndex(45));
}

void LedRing::drawClock(uint8_t hour, uint8_t minute, uint8_t second, DisplayMode mode)
{
    strip.clear();

    if (mode == DisplayMode::CLASSIC)
    {
        for (uint8_t tick = 0; tick < Hardware::LED_COUNT; tick += 5)
        {
            setLogicalPixel(tick, strip.Color(18, 18, 18));
        }

        for (uint8_t index = 0; index < minute; ++index)
        {
            setLogicalPixel(index, strip.Color(80, 80, 0));
        }
    }
    else
    {
        setLogicalPixel(minute, strip.Color(80, 80, 0));
    }

    const uint8_t hourPosition = ((hour % 12) * 5 + minute / 12) % Hardware::LED_COUNT;
    setLogicalPixel(hourPosition - 1, strip.Color(80, 25, 0));
    setLogicalPixel(hourPosition, strip.Color(255, 80, 0));
    setLogicalPixel(hourPosition + 1, strip.Color(80, 25, 0));

    if (mode != DisplayMode::NIGHT)
    {
        setLogicalPixel(second, strip.Color(0, 120, 255));
    }
    strip.show();
}
