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
};
