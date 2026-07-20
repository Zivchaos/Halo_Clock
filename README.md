# HALO CST — Connected Smart Timepiece

HALO CST is an open-source ESP32 clock that combines a 60-pixel WS2812B ring, a 128×64 SH1106 OLED, local controls, and network services. “CST” deliberately means both **Connected Smart Timepiece** and the familiar idea of a time standard.

Current firmware: **1.0.0-rc1**

## Project overview

The LED ring presents time at a glance while the OLED shows exact local time, temporary weather, and service notices. The clock keeps its core interaction local: one physical button and a responsive Web UI on the LAN. Wi-Fi supplies NTP time, current conditions from Open-Meteo, and ArduinoOTA updates.

## Features

- NTP-synchronized local time with a configurable POSIX timezone
- CLASSIC, MINIMAL, NIGHT, and customizable LED-color display modes
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

The dark, responsive product interface takes its visual language from the physical octagonal clock: a cyan perimeter, centered OLED, and restrained instrument-like cards. Desktop uses a persistent section rail; phones put the live clock first and use a compact tab strip. The page polls only `/api/status` every three seconds. Diagnostics and complete network details load on demand.

> **Screenshot placeholders:** desktop overview, 390 x 844 mobile overview, network configuration, and diagnostics captures will be added after browser automation is available. No mock screenshots are presented as live-device evidence.

Editable Web UI sources live in `webui/`. Rebuild the flash-resident page after editing them:

```sh
python tools/build-web-assets.py
```

See [docs/WEB_UI_DESIGN.md](docs/WEB_UI_DESIGN.md) for the responsive layout, accessibility decisions, API usage, and asset-generation rules. Brand SVGs and usage guidance are under `assets/branding/`.

## The Story Behind HALO CST

HALO CST began as an experiment in building a connected LED clock. The first idea was circular, but the practical prototype became octagonal: eight frame pieces of approximately 12.5 cm make a body around 32 x 32 cm, and a one-metre 60 LEDs/m strip follows that perimeter cleanly. The OLED sits at the center with the physical button beneath it. What started as a fabrication compromise became the clock's defining identity.

The software followed the same path from experiment to useful timepiece. A synchronized clock gained display modes, automatic NIGHT, weather, OTA updates, a local Web UI, network configuration, and diagnostics while keeping the physical clock at the center of the experience.

> **Prototype photo placeholder:** add photographs of the octagonal frame and centered OLED/button assembly after the mechanical prototype is ready for publication. The enclosure and centered assembly remain prototype work, not a finalized production design.

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

Open-Meteo provides keyless current conditions over HTTPS. Latitude and longitude can be changed from the Web UI and are saved in the existing Preferences namespace. `WEATHER_LATITUDE` and `WEATHER_LONGITUDE` in `include/Config.h` remain the fresh-install defaults; units, timeouts, refresh interval, backoff, and stale age remain centralized there.

The checked-in coordinates are a public example near Greenwich, UK—not a contributor’s private location. Replace them with the installation’s WGS84 coordinates. Weather data is kept in RAM only; a failed refresh never clears the last valid cache.

## Button controls

- Short press: cycle normal LED brightness through 10, 25, 40, and 80.
- Long press: cycle CLASSIC → MINIMAL → NIGHT → CUSTOM → CLASSIC.
- A long press does not generate an additional short press on release.

The button is active-low on GPIO27 and uses the ESP32 internal pull-up.

## Display modes

- **CLASSIC:** hour ticks, minute progress, hour marker, second marker, and full digital OLED time.
- **MINIMAL:** hour, minute, and second markers with large `HH:MM` on OLED.
- **NIGHT:** fixed low LED brightness, hour/minute markers only, and reduced OLED contrast.
- **CUSTOM:** CLASSIC-style ticks, progress, and markers using six user-selected LED colors from the Web UI.

