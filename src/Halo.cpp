#include "Halo.h"

#include <Arduino.h>

#include "Button.h"
#include "Clock.h"
#include "Config.h"
#include "Ledring.h"
#include "Oled.h"
#include "OtaService.h"
#include "SettingsService.h"
#include "Version.h"

namespace
{
    DisplayMode selectedDisplayMode = DisplayMode::CLASSIC;

    void logDisplayMode()
    {
        Serial.printf("DISPLAY MODE: %s\r\n", DisplayModes::name(selectedDisplayMode));
    }
}

void Halo::begin()
{
    Serial.begin(Config::SERIAL_BAUD);
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
    selectedDisplayMode = SettingsService::displayMode();

    if constexpr (Config::ENABLE_RING_CALIBRATION)
    {
        Oled::status("RING CALIBRATION", "0  15  30  45");
        LedRing::showCalibrationTest();
    }
    else
    {
        Clock::begin(selectedDisplayMode);
        OtaService::begin();
    }
    logDisplayMode();

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
    const ButtonEvent buttonEvent = Button::update();
    if (!OtaService::isUpdating() && buttonEvent == ButtonEvent::ShortPress)
    {
        SettingsService::saveBrightness(LedRing::cycleBrightness());
    }
    else if (!OtaService::isUpdating() && buttonEvent == ButtonEvent::LongPress)
    {
        selectedDisplayMode = DisplayModes::next(selectedDisplayMode);
        SettingsService::saveDisplayMode(selectedDisplayMode);
        logDisplayMode();

        if constexpr (!Config::ENABLE_RING_CALIBRATION)
        {
            Clock::setSelectedDisplayMode(selectedDisplayMode);
        }
    }

    if constexpr (!Config::ENABLE_RING_CALIBRATION)
    {
        Clock::update(!OtaService::isDisplayReserved());
    }
}
