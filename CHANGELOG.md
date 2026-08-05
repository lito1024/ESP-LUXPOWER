# Changelog

## 1.0.0 - 2026-08-05

Sweet POWER v1.0.0 release. Architecture frozen as the stable production
baseline for the Sweet Power project family -- see
`SweetPower-Project-Docs/RELEASE_NOTES_v1.0.md` for the full cross-project
release summary. Read-only throughout: only `MODBUS_FUNCTION_READ_HOLDING
(0x03)` and `MODBUS_FUNCTION_READ_INPUT (0x04)` are implemented. Includes
the second independent Modbus input register block (registers 125-249,
added to reach `load_power`) needed for Stage 32's two-inverter operation,
and the catalog's `update_class` (FAST/NORMAL/SLOW) property backing
catalog-driven telemetry scheduling, on top of everything below
(previously accumulated as unreleased work, now shipped as 1.0.0).

### Stage 22A
- Added a canonical machine-readable verified entity catalog.
- Added generated verified entity audit documentation and future control documentation.
- Added a deterministic entity-model generator with freshness check mode.

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
