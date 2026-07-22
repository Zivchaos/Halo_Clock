#include "Oled.h"

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "Config.h"
#include "Hardware.h"
#include "Version.h"

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
    centerText(Product::NAME, 28);
    oled.setFont(u8g2_font_5x7_tf);
    centerText(Product::EXPANDED_NAME, 48);
    oled.sendBuffer();
}

void Oled::status(const char* line1, const char* line2)
{
    oled.clearBuffer();
    oled.setContrast(255);
    oled.setFont(u8g2_font_6x12_tf);
    oled.drawStr(0, 20, line1);
    oled.drawStr(0, 40, line2);
    oled.sendBuffer();
}

void Oled::time(const tm& localTime, DisplayMode mode)
{
    char shortTimeText[6];
    char timeText[9];
    char dateText[20];
    strftime(shortTimeText, sizeof(shortTimeText), "%H:%M", &localTime);
    strftime(timeText, sizeof(timeText), "%H:%M:%S", &localTime);
    strftime(dateText, sizeof(dateText), "%a %d %b %Y", &localTime);

    oled.clearBuffer();
    oled.setContrast(mode == DisplayMode::NIGHT ? Config::NIGHT_OLED_CONTRAST : 255);

    if (mode == DisplayMode::CLASSIC || mode == DisplayMode::CUSTOM)
    {
        oled.setFont(u8g2_font_logisoso20_tn);
        centerText(timeText, 31);
        oled.setFont(u8g2_font_6x12_tf);
        centerText(dateText, 53);
    }
    else if (mode == DisplayMode::MINIMAL)
    {
        oled.setFont(u8g2_font_logisoso32_tn);
        centerText(shortTimeText, 45);
    }
    else
    {
        oled.setFont(u8g2_font_logisoso20_tn);
        centerText(shortTimeText, 38);
    }

    oled.sendBuffer();
}

void Oled::displayMode(DisplayMode mode)
{
    oled.clearBuffer();
    oled.setContrast(mode == DisplayMode::NIGHT ? Config::NIGHT_OLED_CONTRAST : 255);
    oled.setFont(u8g2_font_6x12_tf);
    centerText("DISPLAY MODE", 23);
    oled.setFont(u8g2_font_logisoso20_tf);
    centerText(DisplayModes::name(mode), 51);
    oled.sendBuffer();
}

void Oled::notice(const char* message, DisplayMode mode)
{
    oled.clearBuffer();
    oled.setContrast(mode == DisplayMode::NIGHT ? Config::NIGHT_OLED_CONTRAST : 255);
    oled.setFont(u8g2_font_7x14B_tf);
    centerText(message, 38);
    oled.sendBuffer();
}

void Oled::weather(const WeatherData& weather, DisplayMode mode)
{
    oled.clearBuffer();
    oled.setContrast(mode == DisplayMode::NIGHT ? Config::NIGHT_OLED_CONTRAST : 255);

    if (!weather.valid)
    {
        oled.setFont(u8g2_font_6x12_tf);
        centerText("WEATHER", 24);
        centerText("UNAVAILABLE", 45);
        oled.sendBuffer();
        return;
    }

    char temperature[16];
    char details[28];
    snprintf(
        temperature,
        sizeof(temperature),
        "%.1f C",
        static_cast<double>(weather.temperature));
    snprintf(
        details,
        sizeof(details),
        "H:%u%%  W:%.1f",
        weather.humidity,
        static_cast<double>(weather.windSpeed));

    oled.setFont(u8g2_font_6x12_tf);
    const char* heading = weather.stale
        ? "WEATHER - STALE"
        : (weather.error[0] != '\0' ? "WEATHER - CACHED" : "WEATHER");
    centerText(heading, 11);
    oled.setFont(u8g2_font_logisoso20_tf);
    centerText(temperature, 36);
    oled.setFont(u8g2_font_6x12_tf);
    centerText(weather.condition, 49);
    centerText(details, 62);
    oled.sendBuffer();
}

void Oled::redAlert(const char* area, bool test)
{
    oled.clearBuffer();
    oled.setContrast(255);
    oled.setFont(u8g2_font_7x14B_tf);
    centerText(test ? "TEST ALERT" : "RED ALERT", 22);
    oled.setFont(u8g2_font_6x12_tf);
    centerText(test ? "SIMULATION ONLY" : "FOLLOW OFFICIAL", 40);
    centerText(test ? "" : "INSTRUCTIONS", 52);
    if (area != nullptr && area[0] != '\0') centerText(area, 63);
    oled.sendBuffer();
}
