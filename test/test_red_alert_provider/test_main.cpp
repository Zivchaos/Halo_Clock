#include <Arduino.h>
#include <cstring>
#include <unity.h>

#include "Config.h"
#include "RedAlertPolicy.h"
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

    void testPollingOnlyStartsWhenEverySafetyConditionIsMet()
    {
        constexpr uint32_t NOW = 5000U;
        constexpr uint32_t DUE = 1000U;
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldStartRequest(false, true, false, false, false, NOW, DUE));
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldStartRequest(true, false, false, false, false, NOW, DUE));
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldStartRequest(true, true, true, false, false, NOW, DUE));
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldStartRequest(true, true, false, true, false, NOW, DUE));
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldStartRequest(true, true, false, false, true, NOW, DUE));
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldStartRequest(true, true, false, false, false, DUE - 1U, DUE));
        TEST_ASSERT_TRUE(RedAlertPolicy::shouldStartRequest(true, true, false, false, false, NOW, DUE));
    }

    void testSchedulingSupportsBackoffAndMillisWraparound()
    {
        TEST_ASSERT_EQUAL_UINT32(16000U, RedAlertPolicy::nextAttemptAfterSuccess(1000U, 15000U));
        TEST_ASSERT_EQUAL_UINT32(31000U, RedAlertPolicy::nextAttemptAfterFailure(1000U, 30000U));
        TEST_ASSERT_FALSE(RedAlertPolicy::isDue(0xFFFFFFF0U, 0x00000010U));
        TEST_ASSERT_TRUE(RedAlertPolicy::isDue(0x00000010U, 0xFFFFFFF0U));
    }

    void testLowHeapGuardUsesStrictSafetyBoundary()
    {
        constexpr uint32_t MINIMUM = 160UL * 1024UL;
        TEST_ASSERT_TRUE(RedAlertPolicy::shouldSuspendForLowHeap(MINIMUM - 1U, MINIMUM));
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldSuspendForLowHeap(MINIMUM, MINIMUM));
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldSuspendForLowHeap(MINIMUM + 1U, MINIMUM));
    }

    void testPollingDefersAtMinuteTransition()
    {
        constexpr uint8_t GUARD_SECONDS = 3U;
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldDeferForMinuteTransition(false, 0U, GUARD_SECONDS));
        TEST_ASSERT_TRUE(RedAlertPolicy::shouldDeferForMinuteTransition(true, 0U, GUARD_SECONDS));
        TEST_ASSERT_TRUE(RedAlertPolicy::shouldDeferForMinuteTransition(true, 2U, GUARD_SECONDS));
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldDeferForMinuteTransition(true, 3U, GUARD_SECONDS));
        TEST_ASSERT_FALSE(RedAlertPolicy::shouldDeferForMinuteTransition(true, 56U, GUARD_SECONDS));
        TEST_ASSERT_TRUE(RedAlertPolicy::shouldDeferForMinuteTransition(true, 57U, GUARD_SECONDS));
        TEST_ASSERT_TRUE(RedAlertPolicy::shouldDeferForMinuteTransition(true, 59U, GUARD_SECONDS));
    }

    void testLiveRedAlertDefaultsToDisabled()
    {
        TEST_ASSERT_FALSE(Config::RED_ALERT_DEFAULT_ENABLED);
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
    RUN_TEST(testPollingOnlyStartsWhenEverySafetyConditionIsMet);
    RUN_TEST(testSchedulingSupportsBackoffAndMillisWraparound);
    RUN_TEST(testLowHeapGuardUsesStrictSafetyBoundary);
    RUN_TEST(testPollingDefersAtMinuteTransition);
    RUN_TEST(testLiveRedAlertDefaultsToDisabled);
    UNITY_END();
}

void loop() {}
