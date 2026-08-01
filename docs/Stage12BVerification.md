# Stage 12B Verification

Stage 12B was a temporary cache-only comparison pass. The five candidate metrics below were compared against Home Assistant on the real inverter and promoted in Stage 12C.

Use the serial output headed by:

```text
===== STAGE 12B CANDIDATE METRICS =====
```

Record the printed candidate value, the matching Home Assistant value, the difference, and the result.

| Metric | Candidate value | Home Assistant entity | HA value | Difference | Result |
| --- | --- | --- | --- | --- | --- |
| PV1 Power | 2276 W | `sensor.lxp_pv1_power` | 2276 W | 0 W | Match |
| PV2 Power | 1739 W | `sensor.lxp_pv2_power` | 1739 W | 0 W | Match |
| Battery SOC | 100 % | `sensor.lxp_battery_soc` | 100 % | 0 % | Match |
| Battery Charge Power | 0 W | `sensor.lxp_battery_charge_power` | 0 W | 0 W | Match |
| Battery Discharge Power | 0 W | `sensor.lxp_battery_discharge_power` | 0 W | 0 W | Match |

## Candidate Register Sources

| Metric | Input register | Raw extraction | Candidate display |
| --- | ---: | --- | --- |
| PV1 Power | 7 | `cachedInputU16(7)` | raw value in W |
| PV2 Power | 8 | `cachedInputU16(8)` | raw value in W |
| Battery SOC | 5 | `cachedInputU16(5) & 0xFF` | low byte in % |
| Battery Charge Power | 10 | `cachedInputU16(10)` | raw value in W |
| Battery Discharge Power | 11 | `cachedInputU16(11)` | raw value in W |

## Verification Rule

These five rows are now `Match` and were promoted to verified register descriptors in Stage 12C.
