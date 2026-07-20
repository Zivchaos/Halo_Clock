# HALO CST Web UI design

## Design goals

The interface should feel like the product itself: a precise octagonal timepiece with a small centered OLED and a quiet cyan perimeter glow. It is deliberately not styled like a router administration page. The visual hierarchy starts with time, then presents the current operating state, everyday controls, and finally advanced configuration.

The redesign changes presentation only. Device APIs, GPIO assignments, stored settings, automatic NIGHT behavior, OTA, weather, and the physical-button interaction remain firmware-owned.

## Brand language

- The octagon is the primary silhouette and represents the eight-piece physical frame.
- A centered dark rectangle recalls the 128 x 64 OLED.
- A small dot beneath the rectangle represents the physical GPIO27 button.
- Cyan is used for presence, selection, and ambient light—not as decoration on every element.
- Monochrome surfaces, restrained borders, compact labels, and numeric readouts create a technical but approachable character.

Full logo usage rules are in [`assets/branding/halo-cst-style-guide.md`](../assets/branding/halo-cst-style-guide.md).

## Palette

| Role | Value | Use |
| --- | --- | --- |
| Near black | `#0E0E0E` | Page background |
| Charcoal | `#1A1A1A` | Elevated surfaces |
| Panel dark | `#101820` | Cards and OLED surround |
| Off-white | `#F2F2F2` | Primary text |
| Cyan | `#22D3EE` | Selected and connected state |
| Light cyan | `#7DD3FC` | Secondary highlight |
| Green | `#22C55E` | Healthy/success state |
| Amber | `#F59E0B` | Warning/stale state |
| Red | `#EF4444` | Error/destructive state |

Status always includes text or a symbol in addition to color.

## Typography

The page uses a compact system-font stack so the device serves no font files and makes no third-party requests. Headings use narrow tracking and uppercase supporting labels to suggest an engineered instrument. Time and diagnostic values use the browser's system monospace stack for stable numeric alignment.

## Responsive layout

- `>= 980px`: persistent left navigation and a multi-column overview.
- `680–979px`: compact header navigation and two-column cards where space permits.
- `< 680px`: clock-first single column with horizontally scrollable section tabs.
- The validation viewport is 390 x 844. Interactive targets are at least 44 pixels high, cards cannot exceed the viewport, and the document must not scroll horizontally.

The mobile navigation is a simple tab strip rather than a JavaScript drawer. Section navigation remains usable with a keyboard and with JavaScript unavailable.

## Page hierarchy

1. **Overview:** live octagonal clock, selected/effective mode, brightness, weather, automatic NIGHT, network, diagnostics, OTA, and quick controls.
2. **Time & Display:** mode, brightness, and automatic NIGHT schedule.
3. **Network:** configured/effective mode, active addresses, DHCP/static fields, fallback state, failure count, apply warning, and recovery guidance.
4. **Weather:** cached current conditions and manual refresh.
5. **Diagnostics:** on-demand device snapshot with copy and JSON download actions.
6. **Settings:** firmware/build information, trusted-LAN warning, project link, and confirmed reboot.

## API dependencies

The browser regularly requests only `GET /api/status`, approximately once every three seconds. The main HTML is loaded once. Diagnostics and network details are fetched on demand.

| UI operation | Endpoint |
| --- | --- |
| Lightweight state | `GET /api/status` |
| Diagnostic snapshot | `GET /api/diagnostics` |
| Network details | `GET /api/network` |
| Change display mode | `POST /api/mode` |
| Change brightness | `POST /api/brightness` |
| Change NIGHT settings | `POST /api/auto-night` |
| Load calibration, location, and colors | `GET /api/customization` |
| Preview/save ring orientation | `POST /api/calibration` |
| Save CUSTOM LED colors | `POST /api/custom-colors` |
| Save weather coordinates | `POST /api/weather/location` |
| Save/apply network | `POST /api/network` |
| Reset network mode | `POST /api/network/reset` |
| Refresh weather | `POST /api/weather/refresh` |
| Confirmed reboot | `POST /api/reboot` |

Failed polls preserve the last known values and visibly mark the interface stale. Controls are disabled while their request is active. Backend validation remains authoritative.

## Asset generation

Editable sources live in `webui/index.html`, `webui/styles.css`, and `webui/app.js`. Generate the flash-resident header with:

```sh
python tools/build-web-assets.py
```

The standard-library-only script validates all inputs, safely removes unnecessary whitespace, embeds CSS and JavaScript into the HTML, embeds the compact favicon SVG, and writes `src/WebUiAssets.h`. It prints input/output sizes and a content hash. Running it twice with unchanged sources produces identical bytes. Generated output should be refreshed whenever a file in `webui/` or the favicon changes.

## Accessibility

- Semantic sections, headings, navigation, forms, labels, buttons, and status regions are used.
- Icon-only brand art is decorative; controls always have visible text or an accessible name.
- Focus indicators use a high-contrast cyan outline.
- Live notices use a polite status region.
- Destructive reboot and network apply/reset operations require explicit confirmation.
- Motion is limited to short state transitions and disabled through `prefers-reduced-motion`.
- Healthy, warning, error, and disconnected states are expressed with words as well as color.

## Performance constraints

- No external fonts, libraries, images, or runtime UI dependencies.
- No canvas and no continuous animation loop.
- The octagon is pure CSS geometry.
- The page is a single embedded response served from program flash.
- Diagnostics and network payloads are loaded only when useful.
- A single status timer prevents polling duplication and request storms.
- The interface does not duplicate scheduling, persistence, network validation, or display logic from firmware.

## Physical prototype influence

The original circular LED-clock idea became an octagon because eight approximately 12.5 cm frame pieces form a practical roughly 32 x 32 cm body and fit a one-metre, 60 LEDs/m strip around the perimeter. That fabrication choice became the product identity. The Web UI's octagonal clock, OLED-like center, button dot, cyan edge light, and card geometry translate those physical cues without pretending to be a literal simulation.

The centered OLED/button assembly and enclosure remain prototype work; the interface does not claim a finalized mechanical design.
