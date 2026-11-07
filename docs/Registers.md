<!-- AUTO-GENERATED - DO NOT EDIT MANUALLY -->
# LuxPower Register Database

Generated from `data/luxpower_entities.json` (the canonical catalog, Stage 28) by `tools/generate_entity_model.py`. Edit the catalog, never this file. lxp_modbus and the upstream integration are references only (D-020). "Checked on site" lists the checks made on our own inverters; an empty cell means catalog/upstream knowledge only. Derived values are in `docs/VerifiedEntityCatalog.md`.

| Space | Address | Bit | Type | Scale | Unit | Key | Name | Status | Field ID | Update class | Checked on site |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| input | 0 |  | uint16 | 1.0 |  | inverter_state | Inverter State | PRODUCTION | 9064 | NORMAL |  |
| input | 1 |  | uint16 | 0.1 | V | pv1_voltage | PV1 Voltage | PRODUCTION | 9006 | NORMAL |  |
| input | 2 |  | uint16 | 0.1 | V | pv2_voltage | PV2 Voltage | PRODUCTION | 9007 | NORMAL |  |
| input | 3 |  | uint16 | 0.1 | V | pv3_voltage | PV3 Voltage | PRODUCTION | 9008 | NORMAL |  |
| input | 4 |  | uint16 | 0.1 | V | battery_voltage | Battery Voltage | PRODUCTION | 9015 | NORMAL |  |
| input | 5 |  | uint16_low_byte | 1.0 | % | battery_soc | Battery SOC | PRODUCTION | 2001 | FAST | Stage 12B real-inverter comparison with Home Assistant |
| input | 5 |  | uint16_high_byte | 1.0 | % | battery_soh | Battery SOH | PRODUCTION | 9016 | SLOW |  |
| input | 6 |  | uint16 | 1.0 |  | internal_fault_code | Internal Fault Code | PRODUCTION | 9065 | NORMAL |  |
| input | 7 |  | uint16 | 1.0 | W | pv1_power | PV1 Power | PRODUCTION | 1001 | FAST | Stage 12B real-inverter comparison with Home Assistant |
| input | 8 |  | uint16 | 1.0 | W | pv2_power | PV2 Power | PRODUCTION | 1002 | FAST | Stage 12B real-inverter comparison with Home Assistant |
| input | 9 |  | uint16 | 1.0 | W | pv3_power | PV3 Power | PRODUCTION | 9009 | NORMAL |  |
| input | 10 |  | uint16 | 1.0 | W | battery_charge_power | Battery Charge Power | PRODUCTION | 2002 | FAST | Stage 12B real-inverter comparison with Home Assistant |
| input | 11 |  | uint16 | 1.0 | W | battery_discharge_power | Battery Discharge Power | PRODUCTION | 2003 | FAST | Stage 12B real-inverter comparison with Home Assistant |
| input | 12 |  | uint16 | 0.1 | V | grid_voltage | Grid Voltage | PRODUCTION | 9037 | NORMAL |  |
| input | 13 |  | uint16 | 0.1 | V | grid_voltage_s | Grid Voltage S-Phase | PRODUCTION | 9038 | NORMAL |  |
| input | 14 |  | uint16 | 0.1 | V | grid_voltage_t | Grid Voltage T-Phase | PRODUCTION | 9039 | NORMAL |  |
| input | 15 |  | uint16 | 0.01 | Hz | grid_frequency | Grid Frequency | PRODUCTION | 9040 | NORMAL |  |
| input | 16 |  | uint16 | 1.0 | W | inverter_power | Inverter Power | PRODUCTION | 9041 | NORMAL |  |
| input | 17 |  | uint16 | 1.0 | W | ac_charging_rectification_power | AC Charging Rectification Power | PRODUCTION | 9042 | NORMAL |  |
| input | 18 |  | uint16 | 0.01 | A | inverter_current | Inverter Current | PRODUCTION | 9043 | NORMAL |  |
| input | 19 |  | uint16 | 0.001 | % | power_factor | Power Factor | PRODUCTION | 9044 | NORMAL |  |
| input | 20 |  | uint16 | 0.1 | V | eps_voltage | EPS Voltage | PRODUCTION | 9053 | NORMAL |  |
| input | 21 |  | uint16 | 0.1 | V | eps_voltage_s | EPS Voltage S-Phase | PRODUCTION | 9054 | NORMAL |  |
| input | 22 |  | uint16 | 0.1 | V | eps_voltage_t | EPS Voltage T-Phase | PRODUCTION | 9055 | NORMAL |  |
| input | 23 |  | uint16 | 0.01 | Hz | eps_frequency | EPS Frequency | PRODUCTION | 9056 | NORMAL |  |
| input | 24 |  | uint16 | 1.0 | W | eps_power | EPS Power | PRODUCTION | 9057 | NORMAL |  |
| input | 25 |  | uint16 | 1.0 | VA | eps_apparent_power | EPS Apparent Power | PRODUCTION | 9058 | NORMAL |  |
| input | 26 |  | uint16 | 1.0 | W | power_to_grid | Power To Grid | PRODUCTION | 9003 | FAST |  |
| input | 27 |  | uint16 | 1.0 | W | power_from_grid | Power From Grid | PRODUCTION | 9004 | FAST |  |
| input | 28 |  | uint16 | 0.1 | kWh | pv1_energy_today | PV1 Energy Today | PRODUCTION | 9010 | SLOW |  |
| input | 29 |  | uint16 | 0.1 | kWh | pv2_energy_today | PV2 Energy Today | PRODUCTION | 9011 | SLOW |  |
| input | 30 |  | uint16 | 0.1 | kWh | pv3_energy_today | PV3 Energy Today | PRODUCTION | 9012 | SLOW |  |
| input | 31 |  | uint16 | 0.1 | kWh | inverter_energy_today | Inverter Energy Today | PRODUCTION | 9068 | SLOW |  |
| input | 32 |  | uint16 | 0.1 | kWh | ac_charge_energy_today | AC Charge Energy Today | PRODUCTION | 9069 | SLOW |  |
| input | 33 |  | uint16 | 0.1 | kWh | charge_energy_today | Charge Energy Today | PRODUCTION | 9033 | SLOW |  |
| input | 34 |  | uint16 | 0.1 | kWh | discharge_energy_today | Discharge Energy Today | PRODUCTION | 9034 | SLOW |  |
| input | 35 |  | uint16 | 0.1 | kWh | eps_energy_today | EPS Energy Today | PRODUCTION | 9059 | SLOW |  |
| input | 36 |  | uint16 | 0.1 | kWh | energy_to_grid_today | Energy To Grid Today | PRODUCTION | 9045 | SLOW |  |
| input | 37 |  | uint16 | 0.1 | kWh | energy_from_grid_today | Energy From Grid Today | PRODUCTION | 9046 | SLOW |  |
| input | 38 |  | uint16 | 0.1 | V | bus1_voltage | Bus 1 Voltage | PRODUCTION | 9050 | NORMAL |  |
| input | 39 |  | uint16 | 0.1 | V | bus2_voltage | Bus 2 Voltage | PRODUCTION | 9051 | NORMAL |  |
| input | 40 |  | uint32 | 0.1 | kWh | pv1_energy_total | PV1 Energy Total | PRODUCTION | 5001 | SLOW | current LuxPower logs; previous working parser; Home Assistant value |
| input | 42 |  | uint32 | 0.1 | kWh | pv2_energy_total | PV2 Energy Total | PRODUCTION | 9013 | SLOW |  |
| input | 44 |  | uint32 | 0.1 | kWh | pv3_energy_total | PV3 Energy Total | PRODUCTION | 9014 | SLOW |  |
| input | 46 |  | uint32 | 0.1 | kWh | inverter_energy_total | Inverter Energy Total (Raw) | PRODUCTION | 9070 | SLOW |  |
| input | 48 |  | uint32 | 0.1 | kWh | ac_charge_energy_total | AC Charge Energy Total | PRODUCTION | 9071 | SLOW |  |
| input | 50 |  | uint32 | 0.1 | kWh | charge_energy_total | Charge Energy Total | PRODUCTION | 9035 | SLOW |  |
| input | 52 |  | uint32 | 0.1 | kWh | discharge_energy_total | Discharge Energy Total | PRODUCTION | 9036 | SLOW |  |
| input | 54 |  | uint32 | 0.1 | kWh | eps_energy_total | EPS Energy Total | PRODUCTION | 9060 | SLOW |  |
| input | 56 |  | uint32 | 0.1 | kWh | energy_to_grid_total | Energy To Grid Total | PRODUCTION | 9047 | SLOW |  |
| input | 58 |  | uint32 | 0.1 | kWh | energy_from_grid_total | Energy From Grid Total | PRODUCTION | 9048 | SLOW |  |
| input | 64 |  | uint16 | 1.0 | deg C | internal_temperature | Internal Temperature | PRODUCTION | 9061 | NORMAL |  |
| input | 65 |  | uint16 | 1.0 | deg C | radiator_temperature | Radiator Temperature | PRODUCTION | 9062 | NORMAL |  |
| input | 66 |  | uint16 | 1.0 | deg C | radiator_temperature_2 | Radiator Temperature 2 | PRODUCTION | 9063 | NORMAL |  |
| input | 67 |  | uint16 | 1.0 | deg C | battery_temperature | Battery Temperature | PRODUCTION | 9017 | NORMAL |  |
| input | 69 |  | uint32 | 1.0 | s | total_running_time | Total Running Time | PRODUCTION | 9072 | SLOW |  |
| input | 71 |  | uint16 | 1.0 |  | auto_test_status | Auto Test Status | PRODUCTION | 9067 | NORMAL |  |
| input | 77 |  | uint16_bit0 | 1.0 |  | ac_input_type | AC Input Type | PRODUCTION | 9066 | NORMAL |  |
| input | 80 |  | uint16 | 1.0 |  | battery_type_and_brand | Battery Type And Brand | PRODUCTION | 9019 | SLOW |  |
| input | 81 |  | uint16 | 0.1 | A | bms_max_charge_current | BMS Max Charge Current | PRODUCTION | 9020 | SLOW |  |
| input | 82 |  | uint16 | 0.1 | A | bms_max_discharge_current | BMS Max Discharge Current | PRODUCTION | 9021 | SLOW |  |
| input | 83 |  | uint16 | 0.1 | V | bms_charge_voltage_reference | BMS Charge Voltage Reference | PRODUCTION | 9022 | SLOW |  |
| input | 84 |  | uint16 | 0.1 | V | bms_discharge_cutoff_voltage | BMS Discharge Cutoff Voltage | PRODUCTION | 9023 | SLOW |  |
| input | 96 |  | uint16 | 1.0 |  | battery_parallel_number | Battery Parallel Number | PRODUCTION | 9018 | SLOW |  |
| input | 97 |  | uint16 | 1.0 | Ah | battery_capacity | Battery Capacity | PRODUCTION | 9001 | SLOW |  |
| input | 98 |  | int16 | 0.1 | A | bms_battery_current | BMS Battery Current | PRODUCTION | 9024 | NORMAL |  |
| input | 99 |  | uint16 | 1.0 |  | bms_fault_code | BMS Fault Code | PRODUCTION | 9025 | NORMAL |  |
| input | 100 |  | uint16 | 1.0 |  | bms_warning_code | BMS Warning Code | PRODUCTION | 9026 | NORMAL |  |
| input | 101 |  | uint16 | 0.001 | V | bms_max_cell_voltage | BMS Max Cell Voltage | PRODUCTION | 9027 | NORMAL |  |
| input | 102 |  | uint16 | 0.001 | V | bms_min_cell_voltage | BMS Min Cell Voltage | PRODUCTION | 9028 | NORMAL |  |
| input | 103 |  | int16_candidate | 0.1 | deg C | bms_max_cell_temperature | BMS Max Cell Temperature | PRODUCTION | 9002 | NORMAL |  |
| input | 104 |  | int16_candidate | 0.1 | deg C | bms_min_cell_temperature | BMS Min Cell Temperature | PRODUCTION | 9029 | NORMAL |  |
| input | 105 |  | uint16 | 1.0 |  | bms_firmware_update_state | BMS Firmware Update State | PRODUCTION | 9030 | SLOW |  |
| input | 106 |  | uint16 | 1.0 | cycles | bms_cycle_count | BMS Cycle Count | PRODUCTION | 9031 | SLOW |  |
| input | 107 |  | uint16 | 0.1 | V | inverter_battery_voltage_sample | Inverter Battery Voltage Sample | PRODUCTION | 9032 | NORMAL |  |
| input | 114 |  | uint16 | 1.0 | W | ongrid_load_power | On-Grid Load Power | PRODUCTION | 9049 | NORMAL |  |
| input | 120 |  | uint16 | 0.1 | V | half_bus_voltage | Half Bus Voltage | PRODUCTION | 9052 | NORMAL |  |
| input | 121 |  | uint16 | 0.1 | V | generator_voltage | Generator Voltage | PRODUCTION | 9073 | NORMAL |  |
| input | 122 |  | uint16 | 0.01 | Hz | generator_frequency | Generator Frequency | PRODUCTION | 9074 | NORMAL |  |
| input | 123 |  | uint16 | 1.0 | W | generator_power | Generator Power | PRODUCTION | 9075 | NORMAL |  |
| input | 124 |  | uint16 | 0.1 | kWh | generator_energy_today | Generator Energy Today | PRODUCTION | 9076 | SLOW |  |
| input | 170 |  | uint16 | 1.0 | W | load_power | Load Power | PRODUCTION | 9085 | FAST |  |
| holding | 21 | 15 | bit | 1.0 |  | zero_export_enabled | Zero Export | PRODUCTION | 7003 | NORMAL | Stage 36 write + read-back on LXP1 |
| holding | 22 | 15 | bit | 1.0 |  | feed_in_grid_enabled | Feed-In Grid Enabled | UNDER_REVIEW | 7001 | NORMAL | holding 22 read; bit 15 interpretation; current inverter state (Stage 35: under review) |
| holding | 103 |  | uint16 | 1.0 | % | max_backflow_power | Max Backflow Power | PRODUCTION | 7002 | NORMAL | Stage 36 write + read-back on LXP1 |
| holding | 168 |  | uint16 | 1.0 | A | ac_charge_current_from_grid | AC Charge Current | UNDER_REVIEW | 7004 | NORMAL | ESP-030 read on LXP1 and LXP2 (10 A, matches the app) |

86 registers (82 input, 4 holding).
