# Changelog

## [Unreleased]

### Stage 15B-15D
- Added a framework-neutral LuxPower transport abstraction plus Arduino compatibility transport.
- Added an ESPHome external component wrapper using ESPHome socket APIs for ESP-IDF builds.
- Added ESPHome sensor and binary sensor platforms for verified cache-only metrics.
- Added local ESPHome development documentation and a basic local validation YAML example.
- Verified ESPHome 2026.6.5 config and compile compatibility with ESP32-S3 ESP-IDF.

### Stage 12C
- Promoted PV1 Power, PV2 Power, Battery SOC, Battery Charge Power, and Battery Discharge Power to verified register descriptors.
- Added public cache-only high-level API methods and metric registry entries for the five newly verified metrics.
- Replaced temporary Stage 12B raw-register output with official metric registry output.
- Updated verification documentation with the real-inverter Home Assistant comparison results.

### Stage 12B
- Added temporary cache-only candidate metric output for real-inverter comparison.
- Documented the Stage 12B Home Assistant comparison table.

### Stage 12A
- Documented upstream luxpower-ha-integration candidate mappings for research only.

### Stage 11
- Added cache-only developer scanner methods for full input and holding register dumps.
- Added cache-only changed-register snapshots for input and holding register analysis.
- Updated the demo application with disabled-by-default full dumps and periodic changed-register scans.
- Documented the register discovery workflow for candidate mapping analysis.

### Stage 10
- Added a verified LuxPower register database header with strongly typed register metadata.
- Refactored the cache-only high-level helpers to depend on the register database as the single source of truth.
- Documented the accepted verified mappings and their verification evidence.
