# HALO CST — Connected Smart Timepiece

HALO CST is an open-source ESP32 clock that combines a 60-pixel WS2812B ring, a 128×64 SH1106 OLED, local controls, and network services. “CST” deliberately means both **Connected Smart Timepiece** and the familiar idea of a time standard.

Current firmware: **1.0.0-rc1**

## Project overview

The LED ring presents time at a glance while the OLED shows exact local time, temporary weather, and service notices. The clock keeps its core interaction local: one physical button and a responsive Web UI on the LAN. Wi-Fi supplies NTP time, current conditions from Open-Meteo, and ArduinoOTA updates.

## Features

- NTP-synchronized local time with a configurable POSIX timezone
- CLASSIC, MINIMAL, and NIGHT display modes
- Scheduled automatic NIGHT mode with manual override
- Four persistent normal brightness levels
- Configurable 60-LED orientation mapping
- WiFiManager captive-portal provisioning
- Local Web UI and JSON status/diagnostics APIs
- Keyless HTTPS current weather with cache, stale state, and backoff
- ArduinoOTA updates with OLED and LED progress
- RAM-only device diagnostics
- Equal 1.9375 MiB OTA application slots on 4 MiB flash

## Hardware requirements

- ESP32-WROOM DevKit or compatible `esp32dev` board with 4 MiB flash
- SH1106 128×64 I²C OLED, normally at address `0x3C`
- WS2812B-compatible 60-pixel ring or strip
- Momentary push button
- Regulated 5 V power source sized for the LED load

## Bill of materials

| Quantity | Part | Notes |
| ---: | --- | --- |
| 1 | ESP32-WROOM DevKit | 4 MiB flash |
| 1 | SH1106 128×64 OLED | I²C, 3.3 V compatible |
| 1 | 60-pixel WS2812B ring/strip | Match the firmware LED count |
| 1 | Normally-open momentary button | Connects GPIO27 to GND |
| 1 | 330–470 Ω resistor | Series resistor at LED data input |
| 1 | 500–1000 µF electrolytic capacitor | Across LED 5 V and GND |
| 1 | 5 V supply and wiring | Current rating appropriate to use |
| Optional | 74AHCT125 level shifter | Recommended for long/noisy data wiring |

## Wiring

| Function | ESP32 pin | Connection |
| --- | --- | --- |
| WS2812B data | GPIO18 | Through series resistor to DIN |
| Button | GPIO27 | Button to GND; firmware uses `INPUT_PULLUP` |
| OLED SDA | GPIO21 | SH1106 SDA |
| OLED SCL | GPIO22 | SH1106 SCL |
| Ground | GND | Common ground for ESP32, OLED, LEDs, and supply |

See [docs/WIRING.md](docs/WIRING.md) for a complete connection guide.

## Power and WS2812 safety

Do not design the power supply around the low average consumption of the clock face. Sixty WS2812B pixels can draw several amperes at unrestricted full white. HALO CST uses conservative brightness, but the supply, wire gauge, connector, and fuse must suit the maximum load you permit.

- Power the LED ring from regulated 5 V; do not draw a large ring through the ESP32 3.3 V regulator.
- Join LED and ESP32 grounds before applying the data signal.
- Add the bulk capacitor near the first pixel and a series data resistor near DIN.
- Never connect or move wiring while powered.
- A 5 V logic buffer is recommended when 3.3 V data is unreliable.

## Build prerequisites

