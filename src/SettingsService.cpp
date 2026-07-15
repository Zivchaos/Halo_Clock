#include "SettingsService.h"

#include <Preferences.h>
#include <stddef.h>

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
    NetworkSettings networkSettings;
    bool networkStored = false;
    uint8_t staticFailureCount = 0;

    constexpr uint32_t NETWORK_RECORD_MAGIC = 0x484E4554UL;
    constexpr uint8_t NETWORK_RECORD_VERSION = 1;

    struct __attribute__((packed)) PersistedNetworkSettings
    {
        uint32_t magic;
        uint8_t version;
        uint8_t mode;
        uint8_t secondaryDnsConfigured;
        uint8_t reserved;
        uint8_t staticIp[4];
        uint8_t gateway[4];
        uint8_t subnet[4];
        uint8_t primaryDns[4];
        uint8_t secondaryDns[4];
        uint32_t checksum;
    };

    uint32_t recordChecksum(const PersistedNetworkSettings& record)
    {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&record);
        uint32_t checksum = 2166136261UL;
        for (size_t index = 0; index < offsetof(PersistedNetworkSettings, checksum); ++index)
        {
            checksum ^= bytes[index];
            checksum *= 16777619UL;
        }
        return checksum;
    }

    void copyAddress(uint8_t destination[4], const IPv4Address& source)
    {
        memcpy(destination, source.bytes, 4);
    }

    void copyAddress(IPv4Address& destination, const uint8_t source[4])
    {
        memcpy(destination.bytes, source, 4);
    }

    PersistedNetworkSettings toRecord(const NetworkSettings& settings)
    {
        PersistedNetworkSettings record = {};
        record.magic = NETWORK_RECORD_MAGIC;
        record.version = NETWORK_RECORD_VERSION;
        record.mode = static_cast<uint8_t>(settings.mode);
        record.secondaryDnsConfigured = settings.secondaryDnsConfigured ? 1U : 0U;
        copyAddress(record.staticIp, settings.staticIp);
        copyAddress(record.gateway, settings.gateway);
        copyAddress(record.subnet, settings.subnet);
        copyAddress(record.primaryDns, settings.primaryDns);
        copyAddress(record.secondaryDns, settings.secondaryDns);
        record.checksum = recordChecksum(record);
        return record;
    }

    bool fromRecord(const PersistedNetworkSettings& record, NetworkSettings& settings)
    {
        if (record.magic != NETWORK_RECORD_MAGIC ||
            record.version != NETWORK_RECORD_VERSION ||
            record.secondaryDnsConfigured > 1U ||
            record.checksum != recordChecksum(record))
        {
            return false;
        }

        NetworkSettings loaded;
        loaded.mode = static_cast<NetworkMode>(record.mode);
        loaded.secondaryDnsConfigured = record.secondaryDnsConfigured == 1U;
        copyAddress(loaded.staticIp, record.staticIp);
        copyAddress(loaded.gateway, record.gateway);
        copyAddress(loaded.subnet, record.subnet);
        copyAddress(loaded.primaryDns, record.primaryDns);
        copyAddress(loaded.secondaryDns, record.secondaryDns);
        if (!NetworkConfig::validate(loaded).valid)
        {
            return false;
        }
        settings = loaded;
        return true;
    }

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
    networkSettings = NetworkConfig::defaults();
    networkStored = false;
    staticFailureCount = 0;
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

    if (storageReady &&
        preferences.isKey(Config::SETTINGS_NETWORK_KEY) &&
        preferences.getType(Config::SETTINGS_NETWORK_KEY) == PT_BLOB &&
        preferences.getBytesLength(Config::SETTINGS_NETWORK_KEY) == sizeof(PersistedNetworkSettings))
    {
        PersistedNetworkSettings record = {};
        if (preferences.getBytes(Config::SETTINGS_NETWORK_KEY, &record, sizeof(record)) == sizeof(record) &&
            fromRecord(record, networkSettings))
        {
            networkStored = true;
            Serial.printf("NETWORK SETTINGS LOADED: %s\r\n", NetworkConfig::modeName(networkSettings.mode));
        }
        else
        {
            Serial.println("NETWORK SETTINGS DEFAULTED: DHCP");
        }
    }
    else
    {
        Serial.println("NETWORK SETTINGS DEFAULTED: DHCP");
    }

    if (hasUnsignedByteKey(Config::SETTINGS_STATIC_FAILURE_COUNT_KEY))
    {
        staticFailureCount = preferences.getUChar(Config::SETTINGS_STATIC_FAILURE_COUNT_KEY, 0);
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

const NetworkSettings& SettingsService::network()
{
    return networkSettings;
}

bool SettingsService::saveNetwork(const NetworkSettings& settings)
{
    if (!storageReady || !NetworkConfig::validate(settings).valid)
    {
        return false;
    }

    if (!networkStored || !NetworkConfig::equals(settings, networkSettings))
    {
        const PersistedNetworkSettings record = toRecord(settings);
        if (preferences.putBytes(Config::SETTINGS_NETWORK_KEY, &record, sizeof(record)) != sizeof(record))
        {
            return false;
        }
        networkSettings = settings;
        networkStored = true;
        Serial.printf("NETWORK SETTINGS SAVED: %s\r\n", NetworkConfig::modeName(settings.mode));
    }

    return clearStaticNetworkFailures();
}

bool SettingsService::resetNetworkToDhcp()
{
    const NetworkSettings dhcp = NetworkConfig::defaults();
    return saveNetwork(dhcp);
}

uint8_t SettingsService::staticNetworkFailureCount()
{
    return staticFailureCount;
}

uint8_t SettingsService::recordStaticNetworkFailure()
{
    if (!storageReady || staticFailureCount == UINT8_MAX)
    {
        return staticFailureCount;
    }

    const uint8_t next = static_cast<uint8_t>(staticFailureCount + 1U);
    if (preferences.putUChar(Config::SETTINGS_STATIC_FAILURE_COUNT_KEY, next) == sizeof(next))
    {
        staticFailureCount = next;
    }
    return staticFailureCount;
}

bool SettingsService::clearStaticNetworkFailures()
{
    if (!storageReady || staticFailureCount == 0)
    {
        return storageReady;
    }

    const uint8_t cleared = 0;
    if (preferences.putUChar(Config::SETTINGS_STATIC_FAILURE_COUNT_KEY, cleared) != sizeof(cleared))
    {
        return false;
    }
    staticFailureCount = 0;
    return true;
}
