# HALO product definition

## Promise

Time should be beautiful, readable, and calm. The LED ring tells time; the OLED provides context.

## Design principles

1. Time remains readable during every animation.
2. Normal operation does not require a serial monitor or recompilation.
3. Physical faceplate style remains independent of firmware.
4. Network loss does not stop the local clock once time has synchronized.
5. Features must be configurable, recoverable, and observable.

## OLED behavior

The OLED rotates through clock, weather, network, and system pages every 12 seconds. A button press advances immediately. Notifications temporarily take over the display and return to the previous page automatically.

## LED modes

- **Progress** — elapsed minute arc, wide hour marker, second trail
- **Analog** — independent hour, minute, and second hands
- **Minimal** — hands only, no ticks
- **Ambient** — calm hour glow only
- **Rainbow** — animated rainbow minute progress

## Themes

Classic, Nordic, Fire, Matrix, Cyberpunk, and Sunset are included. Theme definitions are centralized in `src/Themes.cpp`.
