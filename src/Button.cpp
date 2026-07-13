#include "Button.h"

#include <Arduino.h>

#include "Hardware.h"

namespace
{
    constexpr uint32_t DEBOUNCE_MS = 35;
    constexpr uint32_t LONG_PRESS_MS = 1200;

    bool stableState = HIGH;
    bool lastReading = HIGH;
    uint32_t readingChangedAt = 0;
    uint32_t pressedAt = 0;
    bool longPressReported = false;
}

void Button::begin()
{
    pinMode(Hardware::BUTTON_PIN, INPUT_PULLUP);
}

ButtonEvent Button::update()
{
    const bool reading = digitalRead(Hardware::BUTTON_PIN);
    const uint32_t now = millis();

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
            Serial.println("BUTTON PRESSED");
        }
        else
        {
            Serial.println("BUTTON RELEASED");
            if (!longPressReported)
            {
                Serial.println("SHORT PRESS");
                return ButtonEvent::ShortPress;
            }
        }
    }

    if (stableState == LOW && !longPressReported && now - pressedAt >= LONG_PRESS_MS)
    {
        longPressReported = true;
        Serial.println("LONG PRESS");
        return ButtonEvent::LongPress;
    }

    return ButtonEvent::None;
}
