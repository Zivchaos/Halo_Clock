#pragma once

#include <Arduino.h>

#include "NetworkConfig.h"

struct NetworkStatus
{
    NetworkMode configuredMode = NetworkMode::DHCP;
    NetworkMode effectiveMode = NetworkMode::DHCP;
    IPv4Address activeIp;
    IPv4Address gateway;
    IPv4Address subnet;
    IPv4Address primaryDns;
    IPv4Address secondaryDns;
    bool dhcpFallbackActive = false;
    uint8_t staticConnectionFailureCount = 0;
};

class NetworkService
{
public:
    static void begin();
    static void update();
    static NetworkStatus snapshot();
    static bool resetToDhcpAndReboot();
};
