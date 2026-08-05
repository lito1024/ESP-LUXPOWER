<!-- AUTO-GENERATED - DO NOT EDIT MANUALLY -->
# Entity Catalog

Source catalog: `data/luxpower_entities.json`.

Stage 28 policy: `luxpower_entities.json` is the canonical source of truth. Entities in `entities[]` are production by default (status `PRODUCTION`, `UNDER_REVIEW`, or `KNOWN_ISSUE` -- all three are generated/transported/displayed). `DEPRECATED` entities are retired and no longer generated. A real discrepancy downgrades only the affected entity to `UNDER_REVIEW`, never unrelated entities.

## Battery

| Key | Name | Source register | Type | Scale | Unit | Status | Update class | Protocol group | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| battery_soc | Battery SOC | input[5] | uint16_low_byte | 1.0 | % | PRODUCTION | FAST | FAST_TELEMETRY | Low byte of input register 5: raw & 0xFF. |
| battery_charge_power | Battery Charge Power | input[10] | uint16 | 1.0 | W | PRODUCTION | FAST | FAST_TELEMETRY | Direct U16 input register verified against Home Assistant sensor.lxp_battery_charge_power. |
| battery_discharge_power | Battery Discharge Power | input[11] | uint16 | 1.0 | W | PRODUCTION | FAST | FAST_TELEMETRY | Direct U16 input register verified against Home Assistant sensor.lxp_battery_discharge_power. |
| battery_capacity | Battery Capacity | input[97] | uint16 | 1.0 | Ah | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register. Promoted from CANDIDATE to PRODUCTION under the Stage 28 policy update: luxpower_entities.json is now the canonical source of truth and no longer requires a dedicated manual verification stage before production use. Wire field ID/group/scale unchanged from the Stage 28 diagnostic wiring. |
| bms_max_cell_temperature | BMS Max Cell Temperature | input[103] | int16_candidate | 0.1 | deg C | PRODUCTION | NORMAL | DIAGNOSTICS | Upstream register comment claims signed, but the reference implementation does not sign-convert; the Gateway forwards the raw register value unchanged (see luxpower-gateway/src/main.cpp). Promoted from CANDIDATE to PRODUCTION under the Stage 28 policy update. If real-world negative-temperature behavior is ever observed to be wrong, mark this single entity UNDER_REVIEW rather than reverting the whole catalog. |
| battery_voltage | Battery Voltage | input[4] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| battery_soh | Battery SOH | input[5] | uint16_high_byte | 1.0 | % | PRODUCTION | SLOW | DIAGNOSTICS | High byte of input register 5: (raw>>8)&0xFF. Same register our existing battery_soc reads (low byte). |
| battery_temperature | Battery Temperature | input[67] | uint16 | 1.0 | deg C | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register. |
| battery_parallel_number | Battery Parallel Number | input[96] | uint16 | 1.0 |  | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, count of parallel battery packs. |
| battery_type_and_brand | Battery Type And Brand | input[80] | uint16 | 1.0 |  | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, raw vendor/type code. |
| bms_max_charge_current | BMS Max Charge Current | input[81] | uint16 | 0.1 | A | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register. Upstream constants file comments 0.01A but upstream sensor code uses scale 0.1 -- following the sensor code (the actually-executed conversion), flagged as an upstream doc/code discrepancy. |
| bms_max_discharge_current | BMS Max Discharge Current | input[82] | uint16 | 0.1 | A | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register. Same upstream doc/code scale discrepancy as bms_max_charge_current. |
| bms_charge_voltage_reference | BMS Charge Voltage Reference | input[83] | uint16 | 0.1 | V | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| bms_discharge_cutoff_voltage | BMS Discharge Cutoff Voltage | input[84] | uint16 | 0.1 | V | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| bms_battery_current | BMS Battery Current | input[98] | int16 | 0.1 | A | PRODUCTION | NORMAL | DIAGNOSTICS | Signed S16 input register (sign-extended by the Gateway before forwarding), native 0.1A/LSB. |
| bms_fault_code | BMS Fault Code | input[99] | uint16 | 1.0 |  | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, raw BMS fault code (numeric only this stage; text lookup not wire-transmitted). |
| bms_warning_code | BMS Warning Code | input[100] | uint16 | 1.0 |  | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, raw BMS warning code (numeric only this stage). |
| bms_max_cell_voltage | BMS Max Cell Voltage | input[101] | uint16 | 0.001 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native millivolts, forwarded unchanged. |
| bms_min_cell_voltage | BMS Min Cell Voltage | input[102] | uint16 | 0.001 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native millivolts, forwarded unchanged. |
| bms_min_cell_temperature | BMS Min Cell Temperature | input[104] | int16_candidate | 0.1 | deg C | PRODUCTION | NORMAL | DIAGNOSTICS | Upstream applies no sign conversion (same pattern as our existing bms_max_cell_temperature at register 103); forwarded unchanged. |
| bms_firmware_update_state | BMS Firmware Update State | input[105] | uint16 | 1.0 |  | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, enum code {0:Idle,1:In Process,2:Success,3:Failed}. |
| bms_cycle_count | BMS Cycle Count | input[106] | uint16 | 1.0 | cycles | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, total_increasing. |
| inverter_battery_voltage_sample | Inverter Battery Voltage Sample | input[107] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| charge_energy_today | Charge Energy Today | input[33] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. |
| discharge_energy_today | Discharge Energy Today | input[34] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. |
| charge_energy_total | Charge Energy Total | input[50] | uint32 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | U32 input register pair (50 low, 51 high), native 0.1kWh/LSB, forwarded unchanged. |
| discharge_energy_total | Discharge Energy Total | input[52] | uint32 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | U32 input register pair (52 low, 53 high), native 0.1kWh/LSB, forwarded unchanged. |
| battery_flow | Battery Flow | derived | derived_int32 | 1.0 | W | PRODUCTION | NORMAL | DIAGNOSTICS | Derived: battery_discharge_power - battery_charge_power, matches upstream formula `PDISCHARGE - PCHARGE`. Positive means net discharge. |
| bms_cell_difference | BMS Cell Difference | derived | derived_int32 | 1.0 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Derived: bms_max_cell_voltage - bms_min_cell_voltage, both already raw millivolt passthrough, so this is a plain integer subtraction of the two raw register values. |

