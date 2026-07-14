#pragma once

#include "DisplayMode.h"

class Clock
{
public:

    static void begin(DisplayMode mode);

    static void update(bool renderEnabled = true);

    static void setSelectedDisplayMode(DisplayMode mode);

    static DisplayMode selectedMode();
    static DisplayMode effectiveMode();

};
