#include "Clock.h"

#include "AutoNightService.h"
#include "Ledring.h"
#include "Oled.h"
#include "TimeService.h"

#include <Arduino.h>

#include "Config.h"

namespace
{
    DisplayMode selectedDisplayMode = DisplayMode::CLASSIC;
    DisplayMode activeDisplayMode = DisplayMode::CLASSIC;
    uint32_t modeNoticeStartedAt = 0;
    bool modeNoticeVisible = false;

    void startNotice()
    {
        modeNoticeStartedAt = millis();
        modeNoticeVisible = true;
    }

    void applyEffectiveMode(DisplayMode mode)
    {
        if (mode == activeDisplayMode)
        {
            return;
        }

        activeDisplayMode = mode;
        LedRing::setDisplayMode(mode);
    }

    void showAutomaticNotice(AutoNightEvent event)
    {
        if (event == AutoNightEvent::ACTIVATED)
        {
            Oled::notice("AUTO NIGHT ON", activeDisplayMode);
            startNotice();
        }
        else if (event == AutoNightEvent::DEACTIVATED)
        {
            Oled::notice("AUTO NIGHT OFF", activeDisplayMode);
            startNotice();
        }
    }
}

void Clock::begin(DisplayMode mode)
{
    selectedDisplayMode = mode;
    activeDisplayMode = mode;
    LedRing::setDisplayMode(mode);
    AutoNightService::begin();
    TimeService::begin();
}

void Clock::setSelectedDisplayMode(DisplayMode mode)
{
    selectedDisplayMode = mode;

    if (AutoNightService::activateManualOverride())
    {
        applyEffectiveMode(mode);
        Oled::notice("MANUAL OVERRIDE", activeDisplayMode);
    }
    else
    {
        applyEffectiveMode(mode);
        Oled::displayMode(mode);
    }

    startNotice();
}

void Clock::update()
{
    TimeService::update();

    const AutoNightEvent autoNightEvent = AutoNightService::update(
        TimeService::isSynchronized(),
        TimeService::localTime());

    if (autoNightEvent != AutoNightEvent::NONE)
    {
        applyEffectiveMode(AutoNightService::effectiveMode(selectedDisplayMode));
        showAutomaticNotice(autoNightEvent);
    }

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
