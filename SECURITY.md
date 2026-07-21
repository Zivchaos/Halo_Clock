# Security policy

## Supported versions

Security fixes target the latest stable release. Older milestone tags are retained for history but are not maintained.

## Security model

HALO CST is a local-first maker project intended only for a trusted LAN. Firmware 1.0.0 has:

- an unauthenticated HTTP Web UI and control API on port 80;
- unauthenticated ArduinoOTA on port 3232;
- an open WiFiManager setup access point while provisioning;
- outbound HTTPS requests to Open-Meteo using configured approximate coordinates.

Do not expose the device to the Internet, forward its ports, or place it on an untrusted network. Use firewall/VLAN isolation where appropriate. Anyone with network access may read status, change settings, reboot the device, or upload compatible firmware over OTA.

## Reporting a vulnerability

Please use the repository’s private GitHub Security Advisory reporting feature rather than a public issue. Include the affected firmware version, reproduction steps, impact, and any suggested mitigation. Do not include real Wi-Fi credentials, device addresses, or other private data.

Allow maintainers reasonable time to investigate and coordinate a fix before public disclosure. This community project does not offer a formal service-level agreement or bug-bounty program.
