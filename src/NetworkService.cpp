#include "NetworkService.h"

#include <WiFi.h>

#include "Config.h"
#include "SettingsService.h"

namespace
{
    NetworkMode configuredMode = NetworkMode::DHCP;
    NetworkMode effectiveMode = NetworkMode::DHCP;
    bool fallbackActive = false;
    bool staticConnectionValidated = false;
    bool failureRecorded = false;
    bool matchingConnectionActive = false;
    uint8_t preconfirmationDropCount = 0;
    bool rebootScheduled = false;
    uint32_t validationStartedAt = 0;
    uint32_t matchingConnectionStartedAt = 0;
    uint32_t rebootScheduledAt = 0;

    IPAddress toIPAddress(const IPv4Address& address)
    {
        return IPAddress(
            address.bytes[0],
            address.bytes[1],
            address.bytes[2],
            address.bytes[3]);
    }

    IPv4Address fromIPAddress(const IPAddress& address)
    {
        IPv4Address result;
        for (size_t index = 0; index < 4; ++index)
        {
            result.bytes[index] = address[index];
        }
        return result;
    }

    void scheduleReboot()
    {
        rebootScheduledAt = millis();
        rebootScheduled = true;
    }
}

void NetworkService::begin()
{
    const NetworkSettings& settings = SettingsService::network();
    configuredMode = settings.mode;
    fallbackActive = NetworkConfig::shouldUseDhcpFallback(
        configuredMode,
        SettingsService::staticNetworkFailureCount());
    effectiveMode = fallbackActive ? NetworkMode::DHCP : configuredMode;
    staticConnectionValidated = false;
    failureRecorded = false;
    matchingConnectionActive = false;
    preconfirmationDropCount = 0;
    rebootScheduled = false;
    validationStartedAt = millis();

    WiFi.mode(WIFI_STA);
    bool configured = false;
    if (effectiveMode == NetworkMode::STATIC)
    {
        configured = WiFi.config(
            toIPAddress(settings.staticIp),
            toIPAddress(settings.gateway),
            toIPAddress(settings.subnet),
            toIPAddress(settings.primaryDns),
            settings.secondaryDnsConfigured ? toIPAddress(settings.secondaryDns) : IPAddress());
    }
    else
    {
        configured = WiFi.config(IPAddress(), IPAddress(), IPAddress());
    }

    Serial.printf("NETWORK CONFIGURED MODE: %s\r\n", NetworkConfig::modeName(configuredMode));
    Serial.printf("NETWORK EFFECTIVE MODE: %s\r\n", NetworkConfig::modeName(effectiveMode));
    if (fallbackActive)
    {
        Serial.printf(
            "NETWORK DHCP FALLBACK: ACTIVE (%u static failures)\r\n",
            SettingsService::staticNetworkFailureCount());
    }
    if (!configured)
    {
        Serial.println("NETWORK ERROR: interface configuration failed");
    }
}

void NetworkService::update()
{
    const uint32_t now = millis();
    if (effectiveMode == NetworkMode::STATIC && !staticConnectionValidated && !failureRecorded)
    {
        const bool wifiConnected = WiFi.status() == WL_CONNECTED;
        bool activeConfigurationMatches = false;
        if (wifiConnected)
        {
            const NetworkSettings& configured = SettingsService::network();
            activeConfigurationMatches = NetworkConfig::activeStaticConfigurationMatches(
                configured,
                fromIPAddress(WiFi.localIP()),
                fromIPAddress(WiFi.gatewayIP()),
                fromIPAddress(WiFi.subnetMask()));

            if (activeConfigurationMatches && !matchingConnectionActive)
            {
                matchingConnectionActive = true;
                matchingConnectionStartedAt = now;
                Serial.println("NETWORK STATIC CONNECTION: STABILIZING");
            }
        }
        else if (matchingConnectionActive)
        {
            matchingConnectionActive = false;
            if (preconfirmationDropCount < UINT8_MAX)
            {
                ++preconfirmationDropCount;
            }
            Serial.printf(
                "NETWORK STATIC CONNECTION: PRECONFIRM DROP (%u)\r\n",
                preconfirmationDropCount);
        }

        if (!activeConfigurationMatches && wifiConnected)
        {
            matchingConnectionActive = false;
        }

        const uint32_t stableDuration = matchingConnectionActive
            ? now - matchingConnectionStartedAt
            : 0;
        const StaticConnectionDecision decision = NetworkConfig::evaluateStaticConnection(
            wifiConnected,
            activeConfigurationMatches,
            stableDuration,
            now - validationStartedAt,
            preconfirmationDropCount);

        if (decision == StaticConnectionDecision::CONFIRMED)
        {
            staticConnectionValidated = true;
            SettingsService::clearStaticNetworkFailures();
            Serial.println("NETWORK STATIC CONNECTION: VALIDATED");
        }
        else if (decision == StaticConnectionDecision::FAILED)
        {
            failureRecorded = true;
            const uint8_t failures = SettingsService::recordStaticNetworkFailure();
            Serial.printf("NETWORK STATIC CONNECTION: FAILED (%u)\r\n", failures);
            Serial.println("NETWORK REBOOT: RETRY OR DHCP FALLBACK");
            scheduleReboot();
        }
    }

    if (rebootScheduled && now - rebootScheduledAt >= Config::NETWORK_REBOOT_DELAY_MS)
    {
        rebootScheduled = false;
        ESP.restart();
    }
}

NetworkStatus NetworkService::snapshot()
{
    NetworkStatus status;
    status.configuredMode = configuredMode;
    status.effectiveMode = effectiveMode;
    status.dhcpFallbackActive = fallbackActive;
    status.staticConnectionFailureCount = SettingsService::staticNetworkFailureCount();
    if (WiFi.status() == WL_CONNECTED)
    {
        status.activeIp = fromIPAddress(WiFi.localIP());
        status.gateway = fromIPAddress(WiFi.gatewayIP());
        status.subnet = fromIPAddress(WiFi.subnetMask());
        status.primaryDns = fromIPAddress(WiFi.dnsIP(0));
        status.secondaryDns = fromIPAddress(WiFi.dnsIP(1));
    }
    return status;
}

bool NetworkService::resetToDhcpAndReboot()
{
    if (!SettingsService::resetNetworkToDhcp())
    {
        return false;
    }
    configuredMode = NetworkMode::DHCP;
    Serial.println("NETWORK RECOVERY: DHCP SAVED");
    scheduleReboot();
    return true;
}
