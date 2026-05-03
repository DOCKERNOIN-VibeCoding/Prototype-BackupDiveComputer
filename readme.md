
Warning
This project is experimental and must not be used as a primary, secondary, or backup dive computer for real diving.

Do not use this project for actual dive safety decisions.

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
pio run -e wokwi

Wokwi Serial Commands :
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



# Prototype-BackupDiveComputer

Prototype backup dive computer project based on ESP32-S3, Wokwi simulation, PlatformIO, and OLED UI.


## Status

This repository contains an early-stage prototype.

Current baseline version:

```text
BackupDiveComputer_v7.1


License
No open-source license has been selected yet.

All rights reserved by the author during the prototype phase.

