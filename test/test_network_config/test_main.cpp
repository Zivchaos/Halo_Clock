#include <Arduino.h>
#include <unity.h>

#include "Config.h"
#include "NetworkConfig.h"

namespace
{
    IPv4Address address(const char* text)
    {
        IPv4Address result;
        TEST_ASSERT_TRUE(NetworkConfig::parseIPv4(text, result));
        return result;
    }

    NetworkSettings validStatic()
    {
        NetworkSettings settings;
        settings.mode = NetworkMode::STATIC;
        settings.staticIp = address("192.168.50.42");
        settings.gateway = address("192.168.50.1");
        settings.subnet = address("255.255.255.0");
        settings.primaryDns = address("1.1.1.1");
        return settings;
    }

    void testDhcpIsDefaultAndValid()
    {
        const NetworkSettings settings = NetworkConfig::defaults();
        TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(NetworkMode::DHCP), static_cast<uint8_t>(settings.mode));
        TEST_ASSERT_TRUE(NetworkConfig::validate(settings).valid);
    }

    void testStrictIpv4ParsingAndFormatting()
    {
        IPv4Address parsed;
        TEST_ASSERT_TRUE(NetworkConfig::parseIPv4("10.20.30.40", parsed));
        char text[16];
        NetworkConfig::formatIPv4(parsed, text, sizeof(text));
        TEST_ASSERT_EQUAL_STRING("10.20.30.40", text);
        TEST_ASSERT_FALSE(NetworkConfig::parseIPv4("10.20.30", parsed));
        TEST_ASSERT_FALSE(NetworkConfig::parseIPv4("10.20.30.256", parsed));
        TEST_ASSERT_FALSE(NetworkConfig::parseIPv4("10.20.30.40x", parsed));
        TEST_ASSERT_FALSE(NetworkConfig::parseIPv4(" 10.20.30.40", parsed));
        TEST_ASSERT_FALSE(NetworkConfig::parseIPv4("", parsed));
    }

    void testValidStaticConfiguration()
    {
        TEST_ASSERT_TRUE(NetworkConfig::validate(validStatic()).valid);
    }

    void testInvalidSubnetMasksAreRejected()
    {
        NetworkSettings settings = validStatic();
        settings.subnet = address("255.0.255.0");
        TEST_ASSERT_FALSE(NetworkConfig::validate(settings).valid);
        settings.subnet = address("255.255.255.255");
        TEST_ASSERT_FALSE(NetworkConfig::validate(settings).valid);
        settings.subnet = address("0.0.0.0");
        TEST_ASSERT_FALSE(NetworkConfig::validate(settings).valid);
    }

    void testNetworkAndBroadcastAddressesAreRejected()
    {
        NetworkSettings settings = validStatic();
        settings.staticIp = address("192.168.50.0");
        TEST_ASSERT_FALSE(NetworkConfig::validate(settings).valid);
        settings.staticIp = address("192.168.50.255");
        TEST_ASSERT_FALSE(NetworkConfig::validate(settings).valid);
    }

    void testGatewayMustBelongToSubnet()
    {
        NetworkSettings settings = validStatic();
        settings.gateway = address("192.168.51.1");
        const NetworkValidationResult result = NetworkConfig::validate(settings);
        TEST_ASSERT_FALSE(result.valid);
        TEST_ASSERT_EQUAL_STRING("gateway is outside the configured subnet", result.error);
    }

    void testGatewayMustBeUsableHost()
    {
        NetworkSettings settings = validStatic();
        settings.gateway = address("192.168.50.255");
        TEST_ASSERT_FALSE(NetworkConfig::validate(settings).valid);
    }

    void testDnsValidation()
    {
        NetworkSettings settings = validStatic();
        settings.primaryDns = address("0.0.0.0");
        TEST_ASSERT_FALSE(NetworkConfig::validate(settings).valid);
        settings = validStatic();
        settings.secondaryDnsConfigured = true;
        settings.secondaryDns = address("224.0.0.1");
        TEST_ASSERT_FALSE(NetworkConfig::validate(settings).valid);
    }

    void testOptionalSecondaryDns()
    {
        NetworkSettings settings = validStatic();
        TEST_ASSERT_TRUE(NetworkConfig::validate(settings).valid);
        settings.secondaryDnsConfigured = true;
        settings.secondaryDns = address("8.8.8.8");
        TEST_ASSERT_TRUE(NetworkConfig::validate(settings).valid);
    }

    void testDhcpFallbackPolicy()
    {
        TEST_ASSERT_FALSE(NetworkConfig::shouldUseDhcpFallback(NetworkMode::DHCP, UINT8_MAX));
        TEST_ASSERT_FALSE(NetworkConfig::shouldUseDhcpFallback(NetworkMode::STATIC, Config::STATIC_NETWORK_FAILURE_LIMIT - 1U));
        TEST_ASSERT_TRUE(NetworkConfig::shouldUseDhcpFallback(NetworkMode::STATIC, Config::STATIC_NETWORK_FAILURE_LIMIT));
    }

    void testSettingsEqualityIncludesEveryField()
    {
        const NetworkSettings left = validStatic();
        NetworkSettings right = left;
        TEST_ASSERT_TRUE(NetworkConfig::equals(left, right));
        right.secondaryDnsConfigured = true;
        right.secondaryDns = address("8.8.8.8");
        TEST_ASSERT_FALSE(NetworkConfig::equals(left, right));
    }

    StaticConnectionDecision stableDecision()
    {
        return NetworkConfig::evaluateStaticConnection(
            true,
            true,
            Config::STATIC_NETWORK_CONFIRMATION_MS,
            Config::STATIC_NETWORK_CONFIRMATION_MS,
            0);
    }

    void testStaticWifiSucceedsWhileNtpFails()
    {
        // NTP is deliberately not an input to static-link evaluation.
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(StaticConnectionDecision::CONFIRMED),
            static_cast<uint8_t>(stableDecision()));
    }

    void testStaticWifiSucceedsWhileDnsFails()
    {
        // DNS is deliberately not an input to static-link evaluation.
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(StaticConnectionDecision::CONFIRMED),
            static_cast<uint8_t>(stableDecision()));
    }

    void testStaticWifiSucceedsWithoutInternet()
    {
        // Internet reachability is deliberately not an input.
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(StaticConnectionDecision::CONFIRMED),
            static_cast<uint8_t>(stableDecision()));
    }

    void testWifiAssociationFailureFailsAfterTimeout()
    {
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(StaticConnectionDecision::PENDING),
            static_cast<uint8_t>(NetworkConfig::evaluateStaticConnection(
                false, false, 0, Config::STATIC_NETWORK_VALIDATION_TIMEOUT_MS - 1U, 0)));
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(StaticConnectionDecision::FAILED),
            static_cast<uint8_t>(NetworkConfig::evaluateStaticConnection(
                false, false, 0, Config::STATIC_NETWORK_VALIDATION_TIMEOUT_MS, 0)));
    }

    void testIncorrectActiveIpFails()
    {
        const NetworkSettings configured = validStatic();
        TEST_ASSERT_FALSE(NetworkConfig::activeStaticConfigurationMatches(
            configured,
            address("192.168.50.43"),
            configured.gateway,
            configured.subnet));
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(StaticConnectionDecision::FAILED),
            static_cast<uint8_t>(NetworkConfig::evaluateStaticConnection(
                true, false, 0, 1000, 0)));
    }

    void testStableStaticConnectionConfirmsForFailureClear()
    {
        const NetworkSettings configured = validStatic();
        TEST_ASSERT_TRUE(NetworkConfig::activeStaticConfigurationMatches(
            configured,
            configured.staticIp,
            configured.gateway,
            configured.subnet));
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(StaticConnectionDecision::PENDING),
            static_cast<uint8_t>(NetworkConfig::evaluateStaticConnection(
                true, true, Config::STATIC_NETWORK_CONFIRMATION_MS - 1U,
                Config::STATIC_NETWORK_CONFIRMATION_MS, 0)));
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(StaticConnectionDecision::CONFIRMED),
            static_cast<uint8_t>(stableDecision()));
    }

    void testTwoGenuineStaticFailuresEnableTemporaryDhcpFallback()
    {
        TEST_ASSERT_FALSE(NetworkConfig::shouldUseDhcpFallback(
            NetworkMode::STATIC, Config::STATIC_NETWORK_FAILURE_LIMIT - 1U));
        TEST_ASSERT_TRUE(NetworkConfig::shouldUseDhcpFallback(
            NetworkMode::STATIC, Config::STATIC_NETWORK_FAILURE_LIMIT));
    }

    void testRepeatedPreconfirmationDropsFail()
    {
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(StaticConnectionDecision::PENDING),
            static_cast<uint8_t>(NetworkConfig::evaluateStaticConnection(
                false, false, 0, 5000,
                Config::STATIC_NETWORK_MAX_PRECONFIRM_DROPS - 1U)));
        TEST_ASSERT_EQUAL_UINT8(
            static_cast<uint8_t>(StaticConnectionDecision::FAILED),
            static_cast<uint8_t>(NetworkConfig::evaluateStaticConnection(
                false, false, 0, 5000,
                Config::STATIC_NETWORK_MAX_PRECONFIRM_DROPS)));
    }
}

