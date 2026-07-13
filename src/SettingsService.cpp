#include "SettingsService.h"

#include <Preferences.h>

#include "Config.h"
#include "Hardware.h"

namespace
{
    Preferences preferences;
    uint8_t selectedBrightness = Hardware::LED_BRIGHTNESS;
    DisplayMode selectedDisplayMode = DisplayMode::CLASSIC;
    bool storageReady = false;

    bool isSupportedBrightness(uint8_t level)
    {
        for (size_t index = 0; index < Config::LED_BRIGHTNESS_LEVEL_COUNT; ++index)
        {
            if (Config::LED_BRIGHTNESS_LEVELS[index] == level)
            {
                return true;
            }
        }

        return false;
    }
}

void SettingsService::begin()
{
    selectedBrightness = Hardware::LED_BRIGHTNESS;
    selectedDisplayMode = DisplayMode::CLASSIC;
    storageReady = preferences.begin(Config::SETTINGS_NAMESPACE, false);
    bool settingsDefaulted = !storageReady;

    if (storageReady && preferences.isKey(Config::SETTINGS_BRIGHTNESS_KEY))
    {
        const uint8_t savedBrightness =
            preferences.getUChar(Config::SETTINGS_BRIGHTNESS_KEY, Hardware::LED_BRIGHTNESS);

        if (isSupportedBrightness(savedBrightness))
        {
            selectedBrightness = savedBrightness;
        }
        else
        {
            settingsDefaulted = true;
        }
    }
    else
    {
        settingsDefaulted = true;
    }

    if (storageReady && preferences.isKey(Config::SETTINGS_DISPLAY_MODE_KEY))
    {
        const uint8_t savedMode =
            preferences.getUChar(Config::SETTINGS_DISPLAY_MODE_KEY, static_cast<uint8_t>(DisplayMode::CLASSIC));

        if (DisplayModes::isValid(savedMode))
        {
            selectedDisplayMode = static_cast<DisplayMode>(savedMode);
        }
        else
        {
            settingsDefaulted = true;
        }
    }
    else
    {
        settingsDefaulted = true;
    }

    Serial.println(settingsDefaulted ? "SETTINGS DEFAULTED" : "SETTINGS LOADED");
}

uint8_t SettingsService::brightness()
{
    return selectedBrightness;
}

void SettingsService::saveBrightness(uint8_t level)
{
    if (!storageReady || !isSupportedBrightness(level) || level == selectedBrightness)
    {
        return;
    }

    if (preferences.putUChar(Config::SETTINGS_BRIGHTNESS_KEY, level) == sizeof(level))
    {
        selectedBrightness = level;
        Serial.printf("BRIGHTNESS SAVED: %u\r\n", level);
    }
}

DisplayMode SettingsService::displayMode()
{
    return selectedDisplayMode;
}

void SettingsService::saveDisplayMode(DisplayMode mode)
{
    const uint8_t value = static_cast<uint8_t>(mode);
    if (!storageReady || !DisplayModes::isValid(value) || mode == selectedDisplayMode)
    {
        return;
    }

    if (preferences.putUChar(Config::SETTINGS_DISPLAY_MODE_KEY, value) == sizeof(value))
    {
        selectedDisplayMode = mode;
    }
}
