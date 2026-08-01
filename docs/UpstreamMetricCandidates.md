# Upstream Metric Candidates

Stage 12A is research only. The mappings below are extracted from the upstream Home Assistant integration and are candidates for future verification on the real inverter. Do not add them to `LuxPowerRegisters.h` until independently confirmed in this project.

Upstream repository inspected: `https://github.com/ant0nkr/luxpower-ha-integration`

Upstream commit inspected: `d3d101498bc2796d6d57142b0e8d7351fdd3cab6`

## How Upstream Sensors Work

- Standard sensors use `SENSOR_TYPES` descriptors with `register`, `register_type`, `extract`, and optional `scale`.
- Calculated sensors use `register_type: "calculated"` and run an `extract(registers, entry)` function against cached input-register data.
- Battery-block sensors use `BATTERY_SENSOR_TYPES`, `register_type: "battery"`, and data read from the input register range beginning at 5000.
- Entity names are derived from the configured prefix plus descriptor name. With an `lxp` prefix, `PV1 Power` becomes `sensor.lxp_pv1_power`.

## Current Project Coverage

Our current cache reads:

- Input registers `0..124`
- Holding registers `0..124`

Candidate values at input address `125+`, holding address `125+`, or battery block `5000+` are outside current cache coverage and require a future polling/cache design before runtime support.

## Candidate Table

| Requested HA entity | Upstream descriptor | Source evidence | Space | Address(es) | Type | Word order | Signedness | Scale | Unit | Direct / assembled / calculated | Formula | Current 0-124 coverage | Conditions / notes |
| --- | --- | --- | --- | ---: | --- | --- | --- | ---: | --- | --- | --- | --- | --- |
| `sensor.lxp_pv1_power` | `PV1 Power` / `I_PPV1` | `sensor_types.py:169-176`; `input_registers.py:16` | Input | 7 | U16 | N/A | unsigned by implementation | 1.0 | W | Direct register | `value * 1.0` | Covered | No model condition in descriptor. |
| `sensor.lxp_pv2_power` | `PV2 Power` / `I_PPV2` | `sensor_types.py:220-227`; `input_registers.py:17` | Input | 8 | U16 | N/A | unsigned by implementation | 1.0 | W | Direct register | `value * 1.0` | Covered | No model condition in descriptor. |
| `sensor.lxp_battery_capacity` | `Battery Capacity` / `I_BAT_CAPACITY` | `sensor_types.py:1834-1841`; `input_registers.py:158` | Input | 97 | U16 | N/A | unsigned by implementation | 1.0 | Ah | Direct register | `value * 1.0` | Covered | `master_only: True`. This is not the per-battery `Capacity` from the 5000+ battery block. |
| `sensor.lxp_battery_soc` | `Battery SOC` / `I_SOC_SOH` | `sensor_types.py:496-503`; `input_registers.py:14` | Input | 5 | Bit/packed U16 field | N/A | unsigned field | 1.0 | % | Direct register with field extraction | `(value & 0xFF) * 1.0` | Covered | `master_only: True`; low byte is SOC, high byte is SOH. |
| `sensor.lxp_battery_charge_power` | `Battery Charge Power` / `I_PCHARGE` | `sensor_types.py:571-578`; `input_registers.py:19` | Input | 10 | U16 | N/A | unsigned by implementation | 1.0 | W | Direct register | `value * 1.0` | Covered | No model condition in descriptor. |
| `sensor.lxp_battery_discharge_power` | `Battery Discharge Power` / `I_PDISCHARGE` | `sensor_types.py:587-594`; `input_registers.py:20` | Input | 11 | U16 | N/A | unsigned by implementation | 1.0 | W | Direct register | `value * 1.0` | Covered | No model condition in descriptor. |
| `sensor.lxp_bms_max_cell_temperature` | `BMS Max Cell Temperature` / `I_BMS_MAX_CELL_TEMP` | `sensor_types.py:1944-1951`; `input_registers.py:164` | Input | 103 | S16 candidate | N/A | Constant says signed; implementation does not sign-convert | 0.1 | deg C | Direct register | Upstream entity does `value * 0.1`; register comment says signed number | Covered | `master_only: True`. Verify signed handling before adding. |
| `sensor.lxp_grid_flow` | `Grid Flow` / `I_PTOUSER`, `I_PTOGRID` | `sensor_types.py:75-86`; `input_registers.py:32-33` | Input | 27, 26 | derived | N/A | signed result from unsigned operands | 1.0 | W | Calculated from other metrics | `I_PTOUSER - I_PTOGRID` | Covered | Positive means import/from-grid exceeds export/to-grid. Negative means export/to-grid exceeds import/from-grid. |
| `sensor.lxp_power_to_grid` | `Power to Grid` / `I_PTOGRID` | `sensor_types.py:899-906`; `input_registers.py:32` | Input | 26 | U16 | N/A | unsigned by implementation | 1.0 | W | Direct register | `value * 1.0` | Covered | Export power. R-phase for 3-phase systems according to register comment. |
| `sensor.lxp_power_from_grid` | `Power from Grid` / `I_PTOUSER` | `sensor_types.py:947-954`; `input_registers.py:33` | Input | 27 | U16 | N/A | unsigned by implementation | 1.0 | W | Direct register | `value * 1.0` | Covered | Import power. R-phase for 3-phase systems according to register comment. |
| `sensor.lxp_load_power` | `Load Power` / `I_PLOAD` | `sensor_types.py:995-1002`; `input_registers.py:226` | Input | 170 | U16 | N/A | unsigned by implementation | 1.0 | W | Direct register | `value * 1.0` | Not covered | Requires an input block beyond `0..124`. Register comment says load consumption when working in on-grid mode. |

