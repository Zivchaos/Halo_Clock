#pragma once

#include <stddef.h>
#include <stdint.h>

struct WeatherReading
{
    float temperature = 0.0F;
    float apparentTemperature = 0.0F;
    bool apparentTemperatureAvailable = false;
    uint8_t humidity = 0;
    float windSpeed = 0.0F;
    int16_t conditionCode = -1;
    char condition[24] = "Unknown";
};

enum class WeatherParseResult : uint8_t
{
    OK,
    MALFORMED_RESPONSE,
    MISSING_FIELD,
    INVALID_FIELD
};

class WeatherProvider
{
public:
    static bool buildRequestUrl(char* destination, size_t size);
    static WeatherParseResult parseResponse(
        const char* payload,
        size_t length,
        WeatherReading& reading);
    static const char* conditionForCode(int16_t weatherCode);
    static const char* parseError(WeatherParseResult result);
};
