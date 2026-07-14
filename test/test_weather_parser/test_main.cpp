#include <Arduino.h>
#include <cstring>
#include <unity.h>

#include "Config.h"
#include "WeatherProvider.h"
#include "WeatherService.h"

namespace
{
    void testProviderBuildsCurrentConditionsUrl()
    {
        char url[512];
        TEST_ASSERT_TRUE(WeatherProvider::buildRequestUrl(url, sizeof(url)));
        TEST_ASSERT_EQUAL_INT(0, strncmp(Config::WEATHER_API_URL, "https://", 8));
        TEST_ASSERT_NOT_NULL(strstr(url, Config::WEATHER_API_URL));
        TEST_ASSERT_NOT_NULL(strstr(url, "latitude="));
        TEST_ASSERT_NOT_NULL(strstr(url, "longitude="));
        TEST_ASSERT_NOT_NULL(strstr(url, "current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,wind_speed_10m"));
        TEST_ASSERT_NULL(strstr(url, "hourly="));
        TEST_ASSERT_NULL(strstr(url, "daily="));

        char undersized[16];
        TEST_ASSERT_FALSE(WeatherProvider::buildRequestUrl(undersized, sizeof(undersized)));
    }

    void testValidWeatherResponse()
    {
        const char payload[] =
            R"({"current":{"temperature_2m":27.4,"apparent_temperature":29.1,"relative_humidity_2m":64,"weather_code":1,"wind_speed_10m":12.5}})";
        WeatherReading reading;
        const WeatherParseResult result = WeatherProvider::parseResponse(payload, sizeof(payload) - 1, reading);

        TEST_ASSERT_EQUAL_INT(static_cast<int>(WeatherParseResult::OK), static_cast<int>(result));
        TEST_ASSERT_FLOAT_WITHIN(0.01F, 27.4F, reading.temperature);
        TEST_ASSERT_TRUE(reading.apparentTemperatureAvailable);
        TEST_ASSERT_FLOAT_WITHIN(0.01F, 29.1F, reading.apparentTemperature);
        TEST_ASSERT_EQUAL_UINT8(64, reading.humidity);
        TEST_ASSERT_FLOAT_WITHIN(0.01F, 12.5F, reading.windSpeed);
        TEST_ASSERT_EQUAL_STRING("Partly cloudy", reading.condition);
    }

    void testApparentTemperatureIsOptional()
    {
        const char payload[] =
            R"({"current":{"temperature_2m":18,"relative_humidity_2m":48,"weather_code":0,"wind_speed_10m":4}})";
        WeatherReading reading;
        const WeatherParseResult result = WeatherProvider::parseResponse(payload, sizeof(payload) - 1, reading);

        TEST_ASSERT_EQUAL_INT(static_cast<int>(WeatherParseResult::OK), static_cast<int>(result));
        TEST_ASSERT_FALSE(reading.apparentTemperatureAvailable);
        TEST_ASSERT_EQUAL_STRING("Clear", reading.condition);
    }