Selected mode and normal brightness are saved. NIGHT’s fixed brightness never overwrites the saved normal brightness.

The Web UI also exposes persistent ring-orientation calibration. Choose the physical LED representing 12 o'clock, select clockwise or counterclockwise progression, and use the temporary four-color cardinal test before saving and returning to the clock. The same mapping remains shared by every display mode.

## Automatic NIGHT

The default schedule is 20:00–06:00 and correctly crosses midnight. During the interval, NIGHT becomes the effective mode without changing the persisted selected mode. A long press creates a manual override until the next schedule boundary. Scheduling waits safely until local time is valid.

Change the defaults in `include/Config.h`; the Web UI can update the stored schedule. The default POSIX timezone currently targets Israel with automatic DST. Public builders should set `LOCAL_TIMEZONE` for their installation.

## DHCP and static IPv4

DHCP is the default and recommended network mode. The Web UI's **Network configuration** section can store a static IPv4 address, gateway, subnet mask, primary DNS, and optional secondary DNS. Values are accepted only when every required address is valid, the subnet mask is contiguous, the device address is a usable host address, and the gateway is a usable address in the same subnet. Invalid requests leave all saved settings unchanged.

**Save and Apply** sends its response to the browser before scheduling a controlled reboot; it never changes the active interface during the request. Record or reserve the new address first because an incorrect static configuration can make the device unreachable.

Static settings are applied before WiFiManager connects. A static connection is confirmed only after Wi-Fi uses the requested local IP, gateway, and subnet continuously for 15 seconds. NTP, DNS, Internet, weather, and external-provider availability are independent and do not affect this decision. After two genuine failed static boots, HALO CST temporarily uses DHCP without overwriting the saved static configuration. The Web UI and diagnostics distinguish configured mode, effective mode, active addresses, fallback state, and failure count.

For physical recovery, begin holding GPIO27 within 30 seconds after startup completes and keep it held for 10 seconds. HALO CST saves DHCP mode and reboots. This deliberately long, startup-limited gesture makes accidental activation unlikely. If neither the Web UI nor the gesture is available, connect over USB and install known-good firmware. Erase the whole flash only as a last resort because doing so also removes Wi-Fi credentials and all Preferences.

## Diagnostics

`GET /api/diagnostics` returns uptime, current/minimum heap, reset reason, Wi-Fi state and reconnect count, NTP state and sync age, weather counters/error, OTA state/running partition, firmware version, and build time. Counters are RAM-only and reset at boot. The Web UI can copy or download the same snapshot.

`GET /api/status` remains the lightweight periodically polled endpoint.

## Recovery and factory reset

1. Power-cycle the device and inspect the 115200-baud startup log.
2. Confirm the 5 V supply and shared ground before diagnosing network behavior.
3. If mDNS fails, use the IP address printed on serial or shown by the router.
4. Reinstall the current firmware over USB if OTA is unavailable.
5. To recover from an unreachable static configuration, use the GPIO27 gesture documented above or reinstall known-good firmware over USB.
6. As a last resort, erase flash with `pio run --target erase`, then upload over USB and provision again.

Erasing flash removes Wi-Fi credentials and all Preferences. The GPIO27 gesture resets only network mode to DHCP; it is not a full factory reset.

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

The validated `esp32dev` release-candidate build uses **54,740 bytes RAM** and **1,181,813 bytes flash**. Each application slot is 2,031,616 bytes, leaving **849,803 bytes** (41.8%) free. PlatformIO prints the exact values for every build; small differences in build metadata can occur between environments.

## Project structure

```text
include/                 Central hardware, behavior, and product metadata
src/                     Firmware services and renderers
webui/                   Editable HTML, CSS, and JavaScript source
assets/branding/          HALO CST vector marks and style guide
test/                    PlatformIO embedded tests
tools/                   API, weather, soak, and release-audit scripts
docs/                    Architecture, wiring, partitions, and UI design
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
