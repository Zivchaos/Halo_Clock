#pragma once

#include <Arduino.h>

#include "DisplayMode.h"
#include "NetworkConfig.h"

struct AutoNightSettings
{
    bool enabled;
    uint8_t startHour;
    uint8_t startMinute;
    uint8_t endHour;
    uint8_t endMinute;
};

struct RingCalibrationSettings
{
    uint8_t zeroOffset;
    bool clockwise;
};

struct WeatherLocationSettings
{
    float latitude;
    float longitude;
};

struct RgbColor
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct CustomColorSettings
{
    RgbColor hourTicks;
    RgbColor minuteProgress;
    RgbColor hourCenter;
    RgbColor hourSides;
    RgbColor minuteMarker;
    RgbColor secondMarker;
};

struct RedAlertSettings
{
    bool enabled;
    char locations[512];
    char relayUrl[256];
};

struct HardwareSettings
{
    uint8_t ledData;
    uint8_t oledSda;
    uint8_t oledScl;
    uint8_t button;
};

class SettingsService
{
public:
    static void begin();
    static uint8_t brightness();
    static void saveBrightness(uint8_t level);
    static DisplayMode displayMode();
    static void saveDisplayMode(DisplayMode mode);
    static const AutoNightSettings& autoNight();
    static bool saveAutoNight(const AutoNightSettings& settings);
    static const NetworkSettings& network();
    static bool saveNetwork(const NetworkSettings& settings);
    static bool resetNetworkToDhcp();
    static uint8_t staticNetworkFailureCount();
    static uint8_t recordStaticNetworkFailure();
    static bool clearStaticNetworkFailures();
    static const RingCalibrationSettings& ringCalibration();
    static bool saveRingCalibration(const RingCalibrationSettings& settings);
    static const WeatherLocationSettings& weatherLocation();
    static bool saveWeatherLocation(const WeatherLocationSettings& settings);
    static const CustomColorSettings& customColors();
    static bool saveCustomColors(const CustomColorSettings& settings);
    static const RedAlertSettings& redAlert();
    static bool saveRedAlert(const RedAlertSettings& settings);
    static const HardwareSettings& hardware();
    static bool saveHardware(const HardwareSettings& settings);
    static bool isHardwarePinAllowed(uint8_t pin);
    static bool isHardwareSettingsValid(const HardwareSettings& settings);
};
