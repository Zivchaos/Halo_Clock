# Changelog

Notable project milestones are recorded here. HALO CST follows semantic versioning for stable public releases.

## [1.0.0-rc1] — 2026-07-15

- Renamed the public product to HALO CST — Connected Smart Timepiece.
- Centralized product identity, version, hostname, and setup AP metadata.
- Changed the local/OTA hostname to `halo-cst` and setup AP to `HALO-CST-Setup`.
- Replaced installation-specific weather coordinates with documented public example values.
- Added public build, wiring, security, contribution, migration, and release documentation.
- Preserved the legacy `halo-clock` Preferences namespace for upgrade compatibility.

## [0.6.0-diagnostics] — 2026-07-15

- Added RAM-only health counters, `/api/diagnostics`, and Web UI copy/download actions.

## [0.5.1-expanded-ota] — 2026-07-15

- Replaced unused SPIFFS with two equal 2,031,616-byte OTA slots while retaining NVS, OTA metadata, and coredump.

## [0.5.0-weather] — 2026-07-15

- Added keyless Open-Meteo current conditions, validated cache/stale behavior, Web UI weather, and a temporary OLED view.

## [0.4.0-web-ui] — 2026-07-14

- Added a responsive trusted-LAN dashboard and validated local control/status API.

## [0.3.0-ota] — 2026-07-14

- Added ArduinoOTA with non-blocking OLED and 60-LED progress feedback.

## [0.2.0-display-modes] — 2026-07-14

- Added persistent CLASSIC, MINIMAL, and NIGHT modes with correct normal-brightness restoration.

## [0.1.0-working-clock] — 2026-07-13

- Established the synchronized OLED/LED working clock baseline on ESP32-WROOM, SH1106, and 60 WS2812B pixels.
