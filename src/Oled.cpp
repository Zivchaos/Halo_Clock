#include "Oled.h"

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "Hardware.h"

namespace
{
    U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);

    void centerText(const char* text, uint8_t baseline)
    {
        const int16_t measured = (128 - oled.getStrWidth(text)) / 2;
        const int16_t x = measured > 0 ? measured : 0;
        oled.drawStr(x, baseline, text);
    }
}

void Oled::begin()
{
    Wire.begin(Hardware::OLED_SDA, Hardware::OLED_SCL);
    oled.setI2CAddress(Hardware::OLED_ADDRESS << 1);
    oled.begin();
    oled.setContrast(255);
    oled.clearBuffer();
    oled.sendBuffer();
}

void Oled::splash()
{
    oled.clearBuffer();
    oled.setFont(u8g2_font_logisoso20_tf);
    oled.drawStr(15, 28, "HALO");
    oled.setFont(u8g2_font_6x12_tf);
    oled.drawStr(18, 48, "Connected Timepiece");
    oled.sendBuffer();
}

void Oled::status(const char* line1, const char* line2)
{
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x12_tf);
    oled.drawStr(0, 20, line1);
    oled.drawStr(0, 40, line2);
    oled.sendBuffer();
}

void Oled::time(const tm& localTime)
{
    char timeText[9];
    char dateText[20];
    strftime(timeText, sizeof(timeText), "%H:%M:%S", &localTime);
    strftime(dateText, sizeof(dateText), "%a %d %b %Y", &localTime);

    oled.clearBuffer();
    oled.setFont(u8g2_font_logisoso20_tn);
    centerText(timeText, 31);
    oled.setFont(u8g2_font_6x12_tf);
    centerText(dateText, 53);
    oled.sendBuffer();
}