## EPS

| Key | Name | Source register | Type | Scale | Unit | Status | Update class | Protocol group | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| eps_voltage | EPS Voltage | input[20] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register (R-phase), native 0.1V/LSB, forwarded unchanged. |
| eps_voltage_s | EPS Voltage S-Phase | input[21] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| eps_voltage_t | EPS Voltage T-Phase | input[22] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| eps_frequency | EPS Frequency | input[23] | uint16 | 0.01 | Hz | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.01Hz/LSB, forwarded unchanged. |
| eps_power | EPS Power | input[24] | uint16 | 1.0 | W | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register (R-phase). |
| eps_apparent_power | EPS Apparent Power | input[25] | uint16 | 1.0 | VA | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register (R-phase). |
| eps_energy_today | EPS Energy Today | input[35] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. |
| eps_energy_total | EPS Energy Total | input[54] | uint32 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | U32 input register pair (54 low, 55 high), native 0.1kWh/LSB, forwarded unchanged. |

## Energy

| Key | Name | Source register | Type | Scale | Unit | Status | Update class | Protocol group | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| pv1_energy_total | PV1 Energy Total | input[40] | uint32 | 0.1 | kWh | PRODUCTION | SLOW | ENERGY_TOTALS | U32 input register pair, low word first, converted to kWh in LuxPowerTCP. |
| inverter_energy_today | Inverter Energy Today | input[31] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. |
| ac_charge_energy_today | AC Charge Energy Today | input[32] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. |
| inverter_energy_total | Inverter Energy Total (Raw) | input[46] | uint32 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | U32 input register pair (46 low, 47 high), native 0.1kWh/LSB, forwarded unchanged. Named '(Raw)' to distinguish from our existing pv1_energy_total which uses the legacy verified-metric Wh-based API. |
| ac_charge_energy_total | AC Charge Energy Total | input[48] | uint32 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | U32 input register pair (48 low, 49 high), native 0.1kWh/LSB, forwarded unchanged. |
| total_running_time | Total Running Time | input[69] | uint32 | 1.0 | s | PRODUCTION | SLOW | DIAGNOSTICS | U32 input register pair (69 low, 70 high), seconds, total_increasing. |
| generator_voltage | Generator Voltage | input[121] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| generator_frequency | Generator Frequency | input[122] | uint16 | 0.01 | Hz | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.01Hz/LSB, forwarded unchanged. |
| generator_power | Generator Power | input[123] | uint16 | 1.0 | W | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register. |
| generator_energy_today | Generator Energy Today | input[124] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. Last register in the Gateway's current 0..124 poll block. |

## Grid

