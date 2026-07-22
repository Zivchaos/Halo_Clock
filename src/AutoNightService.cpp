#include "AutoNightService.h"

#include <Arduino.h>

#include "Config.h"
#include "SettingsService.h"

namespace
{
    bool scheduleKnown = false;
    bool scheduledNight = false;
    bool manualOverride = false;
    bool waitingLogged = false;
    bool configurationChanged = false;
    enum class ReportedState : uint8_t
    {
        UNKNOWN,
        WAITING_FOR_TIME,
        INACTIVE,
        ACTIVE
    };
    ReportedState reportedState = ReportedState::UNKNOWN;

    static_assert(Config::AUTO_NIGHT_START_HOUR < 24, "AUTO_NIGHT_START_HOUR must be 0-23");
    static_assert(Config::AUTO_NIGHT_END_HOUR < 24, "AUTO_NIGHT_END_HOUR must be 0-23");
    static_assert(Config::AUTO_NIGHT_START_MINUTE < 60, "AUTO_NIGHT_START_MINUTE must be 0-59");
    static_assert(Config::AUTO_NIGHT_END_MINUTE < 60, "AUTO_NIGHT_END_MINUTE must be 0-59");

    static_assert(!AutoNightSchedule::isActive(19, 59, 20, 0, 6, 0), "19:59 must be inactive");
    static_assert(AutoNightSchedule::isActive(20, 0, 20, 0, 6, 0), "20:00 must be active");
    static_assert(AutoNightSchedule::isActive(23, 59, 20, 0, 6, 0), "23:59 must be active");
    static_assert(AutoNightSchedule::isActive(0, 0, 20, 0, 6, 0), "00:00 must be active");
    static_assert(AutoNightSchedule::isActive(5, 59, 20, 0, 6, 0), "05:59 must be active");
    static_assert(!AutoNightSchedule::isActive(6, 0, 20, 0, 6, 0), "06:00 must be inactive");
    static_assert(!AutoNightSchedule::isActive(6, 1, 20, 0, 6, 0), "06:01 must be inactive");
    static_assert(!AutoNightSchedule::isActive(7, 59, 8, 0, 17, 0), "07:59 must be inactive");
    static_assert(AutoNightSchedule::isActive(8, 0, 8, 0, 17, 0), "08:00 must be active");
    static_assert(AutoNightSchedule::isActive(16, 59, 8, 0, 17, 0), "16:59 must be active");
    static_assert(!AutoNightSchedule::isActive(17, 0, 8, 0, 17, 0), "17:00 must be inactive");
    static_assert(!AutoNightSchedule::isActive(12, 0, 8, 0, 8, 0), "equal boundaries must be inactive");
    static_assert(!AutoNightSchedule::isActive(-1, 0, 20, 0, 6, 0), "negative hour must be invalid");
    static_assert(!AutoNightSchedule::isActive(24, 0, 20, 0, 6, 0), "hour 24 must be invalid");
    static_assert(!AutoNightSchedule::isActive(12, 60, 20, 0, 6, 0), "minute 60 must be invalid");
    static_assert(!AutoNightSchedule::isActive(12, 0, 24, 0, 6, 0), "invalid schedule must be inactive");

    bool isScheduledNight(const tm& localTime)
    {
        const AutoNightSettings& settings = SettingsService::autoNight();
        return AutoNightSchedule::isActive(
            localTime.tm_hour,
            localTime.tm_min,
            settings.startHour,
            settings.startMinute,
            settings.endHour,
            settings.endMinute);
    }

    void logWaitingForTime()
    {
        if (!waitingLogged || reportedState != ReportedState::WAITING_FOR_TIME)
        {
            waitingLogged = true;
            reportedState = ReportedState::WAITING_FOR_TIME;
            Serial.println("AUTO NIGHT: WAITING FOR VALID TIME");
        }
    }

    void logScheduledState(bool active)
    {
        const ReportedState nextState = active ? ReportedState::ACTIVE : ReportedState::INACTIVE;
        if (reportedState == nextState)
        {
            return;
        }

        reportedState = nextState;
        Serial.println(active ? "AUTO NIGHT: ACTIVE" : "AUTO NIGHT: INACTIVE");
    }
}

void AutoNightService::begin()
{
    scheduleKnown = !SettingsService::autoNight().enabled;
    scheduledNight = false;
    manualOverride = false;
    waitingLogged = false;
    configurationChanged = false;
    reportedState = ReportedState::UNKNOWN;

    if (SettingsService::autoNight().enabled)
    {
        logWaitingForTime();
    }
    else
    {
        logScheduledState(false);
    }
}

AutoNightEvent AutoNightService::update(bool timeValid, const tm& localTime)
{
    const bool enabled = SettingsService::autoNight().enabled;
    if (!enabled)
    {
        const bool wasScheduledNight = scheduleKnown && scheduledNight;
        const bool shouldLog = configurationChanged || !scheduleKnown || wasScheduledNight;
        scheduleKnown = true;
        scheduledNight = false;
        manualOverride = false;
        waitingLogged = false;
        configurationChanged = false;
        if (shouldLog)
        {
            logScheduledState(false);
        }
        return wasScheduledNight ? AutoNightEvent::DEACTIVATED : AutoNightEvent::NONE;
    }

    if (!timeValid)
    {
        const bool wasAutomaticallyActive = scheduleKnown && scheduledNight && !manualOverride;
        scheduleKnown = false;
        scheduledNight = false;
        manualOverride = false;
        configurationChanged = false;
        logWaitingForTime();
        return wasAutomaticallyActive ? AutoNightEvent::TIME_INVALID : AutoNightEvent::NONE;
    }

    waitingLogged = false;
    const bool nextScheduledNight = isScheduledNight(localTime);

    if (!scheduleKnown || nextScheduledNight != scheduledNight || configurationChanged)
    {
        const bool previousScheduledNight = scheduleKnown && scheduledNight;
        const bool wasManualOverride = manualOverride;
        scheduleKnown = true;
        scheduledNight = nextScheduledNight;
        manualOverride = false;
        configurationChanged = false;

        logScheduledState(scheduledNight);
        if (scheduledNight != previousScheduledNight)
        {
            return scheduledNight ? AutoNightEvent::ACTIVATED : AutoNightEvent::DEACTIVATED;
        }
        if (scheduledNight && wasManualOverride)
        {
            return AutoNightEvent::ACTIVATED;
        }
    }

    return AutoNightEvent::NONE;
}

void AutoNightService::settingsChanged()
{
    configurationChanged = true;
}

bool AutoNightService::activateManualOverride()
{
    if (!SettingsService::autoNight().enabled || !scheduleKnown || !scheduledNight)
    {
        return false;
    }

    if (manualOverride)
    {
        return false;
    }

    manualOverride = true;
    Serial.println("AUTO NIGHT: MANUAL OVERRIDE");
    return true;
}

DisplayMode AutoNightService::effectiveMode(DisplayMode selectedMode)
{
    if (SettingsService::autoNight().enabled && scheduleKnown && scheduledNight && !manualOverride)
    {
        return DisplayMode::NIGHT;
    }

    return selectedMode;
}

bool AutoNightService::isActive()
{
    return SettingsService::autoNight().enabled && scheduleKnown && scheduledNight;
}

bool AutoNightService::isManualOverride()
{
    return manualOverride;
}
