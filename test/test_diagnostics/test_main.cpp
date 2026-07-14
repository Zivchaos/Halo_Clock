#include <Arduino.h>
#include <unity.h>

#include "DiagnosticsService.h"

namespace
{
    void resetDiagnostics()
    {
        DiagnosticsService::begin();
        DiagnosticsService::update(false, 0, false, false, false);
    }

    void testBootMetadataAndHeapHaveValidValues()
    {
        resetDiagnostics();
        const DiagnosticsData data = DiagnosticsService::snapshot();
        TEST_ASSERT_GREATER_THAN_UINT32(0, data.freeHeap);
        TEST_ASSERT_GREATER_THAN_UINT32(0, data.minimumFreeHeap);
        TEST_ASSERT_NOT_EQUAL(0, strlen(data.resetReason));
        TEST_ASSERT_NOT_EQUAL(0, strlen(data.runningPartition));
        TEST_ASSERT_NOT_EQUAL(0, strlen(data.firmwareVersion));
        TEST_ASSERT_NOT_EQUAL(0, strlen(data.buildDate));
        TEST_ASSERT_NOT_EQUAL(0, strlen(data.buildTime));
    }

    void testInitialWifiConnectionIsNotAReconnect()
    {
        DiagnosticsService::begin();
        DiagnosticsService::update(true, -51, false, false, false);
        const DiagnosticsData data = DiagnosticsService::snapshot();
        TEST_ASSERT_TRUE(data.wifiConnected);
        TEST_ASSERT_EQUAL_INT32(-51, data.wifiRssi);
        TEST_ASSERT_EQUAL_UINT32(0, data.wifiReconnectCount);
    }

    void testWifiReconnectCountsOnlyDisconnectedToConnectedTransitions()
    {
        resetDiagnostics();
        DiagnosticsService::update(true, -60, false, false, false);
        DiagnosticsService::update(true, -59, false, false, false);
        DiagnosticsService::update(false, 0, false, false, false);
        DiagnosticsService::update(true, -58, false, false, false);
        const DiagnosticsData data = DiagnosticsService::snapshot();
        TEST_ASSERT_EQUAL_UINT32(1, data.wifiReconnectCount);
        TEST_ASSERT_EQUAL_INT32(-58, data.wifiRssi);
    }

    void testWeatherRequestCounterIncrements()
    {
        resetDiagnostics();
        DiagnosticsService::recordWeatherRequest();
        DiagnosticsService::recordWeatherRequest();
        TEST_ASSERT_EQUAL_UINT32(2, DiagnosticsService::snapshot().weatherRequestCount);
    }

    void testWeatherSuccessCounterIncrements()
    {
        resetDiagnostics();
        DiagnosticsService::recordWeatherSuccess();
        TEST_ASSERT_EQUAL_UINT32(1, DiagnosticsService::snapshot().weatherSuccessCount);
    }

    void testWeatherFailureCounterAndLastErrorUpdate()
    {
        resetDiagnostics();
        DiagnosticsService::recordWeatherFailure("controlled failure");
        const DiagnosticsData data = DiagnosticsService::snapshot();
        TEST_ASSERT_EQUAL_UINT32(1, data.weatherFailureCount);
        TEST_ASSERT_EQUAL_STRING("controlled failure", data.lastWeatherError);
    }

    void testSuccessfulWeatherRequestPreservesLastErrorHistory()
    {
        resetDiagnostics();
        DiagnosticsService::recordWeatherFailure("earlier failure");
        DiagnosticsService::recordWeatherSuccess();
        const DiagnosticsData data = DiagnosticsService::snapshot();
        TEST_ASSERT_EQUAL_UINT32(1, data.weatherFailureCount);
        TEST_ASSERT_EQUAL_UINT32(1, data.weatherSuccessCount);
        TEST_ASSERT_EQUAL_STRING("earlier failure", data.lastWeatherError);
    }

    void testTimeSyncAndOtaStatesAreReported()
    {
        resetDiagnostics();
        DiagnosticsService::recordTimeSync();
        DiagnosticsService::update(true, -48, true, true, true);
        const DiagnosticsData data = DiagnosticsService::snapshot();
        TEST_ASSERT_TRUE(data.ntpSynchronized);
        TEST_ASSERT_TRUE(data.timeSyncAgeValid);
        TEST_ASSERT_EQUAL_UINT32(0, data.lastTimeSyncAgeSeconds);
        TEST_ASSERT_TRUE(data.otaReady);
        TEST_ASSERT_TRUE(data.otaUpdating);
    }
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(testBootMetadataAndHeapHaveValidValues);
    RUN_TEST(testInitialWifiConnectionIsNotAReconnect);
    RUN_TEST(testWifiReconnectCountsOnlyDisconnectedToConnectedTransitions);
    RUN_TEST(testWeatherRequestCounterIncrements);
    RUN_TEST(testWeatherSuccessCounterIncrements);
    RUN_TEST(testWeatherFailureCounterAndLastErrorUpdate);
    RUN_TEST(testSuccessfulWeatherRequestPreservesLastErrorHistory);
    RUN_TEST(testTimeSyncAndOtaStatesAreReported);
    UNITY_END();
}

void loop()
{
}
