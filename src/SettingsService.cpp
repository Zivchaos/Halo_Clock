#include "SettingsService.h"

#include <Preferences.h>

#include "Config.h"
#include "Hardware.h"

namespace
{
    Preferences preferences;
    uint8_t selectedBrightness = Hardware::LED_BRIGHTNESS;
    DisplayMode selectedDisplayMode = DisplayMode::CLASSIC;
    AutoNightSettings automaticNight = {
        Config::AUTO_NIGHT_ENABLED,
        Config::AUTO_NIGHT_START_HOUR,
        Config::AUTO_NIGHT_START_MINUTE,
        Config::AUTO_NIGHT_END_HOUR,
        Config::AUTO_NIGHT_END_MINUTE};
    bool storageReady = false;
    bool autoNightStored = false;

    bool isValidAutoNight(const AutoNightSettings& settings)
    {
        return settings.startHour < 24 && settings.endHour < 24 &&
            settings.startMinute < 60 && settings.endMinute < 60;
    }

    bool hasUnsignedByteKey(const char* key)
    {
        return storageReady && preferences.isKey(key) && preferences.getType(key) == PT_U8;
    }
}

void SettingsService::begin()
{
    selectedBrightness = Hardware::LED_BRIGHTNESS;
    selectedDisplayMode = DisplayMode::CLASSIC;
    automaticNight = {
        Config::AUTO_NIGHT_ENABLED,
        Config::AUTO_NIGHT_START_HOUR,
        Config::AUTO_NIGHT_START_MINUTE,
        Config::AUTO_NIGHT_END_HOUR,
        Config::AUTO_NIGHT_END_MINUTE};
    storageReady = preferences.begin(Config::SETTINGS_NAMESPACE, false);
    autoNightStored = false;
    bool settingsDefaulted = !storageReady;

    if (hasUnsignedByteKey(Config::SETTINGS_BRIGHTNESS_KEY))
    {
        const uint8_t savedBrightness =
            preferences.getUChar(Config::SETTINGS_BRIGHTNESS_KEY, Hardware::LED_BRIGHTNESS);

        if (Config::isSupportedBrightness(savedBrightness))
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

    const bool hasAllAutoNightKeys =
        hasUnsignedByteKey(Config::SETTINGS_AUTO_NIGHT_ENABLED_KEY) &&
        hasUnsignedByteKey(Config::SETTINGS_AUTO_NIGHT_START_HOUR_KEY) &&
        hasUnsignedByteKey(Config::SETTINGS_AUTO_NIGHT_START_MINUTE_KEY) &&
        hasUnsignedByteKey(Config::SETTINGS_AUTO_NIGHT_END_HOUR_KEY) &&
        hasUnsignedByteKey(Config::SETTINGS_AUTO_NIGHT_END_MINUTE_KEY);

    if (hasAllAutoNightKeys)
    {
        const uint8_t savedEnabled = preferences.getUChar(
            Config::SETTINGS_AUTO_NIGHT_ENABLED_KEY,
            Config::AUTO_NIGHT_ENABLED ? 1 : 0);
        const AutoNightSettings savedAutoNight = {
            savedEnabled == 1,
            preferences.getUChar(Config::SETTINGS_AUTO_NIGHT_START_HOUR_KEY, Config::AUTO_NIGHT_START_HOUR),
            preferences.getUChar(Config::SETTINGS_AUTO_NIGHT_START_MINUTE_KEY, Config::AUTO_NIGHT_START_MINUTE),
            preferences.getUChar(Config::SETTINGS_AUTO_NIGHT_END_HOUR_KEY, Config::AUTO_NIGHT_END_HOUR),
            preferences.getUChar(Config::SETTINGS_AUTO_NIGHT_END_MINUTE_KEY, Config::AUTO_NIGHT_END_MINUTE)};

        if (savedEnabled <= 1 && isValidAutoNight(savedAutoNight))
        {
            automaticNight = savedAutoNight;
            autoNightStored = true;
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

    if (hasUnsignedByteKey(Config::SETTINGS_DISPLAY_MODE_KEY))
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
    if (!storageReady || !Config::isSupportedBrightness(level) || level == selectedBrightness)
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

const AutoNightSettings& SettingsService::autoNight()
{
    return automaticNight;
}

bool SettingsService::saveAutoNight(const AutoNightSettings& settings)
{
    if (!storageReady || !isValidAutoNight(settings))
    {
        return false;
    }

    bool saved = true;
    if (!autoNightStored || settings.enabled != automaticNight.enabled)
    {
        saved = preferences.putBool(Config::SETTINGS_AUTO_NIGHT_ENABLED_KEY, settings.enabled) == sizeof(uint8_t) && saved;
    }
    if (!autoNightStored || settings.startHour != automaticNight.startHour)
    {
        saved = preferences.putUChar(Config::SETTINGS_AUTO_NIGHT_START_HOUR_KEY, settings.startHour) == sizeof(settings.startHour) && saved;
    }
    if (!autoNightStored || settings.startMinute != automaticNight.startMinute)
    {
        saved = preferences.putUChar(Config::SETTINGS_AUTO_NIGHT_START_MINUTE_KEY, settings.startMinute) == sizeof(settings.startMinute) && saved;
    }
    if (!autoNightStored || settings.endHour != automaticNight.endHour)
    {
        saved = preferences.putUChar(Config::SETTINGS_AUTO_NIGHT_END_HOUR_KEY, settings.endHour) == sizeof(settings.endHour) && saved;
    }
    if (!autoNightStored || settings.endMinute != automaticNight.endMinute)
    {
        saved = preferences.putUChar(Config::SETTINGS_AUTO_NIGHT_END_MINUTE_KEY, settings.endMinute) == sizeof(settings.endMinute) && saved;
    }

    if (saved)
    {
        automaticNight = settings;
        autoNightStored = true;
    }
    return saved;
}
