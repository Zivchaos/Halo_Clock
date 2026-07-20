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
    RingCalibrationSettings calibration = {Config::LED_ZERO_OFFSET, Config::LED_CLOCKWISE};
    CustomColorSettings customColors = {
        {18, 18, 18}, {80, 80, 0}, {255, 80, 0},
        {80, 25, 0}, {80, 80, 0}, {0, 120, 255}};

    static_assert(Config::LED_ZERO_OFFSET < Hardware::LED_COUNT, "LED_ZERO_OFFSET must select an existing LED");

    uint8_t logicalToPhysicalIndex(int16_t logicalIndex)
    {
        int16_t normalized = logicalIndex % Hardware::LED_COUNT;
        if (normalized < 0)
        {
            normalized += Hardware::LED_COUNT;
        }

        if (!calibration.clockwise)
        {
            normalized = (Hardware::LED_COUNT - normalized) % Hardware::LED_COUNT;
        }

        return (calibration.zeroOffset + normalized) % Hardware::LED_COUNT;
    }

    uint32_t color(const RgbColor& value)
    {
        return strip.Color(value.red, value.green, value.blue);
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
    setBrightness(Config::LED_BRIGHTNESS_LEVELS[brightnessIndex]);
    Serial.printf("LED BRIGHTNESS %u\r\n", selectedBrightness);
    return selectedBrightness;
}

bool LedRing::setBrightness(uint8_t brightness)
{
    if (!Config::isSupportedBrightness(brightness))
    {
        return false;
    }

    selectedBrightness = brightness;
    brightnessIndex = brightnessLevelIndex(brightness);
    strip.setBrightness(
        activeDisplayMode == DisplayMode::NIGHT ? Config::NIGHT_LED_BRIGHTNESS : selectedBrightness);
    strip.show();
    return true;
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
    Serial.printf("[CAL] logical  0 (12 o'clock, white) -> physical %u\r\n", logicalToPhysicalIndex(0));
    Serial.printf("[CAL] logical 15 ( 3 o'clock, red)   -> physical %u\r\n", logicalToPhysicalIndex(15));
    Serial.printf("[CAL] logical 30 ( 6 o'clock, green) -> physical %u\r\n", logicalToPhysicalIndex(30));
    Serial.printf("[CAL] logical 45 ( 9 o'clock, blue)  -> physical %u\r\n", logicalToPhysicalIndex(45));
}

void LedRing::setRingCalibration(const RingCalibrationSettings& settings)
{
    if (settings.zeroOffset < Hardware::LED_COUNT)
    {
        calibration = settings;
    }
}

void LedRing::setCustomColors(const CustomColorSettings& settings)
{
    customColors = settings;
}

void LedRing::showOtaProgress(uint8_t percent)
{
    const uint8_t boundedPercent = min(percent, static_cast<uint8_t>(100));
    const uint8_t litPixels = static_cast<uint8_t>(
        (static_cast<uint16_t>(boundedPercent) * Hardware::LED_COUNT + 99) / 100);

    strip.clear();
    strip.setBrightness(Config::OTA_LED_BRIGHTNESS);
    for (uint8_t index = 0; index < litPixels; ++index)
    {
        setLogicalPixel(index, strip.Color(0, 80, 255));
    }
    strip.show();
}

void LedRing::drawClock(uint8_t hour, uint8_t minute, uint8_t second, DisplayMode mode)
{
    strip.clear();

    if (mode == DisplayMode::CLASSIC || mode == DisplayMode::CUSTOM)
    {
        for (uint8_t tick = 0; tick < Hardware::LED_COUNT; tick += 5)
        {
            setLogicalPixel(tick, mode == DisplayMode::CUSTOM ? color(customColors.hourTicks) : strip.Color(18, 18, 18));
        }

        for (uint8_t index = 0; index < minute; ++index)
        {
            setLogicalPixel(index, mode == DisplayMode::CUSTOM ? color(customColors.minuteProgress) : strip.Color(80, 80, 0));
        }
        if (mode == DisplayMode::CUSTOM)
        {
            setLogicalPixel(minute, color(customColors.minuteMarker));
        }
    }
    else
    {
        setLogicalPixel(minute, strip.Color(80, 80, 0));
    }

    const uint8_t hourPosition = ((hour % 12) * 5 + minute / 12) % Hardware::LED_COUNT;
    const uint32_t hourSideColor = mode == DisplayMode::CUSTOM ? color(customColors.hourSides) : strip.Color(80, 25, 0);
    const uint32_t hourCenterColor = mode == DisplayMode::CUSTOM ? color(customColors.hourCenter) : strip.Color(255, 80, 0);
    setLogicalPixel(hourPosition - 1, hourSideColor);
    setLogicalPixel(hourPosition, hourCenterColor);
    setLogicalPixel(hourPosition + 1, hourSideColor);

    if (mode != DisplayMode::NIGHT)
    {
        setLogicalPixel(second, mode == DisplayMode::CUSTOM ? color(customColors.secondMarker) : strip.Color(0, 120, 255));
    }
    strip.show();
}
