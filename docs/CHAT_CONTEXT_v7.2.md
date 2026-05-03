# Chat Context - BackupDiveComputer_v7.2

Project repository:

```text
Prototype-BackupDiveComputer

Current uploaded baseline:
BackupDiveComputer_v7.1

Next target:
BackupDiveComputer_v7.2


Key development decisions:

Internal dive logs should be stored compactly, not as XML.
Subsurface XML will be used for export/import compatibility.
Scenario input may support Subsurface XML in the future.
Surface screen shows LAST, MAX, TMP, SURFACE, N-FLY.
LAST displays only YYYY-MM-DD.
Surface interval is calculated from lastDiveStartEpoch + duration.
GF and battery are not scenario-controlled.
Battery low popup appears at 10% or below.
Battery low popup interval: 2 minutes.
Battery low popup duration: 2 seconds.
Charging icon blinks while charging.
Full charge alternates FULL and battery icon.
Ascent graph dot pattern is a fixed risk-zone background.
Surface UI should keep the current large-font, dense, readable layout.



---

## 5.2 `docs/CHANGELOG.md`

```markdown
# CHANGELOG

## v7.1

Baseline prototype before v7.2 refactoring.

Included:
- Scenario JSON based simulation
- Surface screen prototype
- Battery low popup logic
- Charging indicator behavior
- GF setting through mock command
- Manual depth command
- Profile auto/pause/resume command
- Ascent graph fixed risk-zone dot pattern concept

## v7.2 Planned

- Redefine Surface screen data model
- Use LAST instead of LAST DIVE
- LAST value shows YYYY-MM-DD only
- Calculate Surface interval from last dive start epoch and duration
- Remove direct surfaceIntervalSec preload concept
- Add compact dive log design
- Plan Subsurface XML export/import
- Add XML scenario parser later
