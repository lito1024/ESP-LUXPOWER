# Verified LuxPower Register Database

This document records the only LuxPower register mappings currently accepted by the project. The purpose of this database is to keep a single source of truth for verified metadata without introducing speculative mappings.

| Register ID | Name | Register Space | Address | Type | Bit | Scale | Unit | Verification Source | Verification Status |
| --- | --- | --- | ---: | --- | ---: | ---: | --- | --- | --- |
| Pv1EnergyTotal | PV1 Energy Total | Input | 40 | U32 | 0 | 0.1 | kWh | verified by current LuxPower logs; verified by previous working parser; verified against Home Assistant value | Verified |
| FeedInGridEnabled | Feed-In Grid Enabled | Holding | 22 | Bit | 15 | 1.0 |  | verified by holding register 22 read; verified bit 15 interpretation; verified against current inverter state | Verified |
| Pv1Power | PV1 Power | Input | 7 | U16 | 0 | 1.0 | W | upstream luxpower-ha-integration; Stage 12B real-inverter comparison; matched corresponding Home Assistant entity | Verified |
| Pv2Power | PV2 Power | Input | 8 | U16 | 0 | 1.0 | W | upstream luxpower-ha-integration; Stage 12B real-inverter comparison; matched corresponding Home Assistant entity | Verified |
| BatterySoc | Battery SOC | Input | 5 | U16 | 0 | 1.0 | % | upstream luxpower-ha-integration; Stage 12B real-inverter comparison; matched corresponding Home Assistant entity | Verified |
| BatteryChargePower | Battery Charge Power | Input | 10 | U16 | 0 | 1.0 | W | upstream luxpower-ha-integration; Stage 12B real-inverter comparison; matched corresponding Home Assistant entity | Verified |
| BatteryDischargePower | Battery Discharge Power | Input | 11 | U16 | 0 | 1.0 | W | upstream luxpower-ha-integration; Stage 12B real-inverter comparison; matched corresponding Home Assistant entity | Verified |
