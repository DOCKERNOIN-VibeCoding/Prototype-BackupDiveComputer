# CHANGELOG

## v7.1-baseline

Initial public baseline for Prototype-BackupDiveComputer.

Included:
- ESP32-S3 / Wokwi simulation
- OLED 128x64 UI prototype
- Scenario JSON based simulated dive profile
- Surface screen prototype
- Battery low popup logic
- Charging indicator behavior
- GF setting mock command
- Manual depth and profile control commands
- Ascent-rate graph with fixed risk-zone dot pattern

## v7.2 Planned

- Redefine Surface screen data model
- Use LAST instead of LAST DIVE
- Display LAST as YYYY-MM-DD only
- Calculate Surface interval from last dive start time and duration
- Remove direct surfaceIntervalSec preload concept
- Add compact internal dive log design
- Plan Subsurface XML export/import
- Add XML scenario parser later
