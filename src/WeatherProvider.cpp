#include "WeatherProvider.h"

#include <ArduinoJson.h>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "Config.h"

namespace
{
    bool isFiniteNumber(JsonVariantConst value)
    {
        return !value.isNull() && value.is<float>() && std::isfinite(value.as<float>());
    }
}

bool WeatherProvider::buildRequestUrl(
    char* destination,
    size_t size,
    float latitude,
    float longitude)
{
    if (destination == nullptr || size == 0 || !std::isfinite(latitude) || !std::isfinite(longitude) ||
        latitude < -90.0F || latitude > 90.0F || longitude < -180.0F || longitude > 180.0F)
    {
        return false;
    }

    const int written = std::snprintf(
        destination,
        size,
        "%s?latitude=%.4f&longitude=%.4f&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,wind_speed_10m&temperature_unit=%s&wind_speed_unit=%s",
        Config::WEATHER_API_URL,
        static_cast<double>(latitude),
        static_cast<double>(longitude),
        Config::WEATHER_TEMPERATURE_UNIT,
        Config::WEATHER_WIND_SPEED_UNIT);
    return written > 0 && static_cast<size_t>(written) < size;
}

WeatherParseResult WeatherProvider::parseResponse(
    const char* payload,
    size_t length,
    WeatherReading& reading)
{
    if (payload == nullptr || length == 0)
    {
        return WeatherParseResult::MALFORMED_RESPONSE;
    }

    JsonDocument document;
    const DeserializationError error = deserializeJson(document, payload, length);
    if (error)
    {
        return WeatherParseResult::MALFORMED_RESPONSE;
    }

    const JsonObjectConst current = document["current"].as<JsonObjectConst>();
    if (current.isNull() ||
        !isFiniteNumber(current["temperature_2m"]) ||
        !isFiniteNumber(current["relative_humidity_2m"]) ||
        !isFiniteNumber(current["weather_code"]) ||
        !isFiniteNumber(current["wind_speed_10m"]))
    {
        return WeatherParseResult::MISSING_FIELD;
    }

    const float temperature = current["temperature_2m"].as<float>();
    const float humidity = current["relative_humidity_2m"].as<float>();
    const float windSpeed = current["wind_speed_10m"].as<float>();
    const int16_t conditionCode = current["weather_code"].as<int16_t>();
    if (temperature < -100.0F || temperature > 100.0F ||
        humidity < 0.0F || humidity > 100.0F ||
        windSpeed < 0.0F || windSpeed > 500.0F ||
        conditionCode < 0 || conditionCode > 99)
    {
        return WeatherParseResult::INVALID_FIELD;
    }

    WeatherReading parsed;
    parsed.temperature = temperature;
    parsed.humidity = static_cast<uint8_t>(std::lround(humidity));
    parsed.windSpeed = windSpeed;
    parsed.conditionCode = conditionCode;
    const JsonVariantConst apparent = current["apparent_temperature"];
    if (!apparent.isNull())
    {
        if (!isFiniteNumber(apparent))
        {
            return WeatherParseResult::INVALID_FIELD;
        }
        parsed.apparentTemperature = apparent.as<float>();
        parsed.apparentTemperatureAvailable = true;
    }

    std::snprintf(parsed.condition, sizeof(parsed.condition), "%s", conditionForCode(conditionCode));
    reading = parsed;
    return WeatherParseResult::OK;
}

const char* WeatherProvider::conditionForCode(int16_t weatherCode)
{
    if (weatherCode == 0) return "Clear";
    if (weatherCode >= 1 && weatherCode <= 3) return "Partly cloudy";
    if (weatherCode == 45 || weatherCode == 48) return "Fog";
    if (weatherCode >= 51 && weatherCode <= 57) return "Drizzle";
    if (weatherCode >= 61 && weatherCode <= 67) return "Rain";
    if (weatherCode >= 71 && weatherCode <= 77) return "Snow";
    if (weatherCode >= 80 && weatherCode <= 82) return "Rain showers";
    if (weatherCode == 85 || weatherCode == 86) return "Snow showers";
    if (weatherCode >= 95 && weatherCode <= 99) return "Thunderstorm";
    return "Unknown";
}

const char* WeatherProvider::parseError(WeatherParseResult result)
{
    switch (result)
    {
        case WeatherParseResult::OK: return "none";
        case WeatherParseResult::MALFORMED_RESPONSE: return "malformed response";
        case WeatherParseResult::MISSING_FIELD: return "missing weather field";
        case WeatherParseResult::INVALID_FIELD: return "invalid weather field";
        default: return "unknown parse error";
    }
}
