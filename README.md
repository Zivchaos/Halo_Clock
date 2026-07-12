# HALO Clock

ESP32 clock using a 128x64 SH1106 OLED and a 60-pixel WS2812B ring.

## Current hardware baseline

- ESP32-WROOM DevKit
- SH1106 OLED: SDA GPIO21, SCL GPIO22, I2C address 0x3C
- WS2812B ECO strip: 60 LEDs, data GPIO18

## Clock behavior

At boot, the existing OLED splash/status sequence and blue LED sweep run. WiFiManager
then reconnects using saved credentials or starts the `HALO Clock Setup` captive portal.
Once NTP is available, the OLED shows Israel local time and date. The LED strip shows
12 dim hour ticks, elapsed-minute progress, a three-pixel hour marker, and seconds.

Build with `pio run` from a PlatformIO terminal.
