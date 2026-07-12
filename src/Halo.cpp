#include "Halo.h"
#include "LedRing.h"
#include <Arduino.h>
#include "Oled.h"
#include "Version.h"
#include "Config.h"
#include "Clock.h"


void Halo::begin()
{
    Serial.begin(Config::SERIAL_BAUD);
    Oled::begin();
    Oled::splash();
    Oled::status("LED Ring","PASS");
    delay(400);
    Oled::status("OLED","PASS");
    delay(400);
    Oled::status("HALO","READY");
    delay(400);
    LedRing::begin();
    LedRing::bootAnimation();
    delay(200);
    Clock::begin();
    Serial.println();
    Serial.println("========================================");
    Serial.println();
    Serial.println(Version::NAME);
    Serial.println(Version::TAGLINE);

    Serial.print("Firmware : ");
    Serial.println(Version::FIRMWARE);

    Serial.print("Build    : ");
    Serial.print(Version::BUILD_DATE);
    Serial.print(" ");
    Serial.println(Version::BUILD_TIME);

    Serial.println();
    Serial.println("Initialization Complete");
    Serial.println("========================================");
}

void Halo::update()
{
    static uint32_t previousHeartbeat = 0;

    if (millis() - previousHeartbeat >= 1000)
    {
        previousHeartbeat = millis();

        Serial.println("[HALO] Alive");
    }
}