# LuxPower TCP ESPHome External Component

This component wraps the verified read-only `LuxPowerTCP` library for ESPHome.
It publishes only high-level verified metrics and does not expose raw register
numbers or write operations.

Tested target: ESPHome 2026.6.5 with ESP32-S3 and ESP-IDF.

## YAML

Use the external component from this repository and add the same repository as a
PlatformIO library dependency. Pin both to the same commit or release tag.

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/YOUR_ACCOUNT/ESP-LUXPOWER.git
      ref: 8eb75ab
    components: [luxpower_tcp]

esphome:
  libraries:
    - LuxPowerTCP=https://github.com/YOUR_ACCOUNT/ESP-LUXPOWER.git#8eb75ab

luxpower_tcp:
  id: luxpower
  host: !secret luxpower_host
  port: !secret luxpower_port
  dongle_serial: !secret luxpower_dongle_serial
  inverter_serial: !secret luxpower_inverter_serial
  input_poll_interval: 1s
  holding_poll_interval: 30s
```

Supported sensors:

- `pv1_power`
- `pv2_power`
- `battery_soc`
- `battery_charge_power`
- `battery_discharge_power`
- `pv1_energy_total`

Supported binary sensors:

- `feed_in_grid_enabled`
- `tcp_connected`
