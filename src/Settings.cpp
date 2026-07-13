#include "Settings.h"

#include <Preferences.h>

namespace
{
    Preferences preferences;
    HaloSettings values;

    uint8_t clampByte(uint8_t value, uint8_t minimum, uint8_t maximum)
    {
        return constrain(value, minimum, maximum);
    }

    void sanitize()
    {
        values.dayBrightness = clampByte(values.dayBrightness, 1, 255);
        values.nightBrightness = clampByte(values.nightBrightness, 1, 100);
        values.nightStartHour %= 24;
        values.dayStartHour %= 24;
        values.hourWidth = values.hourWidth == 5 ? 5 : (values.hourWidth == 1 ? 1 : 3);
        values.ledOffset %= 60;
        if (static_cast<uint8_t>(values.displayMode) >= static_cast<uint8_t>(DisplayMode::Count))
        {
            values.displayMode = DisplayMode::Progress;
        }
        if (static_cast<uint8_t>(values.theme) >= static_cast<uint8_t>(ThemeId::Count))
        {
            values.theme = ThemeId::Classic;
        }
        if (values.timezone.isEmpty())
        {
            values.timezone = "IST-2IDT,M3.4.4/26,M10.5.0";
        }
        values.latitude = constrain(values.latitude, -90.0F, 90.0F);
        values.longitude = constrain(values.longitude, -180.0F, 180.0F);
    }
}

void Settings::begin()
{
    preferences.begin("halo-clock", false);
    values.dayBrightness = preferences.getUChar("dayBright", values.dayBrightness);
    values.nightBrightness = preferences.getUChar("nightBright", values.nightBrightness);
    values.nightStartHour = preferences.getUChar("nightHour", values.nightStartHour);
    values.dayStartHour = preferences.getUChar("dayHour", values.dayStartHour);
    values.hourWidth = preferences.getUChar("hourWidth", values.hourWidth);
    values.ledOffset = preferences.getUChar("offset", values.ledOffset);
    values.clockwise = preferences.getBool("clockwise", values.clockwise);
    values.showTicks = preferences.getBool("ticks", values.showTicks);
    values.showSeconds = preferences.getBool("seconds", values.showSeconds);
    values.weatherEnabled = preferences.getBool("weather", values.weatherEnabled);
    values.displayMode = static_cast<DisplayMode>(preferences.getUChar("mode", 0));
    values.theme = static_cast<ThemeId>(preferences.getUChar("theme", 0));
    values.timezone = preferences.getString("timezone", values.timezone);
    values.latitude = preferences.getFloat("latitude", values.latitude);
    values.longitude = preferences.getFloat("longitude", values.longitude);
    sanitize();
}

const HaloSettings& Settings::get()
{
    return values;
}

HaloSettings& Settings::edit()
{
    return values;
}

void Settings::save()
{
    sanitize();
    preferences.putUChar("dayBright", values.dayBrightness);
    preferences.putUChar("nightBright", values.nightBrightness);
    preferences.putUChar("nightHour", values.nightStartHour);
    preferences.putUChar("dayHour", values.dayStartHour);
    preferences.putUChar("hourWidth", values.hourWidth);
    preferences.putUChar("offset", values.ledOffset);
    preferences.putBool("clockwise", values.clockwise);
    preferences.putBool("ticks", values.showTicks);
    preferences.putBool("seconds", values.showSeconds);
    preferences.putBool("weather", values.weatherEnabled);
    preferences.putUChar("mode", static_cast<uint8_t>(values.displayMode));
    preferences.putUChar("theme", static_cast<uint8_t>(values.theme));
    preferences.putString("timezone", values.timezone);
    preferences.putFloat("latitude", values.latitude);
    preferences.putFloat("longitude", values.longitude);
}

void Settings::reset()
{
    preferences.clear();
    values = HaloSettings{};
    save();
}
