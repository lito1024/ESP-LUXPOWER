<!-- AUTO-GENERATED - DO NOT EDIT MANUALLY -->
# Future Control Entities

Controls are documentation-only and are not exposed to ESPHome or UART.

| Key | Semantic intent | Source | Range / enum | Status | Safety constraints |
| --- | --- | --- | --- | --- | --- |
| feed_in_grid_enabled_control | Enable or disable feed-in to grid | holding register 22 bit 15 | boolean | CONTROL_CANDIDATE | Must be implemented as a semantic write with read-back verification, explicit user confirmation, inverter state checks, and rollback/error handling. |
