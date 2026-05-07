

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

This repository contains an early-stage prototype.

Current baseline version:

```text
BackupDiveComputer_v7.1
```

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

