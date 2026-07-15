# HALO CST firmware architecture

HALO CST uses small static services so Arduino `setup()` and `loop()` stay simple and normal interaction remains non-blocking.

```text
Halo
├── SettingsService (ESP32 Preferences)
├── Button (active-low debounce and press classification)
├── TimeService (WiFiManager, NTP, local timezone)
├── AutoNightService (selected/effective mode scheduling)
├── Clock
│   ├── LedRing (logical-to-physical 60-pixel renderer)
│   └── Oled (clock, weather, notices)
├── WeatherService + WeatherProvider (scheduled HTTPS task)
├── OtaService (ArduinoOTA and progress state)
├── DiagnosticsService (RAM-only counters and snapshot)
└── WebService (embedded page and local JSON API)
```

## Main update loop

The normal loop has no intentional delays. Button debounce, clock redraw, scheduled NIGHT, notices, weather scheduling, Web requests, and reboot confirmation use `millis()` or event state. `ArduinoOTA.handle()` is called continuously after Wi-Fi becomes available. The only deliberate visual delay is the startup LED sweep before runtime services begin.

Weather HTTPS work runs in one FreeRTOS task at a time. The service prevents overlapping requests, schedules retries, and commits parsed data only after a complete valid response. This keeps the clock, button, Web server, and OTA service responsive.

## State boundaries

- **Selected mode** is the user’s persisted CLASSIC, MINIMAL, or NIGHT choice.
- **Effective mode** is what renders after automatic NIGHT scheduling and manual override are applied.
- **Normal brightness** is persisted; NIGHT uses a fixed low output without overwriting it.
- **Weather cache** and **diagnostic counters** live only in RAM.
- Wi-Fi credentials remain managed by the ESP32 Wi-Fi stack/WiFiManager.

## LED rendering

All clock elements use one logical-to-physical mapping based on `LED_ZERO_OFFSET` and `LED_CLOCKWISE`. CLASSIC renders hour ticks, minute progress, hour marker, and second marker. MINIMAL renders hour/minute/second markers. NIGHT renders only hour and minute at low fixed brightness. Weather never changes the LED clock.

During OTA, normal ring rendering pauses and a progress indicator owns the LEDs. Normal rendering resumes after an OTA failure; a successful OTA reboots normally.

## Local interfaces

- `GET /` — embedded HALO CST Web UI
- `GET /api/status` — lightweight three-second polling snapshot
- `GET /api/diagnostics` — complete on-demand diagnostics snapshot
- `POST /api/mode` — validated selected display mode
- `POST /api/brightness` — validated supported brightness
- `POST /api/auto-night` — validated schedule and enable state
- `POST /api/weather/refresh` — server-rate-limited refresh request
- `POST /api/reboot` — confirmed, deferred reboot request

## Upgrade compatibility

Persistent settings intentionally remain in the ESP32 Preferences namespace `halo-clock`. It is a legacy storage identifier, not current product branding. Changing it would silently default brightness, display mode, and automatic NIGHT settings on existing devices.

The network hostname changes to `halo-cst`, while stored Wi-Fi credentials remain untouched. The custom partition layout retains the standard NVS offset at `0x9000` and OTA metadata at `0xE000`.