Install [Visual Studio Code](https://code.visualstudio.com/) with the PlatformIO extension, or PlatformIO Core. A data-capable USB cable and the board’s serial driver are required for first installation.

The project pins its Arduino libraries in `platformIO.ini`. PlatformIO downloads them during the first build.

## PlatformIO build

From the repository root:

```sh
pio run
```

The normal environment is `esp32dev`. To compile the OTA environment explicitly:

```sh
pio run -e esp32dev_ota
```

## First USB installation

1. Review `include/Hardware.h`, `include/Config.h`, and `include/Version.h`.
2. Connect the ESP32 over USB.
3. Build and install with `pio run --target upload --upload-port <serial-port>`.
4. Open a 115200-baud serial monitor.
5. Provision Wi-Fi through `HALO-CST-Setup` if no saved network is available.

The setup access point is intentionally open. Complete provisioning near the device on a trusted network.

## WiFiManager setup

On a fresh device, or when stored Wi-Fi cannot connect before the portal timeout, join **HALO-CST-Setup** and follow the captive portal. After connection, the device is available at:

```text
http://halo-cst.local
```

The branding migration does not erase existing ESP32 Wi-Fi credentials. Devices upgrading from older firmware change from `halo-clock.local` to `halo-cst.local` after reboot. The new setup AP name only matters when provisioning is required.

Saved application settings intentionally remain in the legacy Preferences namespace `halo-clock`. This internal storage identifier preserves brightness, display mode, and automatic NIGHT settings; it is not the current product or network name.

## Web UI usage

Open `http://halo-cst.local` from the same LAN. The page displays time, network state, selected and effective display modes, brightness, automatic NIGHT state, weather, OTA state, and diagnostics. It can change mode, brightness, and NIGHT schedule; request a weather refresh; download diagnostics; or request a confirmed reboot.

The page polls only `/api/status` every three seconds. Diagnostics load on demand.

## OTA update workflow

ArduinoOTA starts after Wi-Fi connects and advertises as `halo-cst` on the default ArduinoOTA port, 3232. Build and upload with:

```sh
pio run -e esp32dev_ota --target upload --upload-port halo-cst.local
```

During transfer the OLED and LED ring show progress and normal rendering resumes after reboot or an OTA error. USB/serial upload remains supported.

## Custom partition migration warning

HALO CST uses `partitions_halo_4mb.csv`: NVS at `0x9000`, OTA metadata at `0xE000`, equal app slots at `0x10000` and `0x200000`, and coredump at `0x3F0000`. SPIFFS is intentionally absent.

An ordinary OTA update cannot safely replace a device’s partition table. The first migration from another layout must be installed over USB. Existing NVS and Preferences may survive because the NVS offset is unchanged, but preservation is not guaranteed if flash is erased. Record important settings, install over USB, verify provisioning and settings, then resume OTA updates. See [docs/PARTITION_MIGRATION.md](docs/PARTITION_MIGRATION.md).

## Weather configuration

Open-Meteo provides keyless current conditions over HTTPS. Configure `WEATHER_LATITUDE`, `WEATHER_LONGITUDE`, temperature units, wind units, timeouts, refresh interval, backoff, and stale age in `include/Config.h`.

The checked-in coordinates are a public example near Greenwich, UK—not a contributor’s private location. Replace them with the installation’s WGS84 coordinates. Weather data is kept in RAM only; a failed refresh never clears the last valid cache.

## Button controls

- Short press: cycle normal LED brightness through 10, 25, 40, and 80.
- Long press: cycle CLASSIC → MINIMAL → NIGHT → CLASSIC.
- A long press does not generate an additional short press on release.

The button is active-low on GPIO27 and uses the ESP32 internal pull-up.

## Display modes

- **CLASSIC:** hour ticks, minute progress, hour marker, second marker, and full digital OLED time.
- **MINIMAL:** hour, minute, and second markers with large `HH:MM` on OLED.
- **NIGHT:** fixed low LED brightness, hour/minute markers only, and reduced OLED contrast.

Selected mode and normal brightness are saved. NIGHT’s fixed brightness never overwrites the saved normal brightness.

## Automatic NIGHT

The default schedule is 20:00–06:00 and correctly crosses midnight. During the interval, NIGHT becomes the effective mode without changing the persisted selected mode. A long press creates a manual override until the next schedule boundary. Scheduling waits safely until local time is valid.

Change the defaults in `include/Config.h`; the Web UI can update the stored schedule. The default POSIX timezone currently targets Israel with automatic DST. Public builders should set `LOCAL_TIMEZONE` for their installation.

## Diagnostics

`GET /api/diagnostics` returns uptime, current/minimum heap, reset reason, Wi-Fi state and reconnect count, NTP state and sync age, weather counters/error, OTA state/running partition, firmware version, and build time. Counters are RAM-only and reset at boot. The Web UI can copy or download the same snapshot.

`GET /api/status` remains the lightweight periodically polled endpoint.

## Recovery and factory reset

1. Power-cycle the device and inspect the 115200-baud startup log.
2. Confirm the 5 V supply and shared ground before diagnosing network behavior.
3. If mDNS fails, use the IP address printed on serial or shown by the router.
4. Reinstall the current firmware over USB if OTA is unavailable.
5. As a last resort, erase flash with `pio run --target erase`, then upload over USB and provision again.

Erasing flash removes Wi-Fi credentials and all Preferences. There is no button-triggered factory reset in this release.

## Troubleshooting

| Symptom | Check |
| --- | --- |
| No OLED output | Address `0x3C`, 3.3 V, SDA GPIO21, SCL GPIO22 |
| LED ring dark or erratic | 5 V power, common ground, DIN direction, series resistor, level shifting |
| Clock face rotated/reversed | `LED_ZERO_OFFSET` and `LED_CLOCKWISE` in `include/Config.h` |
| Setup portal absent | Saved Wi-Fi may already be valid; inspect serial output |
| `halo-cst.local` unavailable | Same LAN, mDNS support, client isolation, router device list |
| Time wrong | Internet access, NTP reachability, and `LOCAL_TIMEZONE` |
| Weather unavailable | Coordinates, DNS/TLS access, Open-Meteo availability, cache/error fields |
| OTA target absent | Wi-Fi connected, same LAN/VLAN, port 3232 allowed, hostname resolved |

## Security model

HALO CST is designed for a **trusted local network**. Its Web UI and ArduinoOTA service are unauthenticated in this release. Do not expose the ESP32 directly to the public Internet, place it on an untrusted guest LAN, or forward ports 80 or 3232. Network segmentation is recommended.

Weather requests disclose the configured approximate coordinates to the external Open-Meteo service over HTTPS. No cloud telemetry, analytics, or remote logging is included.

## Resource usage

The validated `esp32dev` release-candidate build uses **54,692 bytes RAM** and **1,166,237 bytes flash**. Each application slot is 2,031,616 bytes, leaving **865,379 bytes** (42.6%) free. PlatformIO prints the exact values for every build; small differences in build metadata can occur between environments.

## Project structure

```text
include/                 Central hardware, behavior, and product metadata
src/                     Firmware services and renderers
test/                    PlatformIO embedded tests
tools/                   API, weather, soak, and release-audit scripts
docs/                    Architecture, wiring, and partition guidance
.github/                 CI, issue forms, and pull-request template
partitions_halo_4mb.csv  Dual-OTA 4 MiB partition layout
platformIO.ini           Build and test environments
```

## Testing summary

The project includes PlatformIO builds for normal and OTA firmware, embedded weather-parser and diagnostics tests, a full Web UI/API suite, weather hardware/API regression tests, and polling soak scripts. Before a release, the checklist also covers USB installation, a real OTA update, existing-device settings/Wi-Fi preservation, and a safe fresh-device setup simulation.

## Roadmap

- Stabilize the 1.0 release candidate and public setup experience
- Improve automated build/test coverage across supported ESP32 variants
- Evaluate authenticated management without sacrificing local-first operation
- Document additional enclosures and face designs from contributors

Weather forecasts, cloud dashboards, telemetry, and additional display modes are intentionally outside this release candidate.

## License and contributing

HALO CST is available under the [MIT License](LICENSE). Contributions are welcome; read [CONTRIBUTING.md](CONTRIBUTING.md), [SECURITY.md](SECURITY.md), the [Code of Conduct](CODE_OF_CONDUCT.md), and the [release checklist](RELEASE_CHECKLIST.md) before participating.
