#pragma once

#include <time.h>

#include "DisplayMode.h"
#include "WeatherService.h"

class Oled
{
public:
    static void begin();
    static void splash();
    static void status(const char* line1, const char* line2);
    static void time(const tm& localTime, DisplayMode mode);
    static void displayMode(DisplayMode mode);
    static void notice(const char* message, DisplayMode mode);
    static void weather(const WeatherData& weather, DisplayMode mode);
    static void redAlert(const char* area, bool test);
};
