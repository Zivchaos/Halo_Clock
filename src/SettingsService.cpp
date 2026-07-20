#include "SettingsService.h"

#include <Preferences.h>
#include <cmath>
#include <cstring>
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
    RingCalibrationSettings ringCalibrationSettings = {
        Config::LED_ZERO_OFFSET,
        Config::LED_CLOCKWISE};
    WeatherLocationSettings weatherLocationSettings = {
        Config::WEATHER_LATITUDE,
        Config::WEATHER_LONGITUDE};
    CustomColorSettings customColorSettings = {
        {18, 18, 18},
        {80, 80, 0},
        {255, 80, 0},
        {80, 25, 0},
        {80, 80, 0},
        {0, 120, 255}};

    constexpr uint32_t NETWORK_RECORD_MAGIC = 0x484E4554UL;
    constexpr uint8_t NETWORK_RECORD_VERSION = 1;
    constexpr uint32_t WEATHER_LOCATION_MAGIC = 0x48574C43UL;
    constexpr uint8_t WEATHER_LOCATION_VERSION = 1;
    constexpr uint32_t CUSTOM_COLORS_MAGIC = 0x48434C52UL;
    constexpr uint8_t CUSTOM_COLORS_VERSION = 1;

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

    struct PersistedWeatherLocation
    {
        uint32_t magic;
        uint8_t version;
        float latitude;
        float longitude;
        uint32_t checksum;
    };

    struct PersistedCustomColors
    {
        uint32_t magic;
        uint8_t version;
        CustomColorSettings colors;
        uint32_t checksum;
    };

    uint32_t bytesChecksum(const void* data, size_t length)
    {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
        uint32_t checksum = 2166136261UL;
        for (size_t index = 0; index < length; ++index)
        {
            checksum ^= bytes[index];
            checksum *= 16777619UL;
        }
        return checksum;
    }

    uint32_t recordChecksum(const PersistedNetworkSettings& record)
    {
        return bytesChecksum(&record, offsetof(PersistedNetworkSettings, checksum));
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

    bool validWeatherLocation(const WeatherLocationSettings& settings)
    {
        return std::isfinite(settings.latitude) && std::isfinite(settings.longitude) &&
            settings.latitude >= -90.0F && settings.latitude <= 90.0F &&
            settings.longitude >= -180.0F && settings.longitude <= 180.0F;
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
    ringCalibrationSettings = {Config::LED_ZERO_OFFSET, Config::LED_CLOCKWISE};
    weatherLocationSettings = {Config::WEATHER_LATITUDE, Config::WEATHER_LONGITUDE};
    customColorSettings = {
        {18, 18, 18}, {80, 80, 0}, {255, 80, 0},
        {80, 25, 0}, {80, 80, 0}, {0, 120, 255}};
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

    if (hasUnsignedByteKey(Config::SETTINGS_RING_ZERO_KEY) &&
        hasUnsignedByteKey(Config::SETTINGS_RING_CLOCKWISE_KEY))
    {
        const uint8_t zero = preferences.getUChar(Config::SETTINGS_RING_ZERO_KEY, Config::LED_ZERO_OFFSET);
        const uint8_t clockwise = preferences.getUChar(
            Config::SETTINGS_RING_CLOCKWISE_KEY,
            Config::LED_CLOCKWISE ? 1U : 0U);
        if (zero < Hardware::LED_COUNT && clockwise <= 1U)
        {
            ringCalibrationSettings = {zero, clockwise == 1U};
        }
    }

    if (storageReady && preferences.isKey(Config::SETTINGS_WEATHER_LOCATION_KEY) &&
        preferences.getType(Config::SETTINGS_WEATHER_LOCATION_KEY) == PT_BLOB &&
        preferences.getBytesLength(Config::SETTINGS_WEATHER_LOCATION_KEY) == sizeof(PersistedWeatherLocation))
    {
        PersistedWeatherLocation record = {};
        if (preferences.getBytes(Config::SETTINGS_WEATHER_LOCATION_KEY, &record, sizeof(record)) == sizeof(record) &&
            record.magic == WEATHER_LOCATION_MAGIC && record.version == WEATHER_LOCATION_VERSION &&
            record.checksum == bytesChecksum(&record, offsetof(PersistedWeatherLocation, checksum)))
        {
            const WeatherLocationSettings loaded = {record.latitude, record.longitude};
            if (validWeatherLocation(loaded)) weatherLocationSettings = loaded;
        }
    }

    if (storageReady && preferences.isKey(Config::SETTINGS_CUSTOM_COLORS_KEY) &&
        preferences.getType(Config::SETTINGS_CUSTOM_COLORS_KEY) == PT_BLOB &&
        preferences.getBytesLength(Config::SETTINGS_CUSTOM_COLORS_KEY) == sizeof(PersistedCustomColors))
    {
        PersistedCustomColors record = {};
        if (preferences.getBytes(Config::SETTINGS_CUSTOM_COLORS_KEY, &record, sizeof(record)) == sizeof(record) &&
            record.magic == CUSTOM_COLORS_MAGIC && record.version == CUSTOM_COLORS_VERSION &&
            record.checksum == bytesChecksum(&record, offsetof(PersistedCustomColors, checksum)))
        {
            customColorSettings = record.colors;
        }
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

const RingCalibrationSettings& SettingsService::ringCalibration()
{
    return ringCalibrationSettings;
}

bool SettingsService::saveRingCalibration(const RingCalibrationSettings& settings)
{
    if (!storageReady || settings.zeroOffset >= Hardware::LED_COUNT)
    {
        return false;
    }
    if (settings.zeroOffset == ringCalibrationSettings.zeroOffset &&
        settings.clockwise == ringCalibrationSettings.clockwise)
    {
        return true;
    }

    bool saved = true;
    if (settings.zeroOffset != ringCalibrationSettings.zeroOffset)
    {
        saved = preferences.putUChar(Config::SETTINGS_RING_ZERO_KEY, settings.zeroOffset) == sizeof(uint8_t) && saved;
    }
    if (settings.clockwise != ringCalibrationSettings.clockwise)
    {
        saved = preferences.putBool(Config::SETTINGS_RING_CLOCKWISE_KEY, settings.clockwise) == sizeof(uint8_t) && saved;
    }
    if (saved) ringCalibrationSettings = settings;
    return saved;
}

const WeatherLocationSettings& SettingsService::weatherLocation()
{
    return weatherLocationSettings;
}

bool SettingsService::saveWeatherLocation(const WeatherLocationSettings& settings)
{
    if (!storageReady || !validWeatherLocation(settings)) return false;
    if (settings.latitude == weatherLocationSettings.latitude &&
        settings.longitude == weatherLocationSettings.longitude) return true;

    PersistedWeatherLocation record = {};
    record.magic = WEATHER_LOCATION_MAGIC;
    record.version = WEATHER_LOCATION_VERSION;
    record.latitude = settings.latitude;
    record.longitude = settings.longitude;
    record.checksum = bytesChecksum(&record, offsetof(PersistedWeatherLocation, checksum));
    if (preferences.putBytes(Config::SETTINGS_WEATHER_LOCATION_KEY, &record, sizeof(record)) != sizeof(record))
    {
        return false;
    }
    weatherLocationSettings = settings;
    Serial.printf("WEATHER LOCATION SAVED: %.4f, %.4f\r\n", static_cast<double>(settings.latitude), static_cast<double>(settings.longitude));
    return true;
}

const CustomColorSettings& SettingsService::customColors()
{
    return customColorSettings;
}

bool SettingsService::saveCustomColors(const CustomColorSettings& settings)
{
    if (!storageReady) return false;
    if (memcmp(&settings, &customColorSettings, sizeof(settings)) == 0) return true;

    PersistedCustomColors record = {};
    record.magic = CUSTOM_COLORS_MAGIC;
    record.version = CUSTOM_COLORS_VERSION;
    record.colors = settings;
    record.checksum = bytesChecksum(&record, offsetof(PersistedCustomColors, checksum));
    if (preferences.putBytes(Config::SETTINGS_CUSTOM_COLORS_KEY, &record, sizeof(record)) != sizeof(record))
    {
        return false;
    }
    customColorSettings = settings;
    Serial.println("CUSTOM LED COLORS SAVED");
    return true;
}