## Related But Not Exact Requested Entities

- `Capacity` in `BATTERY_SENSOR_TYPES` uses `B_CAPACITY = 3`, from battery block `5000+`, so absolute battery block offset is `5000 + block * 30 + 3`. It creates per-battery entities containing the battery serial in the entity id, not plain `sensor.lxp_battery_capacity`.
- `Max Cell Temperature` in `BATTERY_SENSOR_TYPES` uses `B_MAX_CELL_TEMP = 12`, from battery block `5000+`, so absolute battery block offset is `5000 + block * 30 + 12`. This is distinct from `BMS Max Cell Temperature` at input register 103.

## Grid Sign Convention

Upstream defines:

- `Power to Grid`: direct input register `I_PTOGRID = 26`, export power.
- `Power from Grid`: direct input register `I_PTOUSER = 27`, import power.
- `Grid Flow`: calculated as `I_PTOUSER - I_PTOGRID`.

Therefore:

- Positive `grid_flow` means net import from grid to user.
- Negative `grid_flow` means net export from user/inverter to grid.
- `power_to_grid` is not derived from `grid_flow`; it is a direct register candidate.
- `power_from_grid` is not derived from `grid_flow`; it is a direct register candidate.

## Evidence Index

- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:75-86`: `Grid Flow` formula.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:169-176`: `PV1 Power` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:220-227`: `PV2 Power` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:496-503`: `Battery SOC` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:571-578`: `Battery Charge Power` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:587-594`: `Battery Discharge Power` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:899-906`: `Power to Grid` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:947-954`: `Power from Grid` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:995-1002`: `Load Power` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:1834-1841`: `Battery Capacity` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:1944-1951`: `BMS Max Cell Temperature` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:2959-2966`: per-battery `Capacity` descriptor.
- `custom_components/lxp_modbus/entity_descriptions/sensor_types.py:3069-3076`: per-battery `Max Cell Temperature` descriptor.
- `custom_components/lxp_modbus/constants/input_registers.py:14-20`: core SOC/PV/battery power input register constants.
- `custom_components/lxp_modbus/constants/input_registers.py:32-33`: grid import/export input register constants.
- `custom_components/lxp_modbus/constants/input_registers.py:157-164`: battery capacity and BMS max cell temperature constants.
- `custom_components/lxp_modbus/constants/input_registers.py:226`: `I_PLOAD = 170`.
- `custom_components/lxp_modbus/constants/battery_registers.py:1-13`: battery block offsets and `5000+` block note.
- `custom_components/lxp_modbus/classes/lxp_batteries.py:12-34`: battery blocks are 30 registers wide from `BATTERY_INFO_START_REGISTER`.
- `custom_components/lxp_modbus/classes/modbus_client.py:187-207`: upstream polls input and holding ranges in block-size chunks up to `TOTAL_REGISTERS`.
- `custom_components/lxp_modbus/classes/modbus_client.py:194-203`: upstream optionally polls battery data from `BATTERY_INFO_START_REGISTER`.
- `custom_components/lxp_modbus/const.py:44,54`: `TOTAL_REGISTERS = 750`, `BATTERY_INFO_START_REGISTER = 5000`.
- `custom_components/lxp_modbus/sensor.py:132-165`: sensor extraction and scale application.
- `custom_components/lxp_modbus/entity.py:27-33`: entity name derivation from descriptor name and configured prefix.

## Safest First Verification Group

The safest first group is made of direct U16 input-register values already inside our current `0..124` input cache:

1. `PV1 Power` at input register 7.
2. `PV2 Power` at input register 8.
3. `Battery SOC` low byte at input register 5.
4. `Battery Charge Power` at input register 10.
5. `Battery Discharge Power` at input register 11.

These avoid new block reads, battery `5000+` reads, signed-temperature ambiguity, and derived grid sign interpretation during the first verification pass.
