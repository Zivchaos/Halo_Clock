#pragma once

#include "DisplayMode.h"

class Clock
{
public:

    static void begin(DisplayMode mode);

    static void update();

    static void setDisplayMode(DisplayMode mode);

};
