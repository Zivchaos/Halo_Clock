#pragma once

#include <Arduino.h>

class OtaService
{
public:
    static void begin();
    static void update();
    static bool isUpdating();
    static bool isReady();
    static bool isDisplayReserved();
    static uint8_t progressPercent();
    static bool enableForSession(const char* password);
    static bool changePassword(const char* currentPassword, const char* newPassword);
    static bool disableForSession();
    static bool isConfigured();
};
