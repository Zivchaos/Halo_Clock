#pragma once

enum class ButtonEvent
{
    None,
    ShortPress,
    LongPress
};

class Button
{
public:
    static void begin();
    static ButtonEvent update();
};