    void testMissingRequiredFieldIsRejected()
    {
        const char payload[] =
            R"({"current":{"temperature_2m":21,"relative_humidity_2m":50,"weather_code":2}})";
        WeatherReading reading;
        const WeatherParseResult result = WeatherProvider::parseResponse(payload, sizeof(payload) - 1, reading);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(WeatherParseResult::MISSING_FIELD), static_cast<int>(result));
    }

    void testMalformedResponseIsRejected()
    {
        const char payload[] = R"({"current":)";
        WeatherReading reading;
        const WeatherParseResult result = WeatherProvider::parseResponse(payload, sizeof(payload) - 1, reading);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(WeatherParseResult::MALFORMED_RESPONSE), static_cast<int>(result));
    }

    void testInvalidFieldIsRejected()
    {
        const char payload[] =
            R"({"current":{"temperature_2m":20,"relative_humidity_2m":101,"weather_code":3,"wind_speed_10m":8}})";
        WeatherReading reading;
        const WeatherParseResult result = WeatherProvider::parseResponse(payload, sizeof(payload) - 1, reading);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(WeatherParseResult::INVALID_FIELD), static_cast<int>(result));
    }

    void testStalePolicyUsesConfiguredAge()
    {
        TEST_ASSERT_FALSE(WeatherPolicy::isStale(
            false,
            Config::WEATHER_STALE_AFTER_MS,
            0U,
            Config::WEATHER_STALE_AFTER_MS));
        TEST_ASSERT_FALSE(WeatherPolicy::isStale(
            true,
            Config::WEATHER_STALE_AFTER_MS - 1U,
            0U,
            Config::WEATHER_STALE_AFTER_MS));
        TEST_ASSERT_TRUE(WeatherPolicy::isStale(
            true,
            Config::WEATHER_STALE_AFTER_MS,
            0U,
            Config::WEATHER_STALE_AFTER_MS));
    }

    void testSchedulingHandlesMillisWraparound()
    {
        TEST_ASSERT_FALSE(WeatherPolicy::isDue(0xFFFFFFF0U, 0x00000010U));
        TEST_ASSERT_TRUE(WeatherPolicy::isDue(0x00000010U, 0xFFFFFFF0U));
    }

    void testFailurePreservesCachedWeather()
    {
        WeatherData cached;
        cached.valid = true;
        cached.updating = true;
        cached.temperature = 28.5F;
        cached.humidity = 61;
        cached.lastSuccessfulUpdateMs = 12345U;

        WeatherPolicy::applyFailure(cached, "timeout");

        TEST_ASSERT_TRUE(cached.valid);
        TEST_ASSERT_FALSE(cached.updating);
        TEST_ASSERT_FLOAT_WITHIN(0.01F, 28.5F, cached.temperature);
        TEST_ASSERT_EQUAL_UINT8(61, cached.humidity);
        TEST_ASSERT_EQUAL_UINT32(12345U, cached.lastSuccessfulUpdateMs);
        TEST_ASSERT_EQUAL_STRING("timeout", cached.error);
    }

    void testOfflineAndOverlapPoliciesSuppressRequests()
    {
        TEST_ASSERT_FALSE(WeatherPolicy::shouldStartRequest(false, false, false, 5000U, 1000U));
        TEST_ASSERT_FALSE(WeatherPolicy::shouldStartRequest(false, false, true, 5000U, 1000U));
        TEST_ASSERT_FALSE(WeatherPolicy::shouldStartRequest(true, true, true, 5000U, 1000U));
        TEST_ASSERT_TRUE(WeatherPolicy::shouldStartRequest(true, false, false, 5000U, 1000U));
        TEST_ASSERT_TRUE(WeatherPolicy::shouldStartRequest(true, false, true, 500U, 1000U));
    }

    void testUpdateEpochCanBeBackfilledAfterNtpSync()
    {
        TEST_ASSERT_EQUAL_INT32(
            1700000040L,
            static_cast<int32_t>(
                WeatherPolicy::estimateUpdateEpoch(1700000100L, 100000U, 40000U)));
    }
}

void setup()
{
    delay(1500);
    UNITY_BEGIN();
    RUN_TEST(testProviderBuildsCurrentConditionsUrl);
    RUN_TEST(testValidWeatherResponse);
    RUN_TEST(testApparentTemperatureIsOptional);
    RUN_TEST(testMissingRequiredFieldIsRejected);
    RUN_TEST(testMalformedResponseIsRejected);
    RUN_TEST(testInvalidFieldIsRejected);
    RUN_TEST(testStalePolicyUsesConfiguredAge);
    RUN_TEST(testSchedulingHandlesMillisWraparound);
    RUN_TEST(testFailurePreservesCachedWeather);
    RUN_TEST(testOfflineAndOverlapPoliciesSuppressRequests);
    RUN_TEST(testUpdateEpochCanBeBackfilledAfterNtpSync);
    UNITY_END();
}

void loop()
{
}
