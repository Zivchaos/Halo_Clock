#include "Halo.h"

#include <Arduino.h>

#include "Button.h"
#include "Clock.h"
#include "Config.h"
#include "DiagnosticsService.h"
#include "Hardware.h"
#include "Ledring.h"
#include "NetworkService.h"
#include "Oled.h"
#include "OtaService.h"
#include "RedAlertService.h"
#include "SettingsService.h"
#include "TimeService.h"
#include "Version.h"
#include "WebService.h"
#include "WeatherService.h"
#include <WiFi.h>

namespace
{
    bool ringCalibrationActive = false;
    bool otaDisplayWasReserved = false;
    bool redAlertWasActive = false;
    bool redAlertPhase = false;
    uint32_t redAlertPhaseAt = 0;
    void logDisplayMode(DisplayMode mode)
    {
        Serial.printf("DISPLAY MODE: %s\r\n", DisplayModes::name(mode));
    }
}

void Halo::begin()
{
    Serial.begin(Config::SERIAL_BAUD);
    ringCalibrationActive = false;
    otaDisplayWasReserved = false;
    redAlertWasActive = false;
    DiagnosticsService::begin();
    SettingsService::begin();
    Button::begin();
    Oled::begin();
    Oled::splash();
    Oled::status("LED Ring", "PASS");
    delay(400);
    Oled::status("OLED", "PASS");
    delay(400);
    Oled::status(Product::NAME, "READY");
    delay(400);
    LedRing::begin(SettingsService::brightness());
    LedRing::setRingCalibration(SettingsService::ringCalibration());
    LedRing::setCustomColors(SettingsService::customColors());
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
        NetworkService::begin();
        Clock::begin(selectedDisplayMode);
        OtaService::begin();
        WeatherService::begin();
        RedAlertService::begin();
        WebService::begin();
    }
    logDisplayMode(selectedDisplayMode);

    Serial.println();
    Serial.println("========================================");
    Serial.println(Product::NAME);
    Serial.println(Product::EXPANDED_NAME);
    Serial.print("Firmware : ");
    Serial.println(Product::FIRMWARE_VERSION);
    Serial.print("Build    : ");
    Serial.print(Product::BUILD_DATE);
    Serial.print(" ");
    Serial.println(Product::BUILD_TIME);
    Serial.println("Initialization Complete");
    Serial.println("========================================");
}

void Halo::update()
{
    OtaService::update();
    const bool otaDisplayReserved = OtaService::isDisplayReserved();
    if (ringCalibrationActive && otaDisplayWasReserved && !otaDisplayReserved)
    {
        Oled::status("RING CALIBRATION", "0  15  30  45");
        LedRing::showCalibrationTest();
    }
    otaDisplayWasReserved = otaDisplayReserved;
    WeatherService::update();
    RedAlertService::update();
    WebService::update();
    if constexpr (Config::ENABLE_WEATHER_OLED)
    {
        if (!otaDisplayReserved && !ringCalibrationActive &&
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
    else if (!OtaService::isUpdating() && buttonEvent == ButtonEvent::NetworkRecovery)
    {
        if (NetworkService::resetToDhcpAndReboot())
        {
            Oled::status("NETWORK RESET", "DHCP / REBOOT");
        }
    }

    const RedAlertData redAlert = RedAlertService::snapshot();
    if (redAlert.active && !otaDisplayReserved && !ringCalibrationActive)
    {
        if (!redAlertWasActive || millis() - redAlertPhaseAt >= 400)
        {
            redAlertPhase = !redAlertPhase;
            redAlertPhaseAt = millis();
            LedRing::showRedAlert(redAlertPhase);
        }
        Oled::redAlert(redAlert.areas, strcmp(redAlert.id, "simulation") == 0);
    }
    else if (redAlertWasActive)
    {
        Clock::requestRefresh();
    }
    redAlertWasActive = redAlert.active;

    if constexpr (!Config::ENABLE_RING_CALIBRATION)
    {
        Clock::update(!otaDisplayReserved && !ringCalibrationActive && !redAlert.active);
    }

    NetworkService::update();

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

bool Halo::setRingCalibration(const RingCalibrationSettings& settings, bool testActive)
{
    if (settings.zeroOffset >= Hardware::LED_COUNT) return false;
    if (!testActive && !SettingsService::saveRingCalibration(settings)) return false;
    LedRing::setRingCalibration(settings);
    ringCalibrationActive = testActive;
    if (testActive)
    {
        Oled::status("RING CALIBRATION", "0  15  30  45");
        LedRing::showCalibrationTest();
    }
    else
    {
        Clock::requestRefresh();
    }
    return true;
}

bool Halo::setCustomColors(const CustomColorSettings& settings)
{
    if (!SettingsService::saveCustomColors(settings)) return false;
    LedRing::setCustomColors(settings);
    Clock::requestRefresh();
    return true;
}

bool Halo::isRingCalibrationActive()
{
    return ringCalibrationActive;
}
