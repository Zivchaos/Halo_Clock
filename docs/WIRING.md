# HALO CST wiring guide

## Pin assignment

```text
ESP32 GPIO18 --- 330–470 Ω --- WS2812B DIN
ESP32 GPIO21 ---------------- SH1106 SDA
ESP32 GPIO22 ---------------- SH1106 SCL
ESP32 GPIO27 --- button ----- GND

regulated 5 V --------------- WS2812B 5 V
ESP32 3V3 ------------------- SH1106 VCC
common GND ------------------ ESP32, WS2812B, SH1106, supply
```

GPIO27 is configured as `INPUT_PULLUP`, so the button is active-low and needs no external pull-up.

## LED power protection

Place a 500–1000 µF electrolytic capacitor across 5 V and GND near the first pixel. Observe polarity. Place a 330–470 Ω resistor in series with the data line near DIN.

Sixty unrestricted WS2812B pixels can draw several amperes at full white. Size the regulated 5 V supply, wire gauge, connector, and fuse for the maximum output your installation permits. Do not power a large ring from the ESP32 3.3 V regulator. Always use a common ground.

## Logic level

Short 3.3 V data wiring often works with WS2812B ECO pixels. For long, noisy, or unreliable wiring, use a 74AHCT125/74HCT245 powered at 5 V. Avoid slow bidirectional I²C level-shifter modules for LED data.

## Orientation calibration

`LED_ZERO_OFFSET` selects the physical pixel representing logical 12 o’clock. `LED_CLOCKWISE` controls direction. Set `ENABLE_RING_CALIBRATION` temporarily to show four colored cardinal positions, install over USB, determine the correct values, then disable calibration and rebuild.

All hour ticks, minute progress, hour markers, and second markers share the same mapping.
