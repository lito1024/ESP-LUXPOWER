<!-- AUTO-GENERATED - DO NOT EDIT MANUALLY -->
# Future Control Entities

Controls are documentation-only and are not exposed to ESPHome or UART.

| Key | Semantic intent | Source | Range / enum | Status | Safety constraints |
| --- | --- | --- | --- | --- | --- |
| feed_in_grid_enabled_control | Enable or disable feed-in to grid | holding register 21 bit 15 (corrected Stage 35 -- see feed_in_grid_enabled reconciliation) | boolean | CONTROL_CANDIDATE | Must be implemented as a semantic write with read-back verification, explicit user confirmation, inverter state checks, and rollback/error handling. Stage 35 reconciliation: this control's original source (holding register 22 bit 15, matching the old feed_in_grid_enabled read mapping) did not respond to the app's Zero Export control during live hardware testing and is now UNDER_REVIEW as a read entity -- do not target register 22 bit 15 for any future write. The upstream reference separately documents a historical incident where writing that exact bit corrupted the PV start voltage instead of toggling grid export. Target register 21 bit 15 (zero_export_enabled) instead, and re-verify with a read-back test before any write path is implemented. |
