#pragma once

enum class ButtonEvent
{
    None,
    ShortPress,
    DoublePress,
    LongPress,
    VeryLongPress
};

class Button
{
public:
    static void begin();
    static ButtonEvent update();
};
