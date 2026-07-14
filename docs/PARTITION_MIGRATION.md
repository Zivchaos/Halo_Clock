# 4 MiB dual-OTA partition migration

HALO CST removes unused SPIFFS and assigns the available 4 MiB flash to NVS, OTA metadata, two equal application slots, and coredump:

| Name | Offset | Size | Bytes |
| --- | ---: | ---: | ---: |
| nvs | `0x9000` | `0x5000` | 20,480 |
| otadata | `0xE000` | `0x2000` | 8,192 |
| app0 | `0x10000` | `0x1F0000` | 2,031,616 |
| app1 | `0x200000` | `0x1F0000` | 2,031,616 |
| coredump | `0x3F0000` | `0x10000` | 65,536 |

The table ends at `0x400000`, exactly 4 MiB. There is no filesystem partition.

## Why USB is required once

The partition table is stored outside an application image. A normal OTA transfer updates only the inactive application slot, so it cannot safely replace the device’s flash geometry. The first migration from a different layout must be installed over USB so the partition table and firmware are written together.

## Safe procedure

1. Record brightness, selected mode, automatic NIGHT schedule, and Wi-Fi information needed for recovery.
2. Build with `pio run` and confirm `partitions_halo_4mb.csv` is selected.
3. Install once over USB without erasing flash.
4. Confirm the device boots and reconnects to Wi-Fi.
5. Verify saved brightness, mode, and schedule.
6. Confirm time, OLED, LED ring, button, Web UI, weather, and automatic NIGHT.
7. Perform one ArduinoOTA upload using the new layout.

NVS remains at `0x9000`, so existing Wi-Fi credentials and Preferences may survive a layout-only USB migration. This is practical compatibility, not a guarantee. If boot or NVS data is corrupt, erase flash as a recovery step, reinstall over USB, and provision again. A flash erase always removes credentials and settings.
