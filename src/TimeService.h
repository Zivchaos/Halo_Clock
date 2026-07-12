#pragma once

#include <time.h>

class TimeService
{
public:
    static void begin();
    static void update();
    static bool isSynchronized();
    static bool hasChanged();
    static const tm& localTime();
};
