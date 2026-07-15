# Network configuration and recovery

HALO CST uses DHCP by default and continues to use WiFiManager for SSID and password
provisioning. IP configuration is an independent setting stored in the existing
`halo-clock` Preferences namespace.

## Static IPv4 validation

The Web UI and API require a confirmed save before accepting changes. Static
mode requires a strict dotted-quad IP address, gateway, contiguous subnet mask,
primary DNS, and optionally secondary DNS. The device rejects malformed
addresses, `/0` and `/32` masks, network and broadcast addresses, unusable
gateway addresses, and gateways outside the selected subnet. A rejected request
does not write Preferences or change the active interface.

`POST /api/network` responds before scheduling a controlled reboot. Static
settings are applied before WiFiManager begins connecting after that reboot.
`POST /api/network/reset` similarly requires explicit confirmation and restores
DHCP after its response has been sent.

## Automatic fallback

The configured mode is the saved user choice. The effective mode is what the
current boot uses. A static boot must associate with Wi-Fi using the requested
local IP, gateway, and subnet, then remain continuously stable for 15 seconds.
NTP, DNS, Internet access, weather, and external providers are deliberately not
part of static-link validation. A failed association, mismatched active
configuration, or repeated pre-confirmation drops increments a persistent
failure counter and reboots. After two failed static boots, the next boot
temporarily uses DHCP while retaining the saved static settings for diagnosis
and retry. The Web UI and `/api/diagnostics` expose the configured/effective
distinction, fallback flag, and failure count.

## Recovery order

1. If the dashboard is reachable through DHCP fallback, correct and save the
   static values or select DHCP.
2. To force DHCP physically, begin holding GPIO27 within 30 seconds after the
   HALO CST finishes its startup sequence, continue for 10 seconds, and wait for
   the automatic reboot.
3. If the device remains unreachable, connect it over USB and install known-good
   firmware. Keep the existing flash contents when possible.
4. Erase the full flash only as a last resort. Erasure removes Wi-FiManager
   credentials and every Preferences setting, so the setup portal and all user
   preferences must be configured again.

Static addressing does not replace WiFiManager: the SSID and password remain
managed by WiFiManager, and its captive portal is still used for provisioning.
