#include "Button.h"

#include <Arduino.h>

#include "Hardware.h"

namespace
{
    constexpr uint32_t DEBOUNCE_MS = 35;
    constexpr uint32_t DOUBLE_PRESS_MS = 350;
    constexpr uint32_t LONG_PRESS_MS = 1200;
    constexpr uint32_t VERY_LONG_PRESS_MS = 5000;

    bool stableState = HIGH;
    bool lastReading = HIGH;
    uint32_t readingChangedAt = 0;
    uint32_t pressedAt = 0;
    uint32_t releasedAt = 0;
    uint8_t clickCount = 0;
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
        }
        else
        {
            const uint32_t held = now - pressedAt;
            if (held >= VERY_LONG_PRESS_MS)
            {
                clickCount = 0;
                return ButtonEvent::VeryLongPress;
            }
            if (held >= LONG_PRESS_MS)
            {
                clickCount = 0;
                return ButtonEvent::LongPress;
            }
            ++clickCount;
            releasedAt = now;
        }
    }

    if (clickCount > 0 && now - releasedAt > DOUBLE_PRESS_MS)
    {
        const ButtonEvent event = clickCount >= 2 ? ButtonEvent::DoublePress : ButtonEvent::ShortPress;
        clickCount = 0;
        return event;
    }

    return ButtonEvent::None;
}
