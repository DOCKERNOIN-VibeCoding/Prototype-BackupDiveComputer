# TODO - BackupDiveComputer_v7.2

## Phase 1 - Stabilize v7.1 baseline

- [ ] Confirm Wokwi build
- [ ] Confirm Serial Monitor commands
- [ ] Confirm battery low popup
- [ ] Confirm charging indicator
- [ ] Confirm Surface screen readability

## Phase 2 - Surface data model

- [ ] Add lastDiveStartEpochSec_
- [ ] Add lastDiveDurationSec_
- [ ] Add lastDiveEndEpochSec_
- [ ] Add lastDiveMaxDepthM_
- [ ] Add lastDiveMinTempC_
- [ ] Add noFlyEndEpochSec_
- [ ] Calculate surface interval from epoch values

## Phase 3 - Scenario refactor

- [ ] Remove direct surfaceIntervalSec usage
- [ ] Use lastDive.startEpoch and durationSec
- [ ] Keep GF out of scenario
- [ ] Keep battery out of scenario

## Phase 4 - Subsurface XML planning

- [ ] Add XML scenario parser to generate_scenario.py
- [ ] Parse dive date/time/duration
- [ ] Parse sample time/depth/temp
- [ ] Generate scenario points from XML

## Phase 5 - Compact log

- [ ] Add include/log_format.h
- [ ] Define DiveLogHeader
- [ ] Define DiveSample
- [ ] Define DiveEvent
- [ ] Plan XML export tool

