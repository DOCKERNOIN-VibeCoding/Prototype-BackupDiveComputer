
# Prototype-BackupDiveComputer

Prototype backup dive computer project based on ESP32-S3, Wokwi simulation, PlatformIO, and OLED UI.


# Warning
## This project is experimental and must not be used as a primary, secondary, or backup dive computer for real diving.

### Do not use this project for actual dive safety decisions.
It is not certified, validated, or tested for real diving use.
It must not be used as a primary, secondary, or backup dive computer.
The decompression calculations, NDL, ascent warnings, safety stop logic, and no-fly calculations may be incomplete or incorrect.
Use only for software development and simulation.

Main Features in v7.1
ESP32-S3 / Wokwi simulation
OLED 128x64 UI prototype
Scenario-based simulated depth profile
Surface screen prototype
Battery low popup logic
Charging indicator behavior
GF setting mock command
Manual depth and profile control commands
Ascent-rate graph with fixed risk-zone dot pattern
Development Direction for v7.2
Compact internal dive log structure
Subsurface XML export/import compatibility
Scenario input based on real dive-log structure
Surface screen based on previous dive start time and duration
BLE log download planning
Build
This project uses PlatformIO.

Example :
```text
pio run -e wokwi
```

Wokwi Serial Commands :
```text
battery 9
battery 50
battery 100

charge on
charge off

gf 35 75

depth 20
depth 0

profile auto
profile pause
profile resume
```

## Hardware Prototype

- [Hardware Prototype Specification](docs/HARDWARE_PROTOTYPE.md)
- [Hardware Prototype JSON Definition](docs/HARDWARE_PROTOTYPE.json)


## Status

Current development snapshot:

```text
v1.3.5-dev
```

Development branch:

```text
dev/v1.3
```

## Current Development Priority

The current v1.3 development priority is focused on:

```text
1. Continuous dive handling
2. PostDive holding window
3. Final log close at PostDive → Surface transition
4. GPS/BLE automatic power control
5. GPS/BLE top-bar status indicators
6. RTS-based dive log time correction
7. bootCount and boot elapsed based log recovery
```

Continuous dive policy:

```text
Depth <0.5m for 60 seconds:
  Dive → PostDive

PostDive duration:
  3 minutes

Total short surfacing grace:
  About 4 minutes

Re-entry during PostDive:
  Continuous dive, same log

Re-entry after Surface mode:
  Repetitive dive, new log, tissue loading retained
```

Time correction policy:

```text
If real time is available during the dive:
  log timeStatus = TimeSynced

If real time is not available:
  log timeStatus = RelativeOnly
  bootCount and boot elapsed time are stored

When GPS/BLE time is later obtained:
  logs from the same bootCount and timeSessionId can be corrected
  corrected logs become TimeCorrected
```

## This version is a development snapshot focused on:

- Air / EAN21 single-gas baseline
- Nitrox-ready FO2 configuration structure
- Bühlmann ZH-L16 based tissue tracking
- DECO.STOP ladder: 18m / 15m / 12m / 9m / 6m / 3m
- CEIL >18m warning state
- No hard lockout after missed decompression
- 48h post-violation advisory
- DECO.VIOL display on SURFACE mode
- Safety Stop and DECO.STOP separation


## Important Safety Policy

This prototype does not lock out the device after a missed decompression stop.

If the diver surfaces with an active decompression obligation, the device shows a strong `MISSED DECO` advisory, starts a 48-hour post-violation advisory, and continues to provide dive information.

This behavior is intentional for a backup dive computer:
the device should continue to provide depth, time, tissue-state based decompression information, and surface data even after a violation.

This does not mean that re-entry is recommended.
It only means that if the diver re-enters, the device continues calculations instead of withholding information.


## Decompression Model Notice

This prototype uses a decompression calculation concept based on the Bühlmann ZH-L16 model with Gradient Factors.

The Bühlmann model is a mathematical decompression model. This project does not claim medical, scientific, or dive safety validation.

The current implementation is for simulation, UI prototyping, and software development only.

Do not use this project for real diving decisions.


## License

No open-source license has been selected yet.

All rights reserved by the author during the prototype phase.

## Third-Party Code and Algorithms

This project may reference decompression concepts such as the Bühlmann ZH-L16 model and Gradient Factors.

The mathematical model and decompression theory are referenced for educational and prototyping purposes.

If any third-party source code is used or ported in the future, the corresponding license and attribution will be documented in this repository.

At this stage, this project should not be assumed to be compatible with or derived from any specific certified dive computer implementation.



All rights reserved by the author during the prototype phase.
```
