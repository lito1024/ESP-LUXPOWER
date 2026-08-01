# LuxPowerTCP

## Project purpose
This project implements a read-only LuxPower TCP client for ESP32-based firmware using PlatformIO and the Arduino framework. It is designed to read inverter data over a persistent TCP connection while keeping the application layer responsible for Wi-Fi management. The protocol library also exposes a framework-neutral transport interface for ESPHome ESP-IDF wrapper use.

## Current architecture
- The transport and protocol logic live in the LuxPowerTCP library under lib/LuxPowerTCP.
- The firmware entry point is in src/main.cpp.
- The library maintains persistent TCP connectivity, caches input and holding registers, and exposes both low-level register access and higher-level cache-based helpers.
- The verified register database now lives in lib/LuxPowerTCP/src/LuxPowerRegisters.h and is documented in docs/Registers.md.

## Implemented stages (1-12C)
1. Initial working sketch preservation and refactor into a reusable library.
2. Protocol parsing and CRC validation fixes.
3. Internal cache and bulk input refresh support.
4. Polling and automatic cache refresh scheduling.
5. Persistent TCP connection handling and timeout-tolerant behavior.
6. Holding-register cache and automatic polling.
7. Explicit diagnostics for scheduled holding-cache refresh origin.
8. Cache-only high-level read-only API for verified metrics.
9. Cache-only metric registry with verified descriptors.
10. Verified register database foundation with only accepted mappings that are already verified in this project.
11. Developer register scanner for cache-only register discovery support.
12A. Research-only upstream metric candidate extraction from luxpower-ha-integration.
12B. Temporary cache-only real-inverter comparison against Home Assistant.
12C. Promotion of five verified input-register metrics to the official register database and public cache-only API.

## Supported hardware
- ESP32-S3 development boards targeting the esp32s3 environment in PlatformIO.
- The current test application is written for an ESP32-S3 board with Wi-Fi connectivity and access to the LuxPower inverter over TCP port 8000.

## Initial setup
Before uploading the test firmware, create a local secrets file:

```bash
cp src/local_secrets.example.h src/local_secrets.h
```

Then edit `src/local_secrets.h` and fill in:
- Wi-Fi SSID;
- Wi-Fi password;
- LuxPower dongle IP address;
- LuxPower TCP port;
- dongle serial;
- inverter serial.

`src/local_secrets.h` is ignored by Git. If it is missing, the firmware still compiles with placeholder values and prints a Serial warning explaining how to create it.

## Current features
- Read-only TCP client for LuxPower inverter data.
- Persistent TCP connection reuse.
- Input-register cache with polling.
- Holding-register cache with polling.
- Cache-only accessors for verified values.
- High-level metric registry for verified metrics.
- Verified PV1/PV2 power, battery SOC, battery charge power, and battery discharge power metrics.
- ESPHome external component wrapper for verified read-only metrics.
- Timeout-tolerant response handling and CRC validation.

## Register database
Only verified LuxPower register mappings are accepted. The current database is documented in docs/Registers.md and is used by the cache-only high-level helpers to avoid duplicating mapping metadata.

## Developer Register Scanner
Developer-only scanner helpers can dump cached input and holding registers and compare cached snapshots to find changed raw registers. These tools are cache-only and are documented in docs/RegisterDiscovery.md. Scanner findings are candidates only and must not be added to the verified register database until independently confirmed.

## Future roadmap
- Add more verified high-level metrics only after mapping evidence is confirmed from project sources.
- Expand diagnostics and logging when needed.
- Refine the demo application for production-style monitoring.

## Build instructions
From the project root:

```bash
platformio run --environment esp32s3
```

## Library consumption from GitHub
This repository includes a root `library.json` so PlatformIO and ESPHome can
consume `LuxPowerTCP` directly from GitHub while preserving the existing source
layout under `lib/LuxPowerTCP/src`.

For ESPHome integration design and version-pinning guidance, see
`docs/ESPHomeIntegration.md`.

Example ESPHome YAML is available in `examples/esphome-basic.yaml` for local
validation of the external component.

## Upload instructions
From the project root:

```bash
platformio run --target upload --environment esp32s3
```
