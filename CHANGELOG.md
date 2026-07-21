# Changelog

Notable project milestones are recorded here. HALO CST follows semantic versioning for stable public releases.

## [1.0.0] — 2026-07-20

HALO CST 1.0.0 is the first public release of the Connected Smart Timepiece: a local-first ESP32 clock with a custom LED display engine, responsive embedded WebUI, persistent settings, weather, diagnostics, and OTA support.

### New features

- Added a responsive, device-hosted WebUI with a live overview, display controls, network configuration, weather, diagnostics, and trusted-LAN safety guidance.
- Added runtime LED-ring calibration: configurable physical 12 o’clock offset, clockwise/counterclockwise direction, a four-color cardinal test, validation, persistence, and WebUI/API controls.
- Added persisted weather-location configuration with validated WGS84 coordinates and Open-Meteo refresh integration.
- Added the CUSTOM display mode with six persisted LED color controls.

### Improvements

- Established the HALO CST visual identity with octagonal marks, favicon, social card, brand guide, and a WebUI that reflects the physical prototype.
- Reworked the README as a product-facing guide with real, privacy-redacted WebUI captures, quick start, hardware overview, configuration, testing, and security guidance.
- Improved API validation and persisted settings handling for calibration, custom colors, and weather location.
- Added architecture and WebUI-design documentation, while preserving existing wiring, network-recovery, partition-migration, contribution, and security references.

### Testing and release engineering

- Added WebUI source checks and public-release metadata checks.
- Validated the release with 39 WebUI source checks, 46 public-release checks, and 38 embedded test cases across weather, diagnostics, and network configuration.
- Confirmed a successful ESP32 firmware build using the dual-OTA 4 MiB partition layout.

### Repository consolidation

- Consolidated the branded WebUI, calibration, weather-location, and CUSTOM-mode work through the integration branch.
- Removed superseded WebUI, calibration, and branding feature branches after merge, while preserving all milestone tags.
- Removed the unused `faceplates/arabic.svg` asset and kept release imagery free of private network addresses and personal coordinates.

### Known limitations

- HALO CST remains a trusted-LAN device: the WebUI and ArduinoOTA service are unauthenticated and must not be exposed to the public Internet.
- Hardware enclosure photographs are not yet part of this release; the documented enclosure remains a maker prototype.
- The supplied WebUI captures are representative desktop views; mobile and physical-prototype imagery can be added in a later release.

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