| Key | Name | Source register | Type | Scale | Unit | Status | Update class | Protocol group | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| power_to_grid | Power To Grid | input[26] | uint16 | 1.0 | W | PRODUCTION | FAST | FAST_TELEMETRY | Direct U16 input register. Promoted from CANDIDATE to PRODUCTION under the Stage 28 policy update. Wire field ID/group/scale unchanged. |
| power_from_grid | Power From Grid | input[27] | uint16 | 1.0 | W | PRODUCTION | FAST | FAST_TELEMETRY | Direct U16 input register. Promoted from CANDIDATE to PRODUCTION under the Stage 28 policy update. Wire field ID/group/scale unchanged. |
| grid_flow | Grid Flow | derived | derived_int32 | 1.0 | W | PRODUCTION | NORMAL | DIAGNOSTICS | Derived: power_from_grid - power_to_grid (I_PTOUSER - I_PTOGRID per docs/UpstreamMetricCandidates.md). Positive means net import, negative means net export. Promoted from CANDIDATE to PRODUCTION under the Stage 28 policy update, depending on the now-production power_to_grid/power_from_grid entities above. |
| grid_voltage | Grid Voltage | input[12] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register (R-phase), native 0.1V/LSB, forwarded unchanged. |
| grid_voltage_s | Grid Voltage S-Phase | input[13] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| grid_voltage_t | Grid Voltage T-Phase | input[14] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| grid_frequency | Grid Frequency | input[15] | uint16 | 0.01 | Hz | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.01Hz/LSB, forwarded unchanged. |
| inverter_power | Inverter Power | input[16] | uint16 | 1.0 | W | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register (R-phase). |
| ac_charging_rectification_power | AC Charging Rectification Power | input[17] | uint16 | 1.0 | W | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register (R-phase). |
| inverter_current | Inverter Current | input[18] | uint16 | 0.01 | A | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register (R-phase), native 0.01A/LSB, forwarded unchanged. |
| power_factor | Power Factor | input[19] | uint16 | 0.001 | % | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register (R-phase), native 0.001/LSB, forwarded unchanged. Unit as documented upstream (sensor_types.py) -- flagged as an unusually small unit for a percentage; treat with caution until cross-checked against Home Assistant. |
| energy_to_grid_today | Energy To Grid Today | input[36] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. |
| energy_from_grid_today | Energy From Grid Today | input[37] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. |
| energy_to_grid_total | Energy To Grid Total | input[56] | uint32 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | U32 input register pair (56 low, 57 high), native 0.1kWh/LSB, forwarded unchanged. |
| energy_from_grid_total | Energy From Grid Total | input[58] | uint32 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | U32 input register pair (58 low, 59 high), native 0.1kWh/LSB, forwarded unchanged. |
| ongrid_load_power | On-Grid Load Power | input[114] | uint16 | 1.0 | W | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register (R-phase). |
| bus1_voltage | Bus 1 Voltage | input[38] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| bus2_voltage | Bus 2 Voltage | input[39] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| half_bus_voltage | Half Bus Voltage | input[120] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| grid_connected | Grid Connected | derived | derived_bool | 1.0 |  | PRODUCTION | NORMAL | DIAGNOSTICS | Derived boolean: grid_frequency raw > 0, matches upstream `I_FAC > 0`. Sent as bool-as-int32, same convention as feed_in_grid_enabled. |
| load_power | Load Power | input[170] | uint16 | 1.0 | W | PRODUCTION | FAST | FAST_TELEMETRY | Direct U16 input register 170 (I_PLOAD), load consumption when working in on-grid mode per upstream register comment. Outside the original 0..124 poll block; Stage 31 added a second contiguous 125-register input block (125..249) specifically to reach this register for FAST-class telemetry, at the cost of one additional Modbus request per ~1s poll cycle. Promoted from NOT_WIRED candidate to PRODUCTION under the Stage 28 policy. |

## PV

