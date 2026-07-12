#include "Clock.h"
#include "LedRing.h"
#include "Oled.h"
#include "TimeService.h"

void Clock::begin()
{
    TimeService::begin();
}

void Clock::update()
{
    TimeService::update();

    if (!TimeService::hasChanged())
    {
        return;
    }

    const tm& localTime = TimeService::localTime();
    Oled::time(localTime);
    LedRing::drawClock(localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
}
