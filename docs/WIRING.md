# HALO wiring guide

## Pin assignment

```text
ESP32 GPIO18 --- 330 Ω --- WS2812B DIN
ESP32 GPIO21 ------------ SH1106 SDA
ESP32 GPIO22 ------------ SH1106 SCL
ESP32 GPIO27 --- button --- GND

ESP32 5V ---------------- WS2812B 5V
ESP32 3V3 --------------- SH1106 VCC
ESP32 GND --------------- WS2812B GND and SH1106 GND
```

## LED power protection

Place a 500–1000 µF electrolytic capacitor near the strip input:

```text
5V  ---- capacitor +
GND ---- capacitor -
```

Observe capacitor polarity. The 330 Ω resistor belongs in series with the data line near the first pixel.

## Logic level

Short 3.3V data wiring commonly works with WS2812B ECO pixels. For longer or unreliable wiring, use a 74AHCT125 or 74HCT245 powered at 5V. Do not use a slow BSS138 I²C level-shifter module for the pixel data signal.

## Power

The default firmware limits estimated LED current to 650 mA and uses conservative brightness. Use a regulated 5V USB supply rated for at least 1A. For higher custom brightness, use a larger supply and inject 5V/GND directly at the strip while maintaining common ground with the ESP32.

## Orientation

After assembly, open the dashboard and adjust:

1. **12 o'clock LED offset** until the top physical LED is the 12 o'clock tick.
2. Disable **Clockwise strip** if time advances in the wrong direction.

No firmware change is required.
