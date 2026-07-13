# Firmware architecture

HALO uses small static services to keep Arduino startup and loop code minimal.

```text
Halo
├── Settings (Preferences)
├── Button
├── Notifications
├── Clock
│   ├── TimeService (WiFiManager + NTP)
│   ├── LedRing (layered 60-pixel renderer)
│   └── Oled (page renderer)
├── Weather (Open-Meteo)
└── WebPortal (dashboard + API + OTA)
```

## Main loop

The normal loop contains no intentional delays. Each service uses `millis()` scheduling or event-driven work. The only visual delays are in the startup LED self-test. Weather retrieval is a short HTTPS transaction every 30 minutes.

## Ring rendering priorities

Each logical pixel has an RGB color and priority. Layers are composed in this order:

1. Hour ticks
2. Minute bar or minute hand
3. Hour marker and side pixels
4. Second trail and second marker

Logical indices are transformed by the saved clockwise/direction settings before output. The completed frame is current-limited, gamma-corrected, and sent to the strip.

## Settings API

- `GET /api/status` — live time, network, memory, and weather state
- `GET /api/settings` — persistent device configuration
- `POST /api/settings` — update configuration JSON
- `POST /api/weather/refresh` — request a weather refresh
- `POST /api/reboot` — restart the ESP32
- `POST /api/wifi/reset` — clear Wi-Fi credentials and open setup
- `GET/POST /update` — browser firmware update

## Persistent compatibility

Settings use individual keys in the ESP32 `Preferences` namespace `halo-clock`. New fields can be added with defaults without invalidating older installations.
