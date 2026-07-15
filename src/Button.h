#pragma once

enum class ButtonEvent
{
    None,
    ShortPress,
    LongPress,
    NetworkRecovery
};

class Button
{
public:
    static void begin();
    static ButtonEvent update();
};
