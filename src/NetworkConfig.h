#pragma once

#include <Arduino.h>

enum class NetworkMode : uint8_t
{
    DHCP = 0,
    STATIC = 1
};

struct IPv4Address
{
    uint8_t bytes[4] = {0, 0, 0, 0};
};

struct NetworkSettings
{
    NetworkMode mode = NetworkMode::DHCP;
    IPv4Address staticIp;
    IPv4Address gateway;
    IPv4Address subnet;
    IPv4Address primaryDns;
    IPv4Address secondaryDns;
    bool secondaryDnsConfigured = false;
};

struct NetworkValidationResult
{
    bool valid;
    const char* error;
};

enum class StaticConnectionDecision : uint8_t
{
    PENDING,
    CONFIRMED,
    FAILED
};

namespace NetworkConfig
{
    NetworkSettings defaults();
    bool parseIPv4(const char* text, IPv4Address& result);
    void formatIPv4(const IPv4Address& address, char* output, size_t outputSize);
    bool isZero(const IPv4Address& address);
    bool equals(const IPv4Address& left, const IPv4Address& right);
    bool equals(const NetworkSettings& left, const NetworkSettings& right);
    bool isValidMode(NetworkMode mode);
    bool isValidSubnetMask(const IPv4Address& mask);
    NetworkValidationResult validate(const NetworkSettings& settings);
    bool activeStaticConfigurationMatches(
        const NetworkSettings& configured,
        const IPv4Address& activeIp,
        const IPv4Address& activeGateway,
        const IPv4Address& activeSubnet);
    StaticConnectionDecision evaluateStaticConnection(
        bool wifiConnected,
        bool activeConfigurationMatches,
        uint32_t stableDurationMs,
        uint32_t attemptDurationMs,
        uint8_t preconfirmationDropCount);
    bool shouldUseDhcpFallback(NetworkMode configuredMode, uint8_t failureCount);
    const char* modeName(NetworkMode mode);
}
