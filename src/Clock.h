#pragma once

#include "DisplayMode.h"
#include "WeatherService.h"

class Clock
{
public:

    static void begin(DisplayMode mode);

    static void update(bool renderEnabled = true);

    static void setSelectedDisplayMode(DisplayMode mode);
    static void showWeather(const WeatherData& weather);

    static DisplayMode selectedMode();
    static DisplayMode effectiveMode();

};
