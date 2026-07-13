#include "Weather.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "Config.h"
#include "Notifications.h"
#include "Settings.h"

namespace
{
    WeatherData current;
    uint32_t lastAttempt = 0;
    bool forceRefresh = true;
    bool updating = false;

    bool fetchWeather()
    {
        const HaloSettings& settings = Settings::get();
        String url = "https://api.open-meteo.com/v1/forecast?latitude=";
        url += String(settings.latitude, 4);
        url += "&longitude=";
        url += String(settings.longitude, 4);
        url += "&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,weather_code,wind_speed_10m";
        url += "&daily=temperature_2m_max,temperature_2m_min&timezone=auto&forecast_days=1";

        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient http;
        http.setConnectTimeout(5000);
        http.setTimeout(5000);
        if (!http.begin(client, url))
        {
            return false;
        }

        const int status = http.GET();
        if (status != HTTP_CODE_OK)
        {
            http.end();
            return false;
        }

        JsonDocument document;
        const DeserializationError error = deserializeJson(document, http.getStream());
        http.end();
        if (error)
        {
            return false;
        }

        current.temperature = document["current"]["temperature_2m"] | 0.0F;
        current.apparentTemperature = document["current"]["apparent_temperature"] | 0.0F;
        current.humidity = document["current"]["relative_humidity_2m"] | 0.0F;
        current.windSpeed = document["current"]["wind_speed_10m"] | 0.0F;
        current.weatherCode = document["current"]["weather_code"] | 0;
        current.isDay = (document["current"]["is_day"] | 1) == 1;
        current.todayHigh = document["daily"]["temperature_2m_max"][0] | 0.0F;
        current.todayLow = document["daily"]["temperature_2m_min"][0] | 0.0F;
        current.updatedAt = millis();
        current.valid = true;
        return true;
    }
}

void Weather::begin()
{
    lastAttempt = millis();
    forceRefresh = true;
}

void Weather::update()
{
    if (!Settings::get().weatherEnabled || WiFi.status() != WL_CONNECTED || updating)
    {
        return;
    }

    if (!forceRefresh && millis() - lastAttempt < Config::WEATHER_REFRESH_MS)
    {
        return;
    }

    if (forceRefresh && millis() < 5000)
    {
        return;
    }

    forceRefresh = false;
    lastAttempt = millis();
    updating = true;
    const bool success = fetchWeather();
    updating = false;

    if (success)
    {
        Notifications::post(NotificationType::Success, "Weather updated", description(current.weatherCode), 1800);
    }
    else
    {
        Notifications::post(NotificationType::Warning, "Weather unavailable", "Will retry later", 2200);
    }
}

void Weather::refresh()
{
    forceRefresh = true;
}

const WeatherData& Weather::data()
{
    return current;
}

bool Weather::isUpdating()
{
    return updating;
}

const char* Weather::description(int weatherCode)
{
    if (weatherCode == 0) return "Clear sky";
    if (weatherCode <= 3) return "Partly cloudy";
    if (weatherCode == 45 || weatherCode == 48) return "Fog";
    if (weatherCode <= 57) return "Drizzle";
    if (weatherCode <= 67) return "Rain";
    if (weatherCode <= 77) return "Snow";
    if (weatherCode <= 82) return "Rain showers";
    if (weatherCode <= 86) return "Snow showers";
    if (weatherCode <= 99) return "Thunderstorm";
    return "Unknown";
}
