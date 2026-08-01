# ESPHome Integration

Stage: 15B-15D

This repository now contains both:

- the reusable read-only `LuxPowerTCP` protocol library;
- a lightweight ESPHome external component wrapper in `components/luxpower_tcp`.

The library remains the source of truth for verified registers, cache handling,
CRC, packet parsing, polling intervals, reconnect behavior and high-level
cache-only metric APIs.

## Tested Source Baseline

Library source baseline before Stage 15B:

```text
8eb75abb33f22f19187c86ebb0d97f5858876f3a
```

ESPHome hardware/API research baseline:

```text
ESPHome 2026.4.5
commit 09dc41435c34ebcfe7ba3928bd7148a8aad1d310
```

Local validation baseline:

```text
ESPHome 2026.6.5
ESP32-S3
ESP-IDF 5.5.4
```

Official source evidence used:

- `esphome/components/socket/socket.h`: socket factories,
  `socket::set_sockaddr`, `Socket::setblocking`.
- `esphome/components/async_tcp/async_tcp_socket.cpp`: official example of
  `socket_loop_monitored`, `setblocking(false)`, `read`, `write` and
  `ready()`.
- `esphome/components/external_components/__init__.py`: git external
  components are loaded from a repository `components/` or `esphome/components/`
  directory.
- `esphome/core/config.py` and `esphome/cpp_generator.py`: `esphome.libraries`
  supports PlatformIO library dependencies and git repository refs.

## Framework Compatibility

Supported:

- Existing PlatformIO application: Arduino framework, `esp32s3`.
- ESPHome wrapper target: ESP32-S3 with ESP-IDF, using ESPHome socket APIs.

Stage 15B introduced a small transport abstraction:

- `LuxPowerTransport`: framework-neutral socket interface.
- `LuxPowerArduinoTransport`: adapter for Arduino `Client`/`WiFiClient`.
- `LuxPowerESPHomeTransport`: adapter in the ESPHome component using
  `esphome::socket`.

The existing Arduino API remains available:

```cpp
WiFiClient client;
LuxPowerTCP luxPower(client);
luxPower.begin(ip, port, dongleSerial, inverterSerial);
```

Framework-neutral use is also available:

```cpp
LuxPowerTCP luxPower(transport);
luxPower.begin("192.0.2.10", 8000, dongleSerial, inverterSerial);
```

## Recommended ESPHome Consumption

Use the external component and the PlatformIO git library dependency pinned to
the same commit or release tag.

Because this local checkout has no configured `git remote`, replace
`YOUR_ACCOUNT` with the actual GitHub owner before using the example.

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
```

The `external_components` entry downloads the YAML/codegen wrapper. The
`esphome.libraries` entry downloads and builds the reusable `LuxPowerTCP`
library from the same repository.

For local validation before publishing a GitHub release, the included
`examples/esphome-basic.yaml` uses the local component and local library path:

```yaml
external_components:
  - source:
      type: local
      path: ../components
    components: [luxpower_tcp]

esphome:
  libraries:
    - LuxPowerTCP=file://../../../../lib/LuxPowerTCP
```

## Component YAML

```yaml
luxpower_tcp:
  id: luxpower
  host: !secret luxpower_host
  port: !secret luxpower_port
  dongle_serial: !secret luxpower_dongle_serial
  inverter_serial: !secret luxpower_inverter_serial
  input_poll_interval: 1s
  holding_poll_interval: 30s
```

Sensors:

```yaml
sensor:
  - platform: luxpower_tcp
    luxpower_id: luxpower
    pv1_power:
      name: "LuxPower PV1 Power"
    pv2_power:
      name: "LuxPower PV2 Power"
    battery_soc:
      name: "LuxPower Battery SOC"
    battery_charge_power:
      name: "LuxPower Battery Charge Power"
    battery_discharge_power:
      name: "LuxPower Battery Discharge Power"
    pv1_energy_total:
      name: "LuxPower PV1 Energy Total"
```

Binary sensors:

```yaml
binary_sensor:
  - platform: luxpower_tcp
    luxpower_id: luxpower
    feed_in_grid_enabled:
      name: "LuxPower Feed-In Grid Enabled"
    tcp_connected:
      name: "LuxPower TCP Connected"
```

## Wrapper Responsibilities

`LuxPowerTCPComponent`:

- owns the ESPHome socket transport;
- owns a `LuxPowerTCP` instance;
- configures host, port, serials and polling intervals from YAML;
- calls `LuxPowerTCP::poll()` from `loop()`;
- publishes sensors about once per second;
- preserves last valid values during temporary communication failures by not
  publishing invalid cache reads;
- exposes no raw register numbers;
- implements no register writes.

Published verified metrics:

- `pv1_power`
- `pv2_power`
- `battery_soc`
- `battery_charge_power`
- `battery_discharge_power`
- `pv1_energy_total`
- `feed_in_grid_enabled`
- `tcp_connected`

## Limitations And Risks

- The underlying protocol refresh is still synchronous, preserving current
  behavior. If the inverter is unavailable, a `poll()` call can wait for the
  existing protocol timeout. A future Stage 15C can split refresh into a fully
  asynchronous state machine if LVGL responsiveness requires it.
- Host values are expected to be numeric IPv4/IPv6 addresses supported by
  `socket::set_sockaddr`.
- No display or LVGL YAML was implemented in Stage 15B.

## Update Strategy

1. Verify or promote metrics in `LuxPowerTCP`.
2. Build the Arduino PlatformIO project.
3. Build/validate an ESPHome node that uses the wrapper.
4. Tag a GitHub release.
5. Pin both `external_components.ref` and `esphome.libraries` to the same tag.

Recommended first tag after Stage 15B:

```text
v0.2.0-esphome
```
