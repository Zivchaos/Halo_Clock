# Security policy

## Supported versions

Security fixes target the latest stable release. Older milestone tags are retained for history but are not maintained.

## Security model

HALO CST is a local-first maker project intended only for a trusted LAN. Firmware 1.0.0 has:

- an HTTP Web UI and control API on port 80, with same-origin request checks for state-changing operations;
- ArduinoOTA on port 3232, disabled by default and protected by an administrator password when enabled for the current session;
- an open WiFiManager setup access point while provisioning;
- outbound HTTPS requests to Open-Meteo using configured approximate coordinates.

Do not expose the device to the Internet, forward its ports, or place it on an untrusted network. Use firewall/VLAN isolation where appropriate. Anyone with local-network access may read status; physical or network access to a trusted browser session can still change settings and reboot the device. Only an operator who knows the configured OTA administrator password can enable OTA and upload compatible firmware during that session.

## Reporting a vulnerability

Please use the repository’s private GitHub Security Advisory reporting feature rather than a public issue. Include the affected firmware version, reproduction steps, impact, and any suggested mitigation. Do not include real Wi-Fi credentials, device addresses, or other private data.

Allow maintainers reasonable time to investigate and coordinate a fix before public disclosure. This community project does not offer a formal service-level agreement or bug-bounty program.