void setup()
{
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(testDhcpIsDefaultAndValid);
    RUN_TEST(testStrictIpv4ParsingAndFormatting);
    RUN_TEST(testValidStaticConfiguration);
    RUN_TEST(testInvalidSubnetMasksAreRejected);
    RUN_TEST(testNetworkAndBroadcastAddressesAreRejected);
    RUN_TEST(testGatewayMustBelongToSubnet);
    RUN_TEST(testGatewayMustBeUsableHost);
    RUN_TEST(testDnsValidation);
    RUN_TEST(testOptionalSecondaryDns);
    RUN_TEST(testDhcpFallbackPolicy);
    RUN_TEST(testSettingsEqualityIncludesEveryField);
    RUN_TEST(testStaticWifiSucceedsWhileNtpFails);
    RUN_TEST(testStaticWifiSucceedsWhileDnsFails);
    RUN_TEST(testStaticWifiSucceedsWithoutInternet);
    RUN_TEST(testWifiAssociationFailureFailsAfterTimeout);
    RUN_TEST(testIncorrectActiveIpFails);
    RUN_TEST(testStableStaticConnectionConfirmsForFailureClear);
    RUN_TEST(testTwoGenuineStaticFailuresEnableTemporaryDhcpFallback);
    RUN_TEST(testRepeatedPreconfirmationDropsFail);
    UNITY_END();
}

void loop()
{
}
