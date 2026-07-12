#include "Oled.h"

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#include "Hardware.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(
    U8G2_R0,
    U8X8_PIN_NONE);

void Oled::begin()
{
    Wire.begin(
        Hardware::OLED_SDA,
        Hardware::OLED_SCL);

    oled.begin();

    oled.setContrast(255);

    oled.clearBuffer();

    oled.sendBuffer();
}

void Oled::splash()
{
    oled.clearBuffer();

    oled.setFont(u8g2_font_logisoso20_tf);
    oled.drawStr(15,28,"HALO");

    oled.setFont(u8g2_font_6x12_tf);
    oled.drawStr(18,48,"Connected Timepiece");

    oled.sendBuffer();
}

void Oled::status(
    const char* line1,
    const char* line2)
{
    oled.clearBuffer();

    oled.setFont(u8g2_font_6x12_tf);

    oled.drawStr(0,20,line1);
    oled.drawStr(0,40,line2);

    oled.sendBuffer();
}