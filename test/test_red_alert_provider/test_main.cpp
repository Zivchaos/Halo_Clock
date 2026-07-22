#include <Arduino.h>
#include <cstring>
#include <unity.h>

#include "RedAlertProvider.h"

namespace
{
    constexpr const char* SELECTED_AREAS = "Tel Aviv\nHaifa\nJerusalem";

    void testEmptyCommunityNotificationsAreHealthy()
    {
        RedAlertReading reading;
        const RedAlertParseResult result = RedAlertProvider::parseResponse("[]", 2, SELECTED_AREAS, reading);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(RedAlertParseResult::OK), static_cast<int>(result));
        TEST_ASSERT_FALSE(reading.sourceAlert);
        TEST_ASSERT_FALSE(reading.matchesSelection);
    }

    void testAllCommunityNotificationsAreConsidered()
    {
        const char payload[] =
            R"([{"notificationId":"first","cities":["Eilat"]},{"notificationId":"second","cities":["Tel Aviv","Haifa"]}])";
        RedAlertReading reading;
        const RedAlertParseResult result = RedAlertProvider::parseResponse(payload, sizeof(payload) - 1, SELECTED_AREAS, reading);

        TEST_ASSERT_EQUAL_INT(static_cast<int>(RedAlertParseResult::OK), static_cast<int>(result));
        TEST_ASSERT_TRUE(reading.sourceAlert);
        TEST_ASSERT_TRUE(reading.matchesSelection);
        TEST_ASSERT_EQUAL_STRING("first", reading.id);
        TEST_ASSERT_NOT_NULL(strstr(reading.areas, "Tel Aviv"));
        TEST_ASSERT_NOT_NULL(strstr(reading.areas, "Haifa"));
    }

    void testOrefObjectFormatIsAccepted()
    {
        const char payload[] = R"({"id":"oref-42","title":"Test","data":["Jerusalem"]})";
        RedAlertReading reading;
        const RedAlertParseResult result = RedAlertProvider::parseResponse(payload, sizeof(payload) - 1, SELECTED_AREAS, reading);

        TEST_ASSERT_EQUAL_INT(static_cast<int>(RedAlertParseResult::OK), static_cast<int>(result));
        TEST_ASSERT_TRUE(reading.sourceAlert);
        TEST_ASSERT_TRUE(reading.matchesSelection);
        TEST_ASSERT_EQUAL_STRING("oref-42", reading.id);
        TEST_ASSERT_EQUAL_STRING("Test", reading.title);
        TEST_ASSERT_EQUAL_STRING("Jerusalem", reading.areas);
    }

    void testAreaMatchingIsExact()
    {
        const char payload[] = R"([{"notificationId":"one","cities":["Tel Aviv-Jaffa"]}])";
        RedAlertReading reading;
        const RedAlertParseResult result = RedAlertProvider::parseResponse(payload, sizeof(payload) - 1, SELECTED_AREAS, reading);

        TEST_ASSERT_EQUAL_INT(static_cast<int>(RedAlertParseResult::OK), static_cast<int>(result));
        TEST_ASSERT_TRUE(reading.sourceAlert);
        TEST_ASSERT_FALSE(reading.matchesSelection);
        TEST_ASSERT_EQUAL_STRING("", reading.areas);
    }

    void testMalformedResponseIsRejected()
    {
        const char payload[] = "[{";
        RedAlertReading reading;
        const RedAlertParseResult result = RedAlertProvider::parseResponse(payload, sizeof(payload) - 1, SELECTED_AREAS, reading);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(RedAlertParseResult::MALFORMED_RESPONSE), static_cast<int>(result));
    }
}

void setup()
{
    delay(1500);
    UNITY_BEGIN();
    RUN_TEST(testEmptyCommunityNotificationsAreHealthy);
    RUN_TEST(testAllCommunityNotificationsAreConsidered);
    RUN_TEST(testOrefObjectFormatIsAccepted);
    RUN_TEST(testAreaMatchingIsExact);
    RUN_TEST(testMalformedResponseIsRejected);
    UNITY_END();
}

void loop() {}
