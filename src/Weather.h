#pragma once

#include <Arduino.h>

struct WeatherData
{
    bool valid = false;
    float temperature = 0.0F;
    float apparentTemperature = 0.0F;
    float humidity = 0.0F;
    float windSpeed = 0.0F;
    float todayHigh = 0.0F;
    float todayLow = 0.0F;
    int weatherCode = 0;
    bool isDay = true;
    uint32_t updatedAt = 0;
};

class Weather
{
public:
    static void begin();
    static void update();
    static void refresh();
    static const WeatherData& data();
    static bool isUpdating();
    static const char* description(int weatherCode);
};
