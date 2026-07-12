#include "Clock.h"
#include <WiFi.h>
#include <time.h>
#include "LedRing.h"

void Clock::begin()
{
    configTime(0, 0, "pool.ntp.org");

    setenv("TZ", "IST-2IDT,M3.4.4/26,M10.5.0", 1);
    tzset();
}

void Clock::update()
{
    time_t now;
    struct tm timeinfo;

    time(&now);

    localtime_r(&now, &timeinfo);

    LedRing::drawClock(
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec);
}