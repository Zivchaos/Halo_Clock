#pragma once

#include <Arduino.h>

#include "AppTypes.h"

struct HaloNotification
{
    NotificationType type = NotificationType::Info;
    String title;
    String message;
    uint32_t durationMs = 2500;
};

class Notifications
{
public:
    static void begin();
    static void update();
    static bool post(NotificationType type, const String& title, const String& message = "", uint32_t durationMs = 2500);
    static bool active();
    static const HaloNotification& current();
};