| Key | Name | Source register | Type | Scale | Unit | Status | Update class | Protocol group | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| pv1_power | PV1 Power | input[7] | uint16 | 1.0 | W | PRODUCTION | FAST | FAST_TELEMETRY | Direct U16 input register verified against Home Assistant sensor.lxp_pv1_power. |
| pv2_power | PV2 Power | input[8] | uint16 | 1.0 | W | PRODUCTION | FAST | FAST_TELEMETRY | Direct U16 input register verified against Home Assistant sensor.lxp_pv2_power. |
| pv1_voltage | PV1 Voltage | input[1] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| pv2_voltage | PV2 Voltage | input[2] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| pv3_voltage | PV3 Voltage | input[3] | uint16 | 0.1 | V | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, native 0.1V/LSB, forwarded unchanged. |
| pv3_power | PV3 Power | input[9] | uint16 | 1.0 | W | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, same pattern as pv1_power/pv2_power. |
| pv1_energy_today | PV1 Energy Today | input[28] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. |
| pv2_energy_today | PV2 Energy Today | input[29] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. |
| pv3_energy_today | PV3 Energy Today | input[30] | uint16 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | Direct U16 input register, native 0.1kWh/LSB, forwarded unchanged. |
| pv2_energy_total | PV2 Energy Total | input[42] | uint32 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | U32 input register pair (42 low, 43 high), native 0.1kWh/LSB, forwarded unchanged. Raw passthrough differs from pv1_energy_total's legacy Wh-based verified-metric API (protocol_scale 1000) -- see notes in ENTITY_MODEL.md. |
| pv3_energy_total | PV3 Energy Total | input[44] | uint32 | 0.1 | kWh | PRODUCTION | SLOW | DIAGNOSTICS | U32 input register pair (44 low, 45 high), native 0.1kWh/LSB, forwarded unchanged. |
| pv1_current | PV1 Current | derived | derived_int32 | 1.0 | A | PRODUCTION | NORMAL | DIAGNOSTICS | Derived: pv1_power / (pv1_voltage * 0.1), matches upstream formula `PPV1/(VPV1*0.1)`. Guarded against divide-by-zero (night/no-PV). |
| pv2_current | PV2 Current | derived | derived_int32 | 1.0 | A | PRODUCTION | NORMAL | DIAGNOSTICS | Derived: pv2_power / (pv2_voltage * 0.1), matches upstream formula `PPV2/(VPV2*0.1)`. |
| pv3_current | PV3 Current | derived | derived_int32 | 1.0 | A | PRODUCTION | NORMAL | DIAGNOSTICS | Derived: pv3_power / (pv3_voltage * 0.1), matches upstream formula `PPV3/(VPV3*0.1)`. |

## Status

| Key | Name | Source register | Type | Scale | Unit | Status | Update class | Protocol group | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| feed_in_grid_enabled | Feed-In Grid Enabled | holding[22] bit 15 | bit | 1.0 |  | PRODUCTION | NORMAL | STATUS | Read-only exposure of verified holding bit 15. No writes are allowed in this stage. |
| inverter_state | Inverter State | input[0] | uint16 | 1.0 |  | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, raw inverter state code (numeric only this stage; text lookup not wire-transmitted). |
| internal_fault_code | Internal Fault Code | input[6] | uint16 | 1.0 |  | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, raw internal fault code. |
| ac_input_type | AC Input Type | input[77] | uint16_bit0 | 1.0 |  | PRODUCTION | NORMAL | DIAGNOSTICS | Bit 0 of input register 77: raw&1. Options {0:Grid,1:Generator}. |
| auto_test_status | Auto Test Status | input[71] | uint16 | 1.0 |  | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register, raw bitfield. |
| active_fault_code | Active Fault Code | derived | derived_bool | 1.0 |  | PRODUCTION | NORMAL | DIAGNOSTICS | Derived: highest set bit index of the 32-bit value formed from input registers 60(low)/61(high), matches upstream `get_highest_set_bit((H<<16)|L)`. Numeric code only this stage; FAULT_CODES text lookup is display-side future work, not wire-transmitted. |
| active_warning_code | Active Warning Code | derived | derived_bool | 1.0 |  | PRODUCTION | NORMAL | DIAGNOSTICS | Derived: highest set bit index of the 32-bit value formed from input registers 62(low)/63(high), matches upstream `get_highest_set_bit((H<<16)|L)`. Numeric code only this stage; WARNING_CODES text lookup is display-side future work, not wire-transmitted. |

## Temperature

| Key | Name | Source register | Type | Scale | Unit | Status | Update class | Protocol group | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| internal_temperature | Internal Temperature | input[64] | uint16 | 1.0 | deg C | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register. |
| radiator_temperature | Radiator Temperature | input[65] | uint16 | 1.0 | deg C | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register. |
| radiator_temperature_2 | Radiator Temperature 2 | input[66] | uint16 | 1.0 | deg C | PRODUCTION | NORMAL | DIAGNOSTICS | Direct U16 input register. |

## Entities Under Review Or With Known Issues

None. All production entities are currently in normal `PRODUCTION` status.

## Catalogued But Not Wired

Register mappings that are catalogued in `candidates[]` but do not carry a `protocol_field_id` -- they are not part of the wire protocol at all yet, and this is a wiring/transport gap, not a verification gap.

| Key | Name | Status | Area | Address | Unit | Reason not wired |
| --- | --- | --- | --- | --- | --- | --- |

## Potential Future Controls Not Included

| Key | Semantic intent | Source | Status | Safety constraints |
| --- | --- | --- | --- | --- |
| feed_in_grid_enabled_control | Enable or disable feed-in to grid | holding register 22 bit 15 | CONTROL_CANDIDATE | Must be implemented as a semantic write with read-back verification, explicit user confirmation, inverter state checks, and rollback/error handling. |

## Duplicates / Aliases

None identified in the catalog.
