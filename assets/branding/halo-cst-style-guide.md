# HALO CST brand guide

HALO CST is a Connected Smart Timepiece whose visual identity comes from the physical octagonal prototype: an eight-sided illuminated perimeter, centered OLED, and button directly beneath it. The system should feel precise, calm, technical, and deliberately local—not like a generic router dashboard.

## Marks

- `halo-cst-logo-primary.svg` is the preferred horizontal, dark-background lockup. The final **O** is an octagon and CST reads as a model designation.
- `halo-cst-logo-stacked.svg` is the light-background lockup for README pages, documents, and release layouts.
- `halo-cst-icon-octagon.svg` is the compact product mark. Its OLED rectangle and button dot must remain visible.
- `halo-cst-favicon.svg` is simplified for small browser sizes.
- `halo-cst-web-header.svg` is a compact horizontal header lockup.
- `halo-cst-social-card.svg` is repository/release artwork and is never embedded in firmware.

The SVGs use system-safe font fallbacks and contain no remote font or image dependency.

## Palette

| Role | Value | Usage |
| --- | --- | --- |
| Near black | `#0E0E0E` | Product body, deep background |
| Charcoal | `#1A1A1A` | Secondary surfaces |
| Panel dark | `#101820` | Web UI cards |
| Off-white | `#F2F2F2` | Primary copy and light marks |
| Cyan | `#22D3EE` | Perimeter, focus, active state |
| Light cyan | `#7DD3FC` | Supporting labels |
| Green | `#22C55E` | Healthy/success state with text |
| Amber | `#F59E0B` | Warning state with text |
| Red | `#EF4444` | Error/destructive state with text |

Status colors must always be accompanied by a word or icon; color alone is not enough.

## Clear space and minimum size

Keep clear space around every mark equal to at least one quarter of the icon's width. Do not place text, trim, or another strong edge inside that area.

- Primary/stacked marks: minimum 180 px wide on screen or 38 mm in print.
- Compact icon: minimum 24 px. Use the favicon below 32 px.
- Tagline: omit it when it would render below 10 px rather than allowing it to blur.

## Background use

The primary, Web header, icon, favicon, and social card are dark-background-safe. The stacked lockup is the monochrome light-background option. When adapting a mark, keep the near-black/off-white relationship and use cyan only as the ambient accent.

## Typography

Use a condensed system sans-serif stack for brand headings (`Arial Narrow`, `Roboto Condensed` when locally available, then `Arial`). Use the platform UI sans-serif stack for controls and body copy. Use a system monospace stack for time, addresses, and diagnostic values. No runtime font download is permitted.

## Do not

- Stretch, skew, rotate, round, or redraw the octagon as a circle.
- Move the OLED or button away from the central vertical axis in the icon.
- Add gradients to the wordmark itself or use multiple competing accent colors.
- Place the cyan mark on a background that reduces its contrast.
- use the icon as a decorative pattern; octagons should communicate product structure.
- add shadows or glow strong enough to soften the engineered edges.
