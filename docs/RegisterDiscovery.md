# Register Discovery Workflow

Developer register scanning is intended for experimental cache analysis only. It must not be treated as verification by itself.

## Capture a baseline
1. Start the firmware and wait until both input and holding caches are valid.
2. Let the developer scanner capture its first baseline.
3. Do not record first-baseline output as a register change.

## Change one condition at a time
Change only one physical inverter or system condition before each scan window. Examples include changing PV load, toggling a known inverter setting from the official UI, or waiting for a single observable Home Assistant value to move.

Avoid testing multiple conditions at once because correlated register changes are not enough to identify a mapping.

## Compare with Home Assistant
Compare changed register values against the matching Home Assistant entity at the same time. Record the Home Assistant value, timestamp, raw register address, raw value, and observed delta.

## Record candidates
For every candidate, record:
- register space;
- register address;
- candidate type;
- signedness;
- scale;
- unit;
- physical condition tested;
- independent comparison source.

## Verification rule
Do not add a candidate to `LuxPowerRegisters.h` until it has been independently confirmed. Scanner output is a discovery aid, not a verified mapping source.
