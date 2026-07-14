#include "Halo.h"

#include <Arduino.h>

#include "Button.h"
#include "Clock.h"
#include "Config.h"
#include "DiagnosticsService.h"
#include "Ledring.h"
#include "Oled.h"
#include "OtaService.h"
#include "SettingsService.h"
#include "TimeService.h"
#include "Version.h"
#include "WebService.h"
#include "WeatherService.h"
#include <WiFi.h>

namespace
{
    void logDisplayMode(DisplayMode mode)
    {
        Serial.printf("DISPLAY MODE: %s\r\n", DisplayModes::name(mode));
    }
}

void Halo::begin()
{
    Serial.begin(Config::SERIAL_BAUD);
    DiagnosticsService::begin();
    SettingsService::begin();
    Button::begin();
    Oled::begin();
    Oled::splash();
    Oled::status("LED Ring", "PASS");
    delay(400);
    Oled::status("OLED", "PASS");
    delay(400);
    Oled::status("HALO", "READY");
    delay(400);
    LedRing::begin(SettingsService::brightness());
    LedRing::bootAnimation();
    delay(200);
    const DisplayMode selectedDisplayMode = SettingsService::displayMode();

    if constexpr (Config::ENABLE_RING_CALIBRATION)
    {
        Oled::status("RING CALIBRATION", "0  15  30  45");
        LedRing::showCalibrationTest();
    }
    else
    {
        Clock::begin(selectedDisplayMode);
        OtaService::begin();
        WeatherService::begin();
        WebService::begin();
    }
    logDisplayMode(selectedDisplayMode);

    Serial.println();
    Serial.println("========================================");
    Serial.println(Version::NAME);
    Serial.println(Version::TAGLINE);
    Serial.print("Firmware : ");
    Serial.println(Version::FIRMWARE);
    Serial.print("Build    : ");
    Serial.print(Version::BUILD_DATE);
    Serial.print(" ");
    Serial.println(Version::BUILD_TIME);
    Serial.println("Initialization Complete");
    Serial.println("========================================");
}

void Halo::update()
{
    OtaService::update();
    WeatherService::update();
    WebService::update();
    if constexpr (Config::ENABLE_WEATHER_OLED)
    {
        if (!OtaService::isDisplayReserved() &&
            WeatherService::consumeDisplayUpdate())
        {
            Clock::showWeather(WeatherService::snapshot());
        }
    }
    const ButtonEvent buttonEvent = Button::update();
    if (!OtaService::isUpdating() && buttonEvent == ButtonEvent::ShortPress)
    {
        setBrightness(LedRing::cycleBrightness());
    }
    else if (!OtaService::isUpdating() && buttonEvent == ButtonEvent::LongPress)
    {
        setDisplayMode(DisplayModes::next(SettingsService::displayMode()));
    }

    if constexpr (!Config::ENABLE_RING_CALIBRATION)
    {
        Clock::update(!OtaService::isDisplayReserved());
    }

    const bool wifiConnected = WiFi.status() == WL_CONNECTED;
    DiagnosticsService::update(
        wifiConnected,
        wifiConnected ? WiFi.RSSI() : 0,
        TimeService::isSynchronized(),
        OtaService::isReady(),
        OtaService::isUpdating());
}

bool Halo::setBrightness(uint8_t brightness)
{
    if (!LedRing::setBrightness(brightness))
    {
        return false;
    }
    SettingsService::saveBrightness(brightness);
    return true;
}

bool Halo::setDisplayMode(DisplayMode mode)
{
    if (!DisplayModes::isValid(static_cast<uint8_t>(mode)))
    {
        return false;
    }

    SettingsService::saveDisplayMode(mode);
    logDisplayMode(mode);
    if constexpr (!Config::ENABLE_RING_CALIBRATION)
    {
        Clock::setSelectedDisplayMode(mode);
    }
    return true;
}
