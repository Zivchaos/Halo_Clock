#include "Clock.h"

#include "Ledring.h"
#include "Oled.h"
#include "TimeService.h"

#include <Arduino.h>

#include "Config.h"

namespace
{
    DisplayMode activeDisplayMode = DisplayMode::CLASSIC;
    uint32_t modeNoticeStartedAt = 0;
    bool modeNoticeVisible = false;
}

void Clock::begin(DisplayMode mode)
{
    activeDisplayMode = mode;
    TimeService::begin();
}

void Clock::setDisplayMode(DisplayMode mode)
{
    activeDisplayMode = mode;
    modeNoticeStartedAt = millis();
    modeNoticeVisible = true;
    Oled::displayMode(mode);
}

void Clock::update()
{
    TimeService::update();

    bool modeNoticeExpired = false;
    if (modeNoticeVisible && millis() - modeNoticeStartedAt >= Config::MODE_NOTICE_DURATION_MS)
    {
        modeNoticeVisible = false;
        modeNoticeExpired = true;
    }

    if (TimeService::hasChanged())
    {
        const tm& localTime = TimeService::localTime();
        LedRing::drawClock(
            localTime.tm_hour,
            localTime.tm_min,
            localTime.tm_sec,
            activeDisplayMode);
    }

    if (modeNoticeVisible || !TimeService::isSynchronized())
    {
        return;
    }

    if (TimeService::hasChanged() || modeNoticeExpired)
    {
        Oled::time(TimeService::localTime(), activeDisplayMode);
    }
}
