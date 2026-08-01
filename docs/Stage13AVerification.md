# Stage 13A Verification

Stage 13A is a temporary cache-only comparison pass for the second upstream candidate group. These mappings are not verified yet and must not be added to `LuxPowerRegisters.h` until the candidate values are compared against Home Assistant on the real inverter.

Use the serial output headed by:

```text
===== STAGE 13A CANDIDATE METRICS =====
```

Record the printed candidate value, the matching Home Assistant value, the difference, and the result.

| Metric | Candidate value | Home Assistant entity | HA value | Difference | Result |
| --- | --- | --- | --- | --- | --- |
| Battery Capacity | Pending | `sensor.lxp_battery_capacity` | Pending | Pending | Pending |
| BMS Max Cell Temperature | Pending | `sensor.lxp_bms_max_cell_temperature` | Pending | Pending | Pending |
| Power To Grid | Pending | `sensor.lxp_power_to_grid` | Pending | Pending | Pending |
| Power From Grid | Pending | `sensor.lxp_power_from_grid` | Pending | Pending | Pending |
| Grid Flow | Pending | `sensor.lxp_grid_flow` | Pending | Pending | Pending |

## Candidate Register Sources

| Metric | Input register | Raw extraction | Candidate display |
| --- | ---: | --- | --- |
| Battery Capacity | 97 | `cachedInputU16(97)` | raw value in Ah |
| BMS Max Cell Temperature | 103 | `cachedInputU16(103)` | raw value * 0.1 in deg C |
| Power To Grid | 26 | `cachedInputU16(26)` | raw value in W |
| Power From Grid | 27 | `cachedInputU16(27)` | raw value in W |
| Grid Flow | 26, 27 | `cachedInputU16(27) - cachedInputU16(26)` | positive = import, negative = export |

## Signed Temperature Caveat

`docs/UpstreamMetricCandidates.md` records the upstream caveat for BMS Max Cell Temperature: the upstream register constant describes the value as signed, but the upstream sensor implementation scales the raw U16 value without sign conversion. Stage 13A preserves that behavior for comparison.

## Verification Rule

Leave each result as `Pending` until the value has been compared live against Home Assistant. Use `Match` only when the candidate value and Home Assistant value agree within the expected display precision. Use `Mismatch` when the values do not agree or the sign/scale appears wrong.
