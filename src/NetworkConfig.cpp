#include "NetworkConfig.h"

#include "Config.h"

namespace
{
    uint32_t toUint32(const IPv4Address& address)
    {
        return (static_cast<uint32_t>(address.bytes[0]) << 24U) |
            (static_cast<uint32_t>(address.bytes[1]) << 16U) |
            (static_cast<uint32_t>(address.bytes[2]) << 8U) |
            static_cast<uint32_t>(address.bytes[3]);
    }

    bool isUnicastAddress(const IPv4Address& address)
    {
        const uint32_t value = toUint32(address);
        return value != 0U && value != 0xFFFFFFFFU &&
            address.bytes[0] != 127U && address.bytes[0] < 224U;
    }

    bool isUsableHost(
        const IPv4Address& address,
        const IPv4Address& subnet)
    {
        if (!isUnicastAddress(address))
        {
            return false;
        }

        const uint32_t value = toUint32(address);
        const uint32_t mask = toUint32(subnet);
        const uint32_t network = value & mask;
        const uint32_t broadcast = network | ~mask;
        return value != network && value != broadcast;
    }
}

NetworkSettings NetworkConfig::defaults()
{
    return NetworkSettings{};
}

bool NetworkConfig::parseIPv4(const char* text, IPv4Address& result)
{
    if (text == nullptr || *text == '\0')
    {
        return false;
    }

    IPv4Address parsed;
    const char* cursor = text;
    for (size_t octet = 0; octet < 4; ++octet)
    {
        if (*cursor < '0' || *cursor > '9')
        {
            return false;
        }

        uint16_t value = 0;
        size_t digits = 0;
        while (*cursor >= '0' && *cursor <= '9')
        {
            value = static_cast<uint16_t>(value * 10U + static_cast<uint8_t>(*cursor - '0'));
            if (value > 255U || ++digits > 3U)
            {
                return false;
            }
            ++cursor;
        }
        parsed.bytes[octet] = static_cast<uint8_t>(value);

        if (octet < 3)
        {
            if (*cursor != '.')
            {
                return false;
            }
            ++cursor;
        }
        else if (*cursor != '\0')
        {
            return false;
        }
    }

    result = parsed;
    return true;
}

void NetworkConfig::formatIPv4(
    const IPv4Address& address,
    char* output,
    size_t outputSize)
{
    if (output == nullptr || outputSize == 0)
    {
        return;
    }
    snprintf(
        output,
        outputSize,
        "%u.%u.%u.%u",
        address.bytes[0],
        address.bytes[1],
        address.bytes[2],
        address.bytes[3]);
}

bool NetworkConfig::isZero(const IPv4Address& address)
{
    return toUint32(address) == 0U;
}

bool NetworkConfig::equals(const IPv4Address& left, const IPv4Address& right)
{
    return memcmp(left.bytes, right.bytes, sizeof(left.bytes)) == 0;
}

bool NetworkConfig::equals(const NetworkSettings& left, const NetworkSettings& right)
{
    return left.mode == right.mode &&
        equals(left.staticIp, right.staticIp) &&
        equals(left.gateway, right.gateway) &&
        equals(left.subnet, right.subnet) &&
        equals(left.primaryDns, right.primaryDns) &&
        equals(left.secondaryDns, right.secondaryDns) &&
        left.secondaryDnsConfigured == right.secondaryDnsConfigured;
}

bool NetworkConfig::isValidMode(NetworkMode mode)
{
    return mode == NetworkMode::DHCP || mode == NetworkMode::STATIC;
}

bool NetworkConfig::isValidSubnetMask(const IPv4Address& maskAddress)
{
    const uint32_t mask = toUint32(maskAddress);
    if (mask == 0U || mask == 0xFFFFFFFFU)
    {
        return false;
    }

    bool zeroSeen = false;
    uint8_t prefixLength = 0;
    for (int8_t bit = 31; bit >= 0; --bit)
    {
        const bool set = (mask & (1UL << static_cast<uint8_t>(bit))) != 0U;
        if (!set)
        {
            zeroSeen = true;
        }
        else
        {
            if (zeroSeen)
            {
                return false;
            }
            ++prefixLength;
        }
    }
    return prefixLength <= 30U;
}

NetworkValidationResult NetworkConfig::validate(const NetworkSettings& settings)
{
    if (!isValidMode(settings.mode))
    {
        return {false, "invalid network mode"};
    }
    if (settings.mode == NetworkMode::DHCP)
    {
        return {true, ""};
    }
    if (!isValidSubnetMask(settings.subnet))
    {
        return {false, "invalid subnet mask"};
    }
    if (!isUsableHost(settings.staticIp, settings.subnet))
    {
        return {false, "static IP is not a usable host address"};
    }
    if (!isUsableHost(settings.gateway, settings.subnet))
    {
        return {false, "gateway is not a usable host address"};
    }

    const uint32_t mask = toUint32(settings.subnet);
    if ((toUint32(settings.staticIp) & mask) != (toUint32(settings.gateway) & mask))
    {
        return {false, "gateway is outside the configured subnet"};
    }
    if (!isUnicastAddress(settings.primaryDns))
    {
        return {false, "invalid primary DNS address"};
    }
    if (settings.secondaryDnsConfigured && !isUnicastAddress(settings.secondaryDns))
    {
        return {false, "invalid secondary DNS address"};
    }
    return {true, ""};
}

bool NetworkConfig::activeStaticConfigurationMatches(
    const NetworkSettings& configured,
    const IPv4Address& activeIp,
    const IPv4Address& activeGateway,
    const IPv4Address& activeSubnet)
{
    return configured.mode == NetworkMode::STATIC &&
        isValidSubnetMask(activeSubnet) &&
        isUsableHost(activeIp, activeSubnet) &&
        isUsableHost(activeGateway, activeSubnet) &&
        equals(activeIp, configured.staticIp) &&
        equals(activeGateway, configured.gateway) &&
        equals(activeSubnet, configured.subnet);
}

StaticConnectionDecision NetworkConfig::evaluateStaticConnection(
    bool wifiConnected,
    bool activeConfigurationMatches,
    uint32_t stableDurationMs,
    uint32_t attemptDurationMs,
    uint8_t preconfirmationDropCount)
{
    if (preconfirmationDropCount >= Config::STATIC_NETWORK_MAX_PRECONFIRM_DROPS)
    {
        return StaticConnectionDecision::FAILED;
    }
    if (!wifiConnected)
    {
        return attemptDurationMs >= Config::STATIC_NETWORK_VALIDATION_TIMEOUT_MS
            ? StaticConnectionDecision::FAILED
            : StaticConnectionDecision::PENDING;
    }
    if (!activeConfigurationMatches)
    {
        return StaticConnectionDecision::FAILED;
    }
    return stableDurationMs >= Config::STATIC_NETWORK_CONFIRMATION_MS
        ? StaticConnectionDecision::CONFIRMED
        : StaticConnectionDecision::PENDING;
}

bool NetworkConfig::shouldUseDhcpFallback(NetworkMode configuredMode, uint8_t failureCount)
{
    return configuredMode == NetworkMode::STATIC &&
        failureCount >= Config::STATIC_NETWORK_FAILURE_LIMIT;
}

const char* NetworkConfig::modeName(NetworkMode mode)
{
    return mode == NetworkMode::STATIC ? "STATIC" : "DHCP";
}
