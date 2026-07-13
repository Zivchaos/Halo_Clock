#pragma once

#include "DisplayMode.h"

class Clock
{
public:

    static void begin(DisplayMode mode);

    static void update();

    static void setSelectedDisplayMode(DisplayMode mode);

};
