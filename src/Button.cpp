#include "Button.h"

#include <Arduino.h>

#include "Config.h"
#include "Hardware.h"

namespace
{
    constexpr uint32_t DEBOUNCE_MS = 35;
    constexpr uint32_t LONG_PRESS_MS = 1200;

    bool stableState = HIGH;
    bool lastReading = HIGH;
    uint32_t readingChangedAt = 0;
    uint32_t pressedAt = 0;
    uint32_t recoveryWindowStartedAt = 0;
    bool longPressReported = false;
    bool bootWindowPress = false;
    bool recoveryWindowStarted = false;
}

void Button::begin()
{
    pinMode(Hardware::BUTTON_PIN, INPUT_PULLUP);
    recoveryWindowStarted = false;
}

ButtonEvent Button::update()
{
    const bool reading = digitalRead(Hardware::BUTTON_PIN);
    const uint32_t now = millis();
    if (!recoveryWindowStarted)
    {
        recoveryWindowStartedAt = now;
        recoveryWindowStarted = true;
    }

    if (reading != lastReading)
    {
        lastReading = reading;
        readingChangedAt = now;
    }

    if (now - readingChangedAt >= DEBOUNCE_MS && reading != stableState)
    {
        stableState = reading;
        if (stableState == LOW)
        {
            pressedAt = now;
            longPressReported = false;
            bootWindowPress =
                now - recoveryWindowStartedAt <= Config::NETWORK_RECOVERY_BOOT_WINDOW_MS;
            Serial.println("BUTTON PRESSED");
        }
        else
        {
            Serial.println("BUTTON RELEASED");
            if (!longPressReported)
            {
                if (bootWindowPress && now - pressedAt >= LONG_PRESS_MS)
                {
                    longPressReported = true;
                    Serial.println("LONG PRESS");
                    return ButtonEvent::LongPress;
                }
                else
                {
                    Serial.println("SHORT PRESS");
                    return ButtonEvent::ShortPress;
                }
            }
        }
    }

    if (stableState == LOW && !longPressReported && now - pressedAt >= LONG_PRESS_MS)
    {
        if (bootWindowPress)
        {
            if (now - pressedAt >= Config::NETWORK_RECOVERY_HOLD_MS)
            {
                longPressReported = true;
                Serial.println("NETWORK RECOVERY PRESS");
                return ButtonEvent::NetworkRecovery;
            }
        }
        else
        {
            longPressReported = true;
            Serial.println("LONG PRESS");
            return ButtonEvent::LongPress;
        }
    }

    return ButtonEvent::None;
}
