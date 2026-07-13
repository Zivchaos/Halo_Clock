#include "Halo.h"

#include <Arduino.h>

#include "Clock.h"
#include "Config.h"
#include "Ledring.h"
#include "Oled.h"
#include "Version.h"

void Halo::begin()
{
    Serial.begin(Config::SERIAL_BAUD);
    Oled::begin();
    Oled::splash();
    Oled::status("LED Ring", "PASS");
    delay(400);
    Oled::status("OLED", "PASS");
    delay(400);
    Oled::status("HALO", "READY");
    delay(400);
    LedRing::begin();
    LedRing::bootAnimation();
    delay(200);

    Clock::begin();

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
    Clock::update();
}
