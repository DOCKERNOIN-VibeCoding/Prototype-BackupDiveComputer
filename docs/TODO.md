# TODO - BackUpDiveAssistant

> Current branch: `dev/v1.3.7`  
> Firmware version: `v1.3.7-dev`  
> Native log format version: `3`  
> Last updated: 2026-05-15  
> Status: Experimental prototype / Not for real diving

---

## 0. Safety Notice

BackUpDiveAssistant is an experimental backup dive computer prototype.

```text
This firmware must not be used for real diving.
All decompression, alarm, logging, BLE, and UI behavior must be verified
against real dive-computer requirements before any practical use.
```

Development priorities:

```text
1. Safety-critical behavior correctness
2. Predictable UI behavior
3. Reliable native log persistence
4. Minimal unnecessary alarms
5. Wokwi / simulation reproducibility
6. Future BLE raw log transfer
7. Future Subsurface-compatible export
```

---

## 1. Current Completed Baseline

This section summarizes what is considered implemented as of `v1.3.7-dev`.

### 1.1 Core firmware / versioning

- [x] Development branch moved to `dev/v1.3.7`
- [x] Firmware version set to `v1.3.7-dev`
- [x] Native BDC log format version bumped to `3`
- [x] Wokwi / PlatformIO simulation environment maintained
- [x] `generated_scenario.h` auto-generation flow maintained
- [x] ESP32-S3 / Wokwi simulation target maintained
- [x] Experimental safety notice maintained

---

### 1.2 Gas / Bühlmann / Nitrox-ready structure

- [x] Default gas defined as Air / EAN21
- [x] `DIVE_GAS_FO2_PERCENT` added
- [x] `DIVE_GAS_PPO2_MAX_BAR` added
- [x] `DIVE_GAS_FO2_MIN_PERCENT` added
- [x] `DIVE_GAS_FO2_MAX_PERCENT` added
- [x] FO2 compile-time range check added
- [x] Bühlmann FN2 hardcoding removed
- [x] FN2 calculated from FO2
- [x] Tissue nitrogen loading uses gas FN2
- [x] NDL calculation uses gas FN2
- [x] DECO calculation uses gas FN2
- [x] GF99 calculation uses gas FN2
- [x] No-Fly calculation uses gas FN2
- [x] MOD helper added
- [x] ppO2 helper added

---

### 1.3 DECO.STOP ladder

- [x] DECO.STOP ladder implemented
  - [x] 18m
  - [x] 15m
  - [x] 12m
  - [x] 9m
  - [x] 6m
  - [x] 3m
- [x] Raw decompression ceiling calculated
- [x] Raw ceiling mapped to ladder stop depth
- [x] Raw ceiling deeper than 18m handled as special warning state
- [x] `CEIL >18m` warning state added
- [x] `DECO.STOP 0m` display avoided
- [x] Current required DECO.STOP only displayed
- [x] Full DecoPlan display intentionally not implemented

---

### 1.4 DECO.STOP margin / action UI

- [x] Old symmetric `DECO_STOP_WINDOW_M` policy replaced
- [x] Shallow margin policy added
- [x] Deep margin policy added
- [x] DECO stop HOLD range defined using asymmetric margins
- [x] `DESCEND` shown when too shallow
- [x] `HOLD` shown inside valid stop window
- [x] `ASCEND` shown when deeper than target stop
- [x] Timer runs inside valid HOLD window
- [x] Timer pauses outside valid HOLD window
- [x] At 3.7m during 3m DECO stop, UI shows `HOLD`, not `ASCEND`

---

### 1.5 DECO timer prediction / v1.3.7 safety-critical fix

- [x] DECO timer changed from minute-based to second-based active countdown
- [x] DECO timer rounded up to 20-second units
- [x] Timer rounding uses upward rounding, not nearest rounding
- [x] Repeated `0:20 -> 0:00 -> 0:20` timer reset issue fixed
- [x] Current stop timer now predicts full required stop duration
- [x] Timer prediction uses copied tissue state, not live tissue mutation
- [x] Timer prediction simulates future off-gassing in 20-second steps
- [x] Timer prediction uses GF Low based stop transition criterion
- [x] Timer prediction uses conservative calculation depth:
  - [x] `effectiveCalcDepthM = stopDepthM + DECO_STOP_DEEP_MARGIN_M`
- [x] Displayed stop depth remains nominal ladder depth
- [x] Displayed rounded timer is enforced as minimum stop duration
- [x] Model transition alone does not end the stop before displayed timer reaches zero
- [x] Stop transition requires:
  - [x] model allows shallower stop or DECO clear
  - [x] displayed local countdown reached zero
- [x] Premature DECO stop transition before displayed timer completion fixed
- [x] `buhlmann.cpp` repetitive calculation Serial log removed/commented
- [x] `app.cpp` logs only state transition style DECO messages
- [x] Resync log intentionally kept visible as diagnostic signal

---

### 1.6 DECO / Safety Stop priority

- [x] DECO.STOP has priority over Safety Stop
- [x] Firmware must not switch from DECO.STOP to Safety Stop while DECO obligation remains
- [x] Safety Stop does not override DECO.STOP
- [x] No forced separate Safety Stop after DECO completion
- [x] Safety Stop and DECO.STOP are treated as separate concepts

---

### 1.7 Safety Stop

- [x] Safety Stop trigger depth configured
- [x] Safety Stop zone configured
- [x] Safety Stop duration configured
- [x] Safety Stop started state implemented
- [x] Safety Stop paused state implemented
- [x] Safety Stop completed state implemented
- [x] Safety Stop skipped flow implemented
- [x] Shallow-out skip grace window implemented
- [x] Deep-out cancellation / resumed deeper policy implemented
- [x] S.STOP SKIPPED display implemented

---

### 1.8 DECO violation / missed DECO / advisory

- [x] `activeDecoViolation_` state added
- [x] `postViolationAdvisory_` state added
- [x] `postViolationAdvisoryEndEpochSec_` added
- [x] `reentryCount_` added
- [x] `clearedAfterReentry_` added
- [x] Missed DECO surfacing detection implemented
- [x] DECO violation does not trigger hard lockout
- [x] 48-hour post-violation advisory policy implemented
- [x] Re-entry after missed DECO keeps tissue loading
- [x] Re-entry after missed DECO continues DECO calculation
- [x] Re-entry increments re-entry count
- [x] Re-entry event logging implemented
- [x] DECO cleared after re-entry event implemented
- [x] Advisory can remain even after active violation is cleared

---

### 1.9 Native BDC log format / persistence

- [x] `include/log_format.h` added
- [x] `BDC_LOG_MAGIC` defined
- [x] `BDC_LOG_VERSION` defined as `3`
- [x] `LogTimeStatus` enum defined
- [x] `DiveLogHeader` defined
- [x] `DiveSample` defined
- [x] `DiveEvent` defined
- [x] Native log structure defined as:
  - [x] `DiveLogHeader`
  - [x] `DiveSample[]`
  - [x] `DiveEvent[]`
- [x] `DiveSample.timeSec` uses `uint32_t`
- [x] `DiveEvent.value` uses `int32_t`
- [x] Sample capacity defined
- [x] Event capacity defined
- [x] Sample buffer added to runtime
- [x] Event buffer added to runtime
- [x] `recordDiveSample()` implemented
- [x] `logDiveEvent()` / event recording extended to buffer
- [x] `sampleCount` reflected in header
- [x] `eventCount` reflected in header
- [x] Header + samples + events save path implemented
- [x] Header-only RTS update path preserves samples/events
- [x] RAM fallback mirrors full log structure
- [x] Stack overflow due to large runtime reset fixed by avoiding temporary `DiveRuntime()`

---

### 1.10 Persisted / buffered event coverage

- [x] `EVENT_DECO_REQUIRED`
- [x] `EVENT_DECO_STOP_STARTED`
- [x] `EVENT_DECO_STOP_COMPLETED`
- [x] `EVENT_DECO_CLEARED`
- [x] `EVENT_DECO_MISSED`
- [x] `EVENT_DECO_VIOLATION_SURFACED`
- [x] `EVENT_DECO_REENTRY`
- [x] `EVENT_DECO_CLEARED_AFTER_REENTRY`
- [x] `EVENT_POST_VIOLATION_ADVISORY_STARTED`
- [x] `EVENT_POST_VIOLATION_ADVISORY_ENDED`
- [x] `EVENT_CEILING_EXCEEDED`
- [x] `EVENT_CEIL_GT_18M`
- [x] `EVENT_SAFETY_STOP_STARTED`
- [x] `EVENT_SAFETY_STOP_COMPLETED`
- [x] `EVENT_SAFETY_STOP_SKIPPED`
- [x] `EVENT_SAFETY_STOP_CANCELLED_DEEP`
- [x] `EVENT_ASCENT_RATE_WARN`
- [x] `EVENT_LOW_BATTERY`

---

### 1.11 Time / RTS / GPS correction

- [x] `bootCount` stored
- [x] `timeSessionId` stored
- [x] `bootElapsedStartSec` stored
- [x] `bootElapsedEndSec` stored
- [x] RelativeOnly log support implemented
- [x] TimeSynced log support implemented
- [x] TimeCorrected log support implemented
- [x] GPS RTS acquisition implemented
- [x] RelativeOnly log time correction using GPS RTS implemented
- [x] bootCount mismatch prevents correction
- [x] timeSessionId mismatch prevents correction
- [x] Header-only update preserves sample/event payload

---

### 1.12 GPS / BLE / Charging policy

- [x] GPS/BLE top-bar status indicator implemented
- [x] GPS/BLE mock power control implemented
- [x] GPS limited surface search policy implemented
- [x] GPS retry policy implemented
- [x] Qi / Charging GPS search behavior implemented
- [x] Qi / Charging BLE advertising behavior implemented
- [x] BLE access window after Charging removal implemented
- [x] BLE stays active while connected
- [x] BLE turns off when entering Dive state
- [x] GPS turns off when entering Dive state

---

### 1.13 Alarm policy

- [x] Battery low beep implemented
- [x] Fast ascent triple warning implemented
- [x] Ceiling violation alarm implemented
- [x] MISSED DECO negative alert implemented
- [x] NDL 10-second pre-warning explicitly rejected
- [x] DECO entry uses one long alarm
- [x] DECO entry alarm may occur again if DECO clears and new DECO obligation appears later
- [x] Non-critical alarm minimization policy adopted

---

### 1.14 UI baseline

- [x] Surface screen implemented
- [x] Dive normal screen implemented
- [x] Safety Stop screen implemented
- [x] DECO.STOP screen implemented
- [x] MISSED DECO alert screen implemented
- [x] Battery low popup implemented
- [x] Charging indicator implemented
- [x] GPS/BLE status display implemented
- [x] Unicode arrow dependency avoided
- [x] Triangle icon drawing used for action indication

---

## 2. Immediate Verification Checklist

These items should be verified before considering `v1.3.7-dev` stable enough for a tag.

### 2.1 Build / boot

- [ ] `pio run -e wokwi` clean build succeeds
- [ ] Wokwi simulation boots without stack overflow
- [ ] Boot log shows `v1.3.7-dev`
- [ ] Boot log shows expected `BDC_LOG_VERSION = 3` if printed
- [ ] No unexpected watchdog reset
- [ ] No loop task stack overflow
- [ ] I2C initialization warning does not affect simulation behavior
- [ ] `git status` clean after build artifacts are ignored

---

### 2.2 Native log persistence verification

- [ ] Completed dive saves `DiveLogHeader + DiveSample[] + DiveEvent[]`
- [ ] `header.version == 3`
- [ ] `header.sampleCount > 0` after a dive
- [ ] `header.eventCount > 0` after a dive with events
- [ ] `sampleCount` matches stored sample array count
- [ ] `eventCount` matches stored event array count
- [ ] Log reload after reboot preserves header
- [ ] Log reload after reboot preserves key metadata
- [ ] RTS/GPS header-only correction preserves sample payload
- [ ] RTS/GPS header-only correction preserves event payload
- [ ] RAM fallback full-log behavior verified in Wokwi
- [ ] LittleFS full-log behavior verified on real ESP32-S3 later

---

### 2.3 DECO timer verification

- [ ] DECO timer does not repeat `0:20 -> 0:00 -> 0:20`
- [ ] 12m stop requiring more than 20 seconds displays full predicted duration
- [ ] 9m stop requiring more than 20 seconds displays full predicted duration
- [ ] 6m stop requiring more than 20 seconds displays full predicted duration
- [ ] 3m stop requiring more than 20 seconds displays full predicted duration
- [ ] Timer rounds up to 20-second units
- [ ] Example: required `1:01` displays/enforces `1:20`
- [ ] Displayed timer does not end early before local countdown reaches zero
- [ ] Model allowing shallower transition does not override remaining displayed timer
- [ ] Stop transition occurs only after displayed timer reaches zero
- [ ] Stop transition occurs only when model allows next stop or clear
- [ ] DECO clear does not occur before displayed timer reaches zero
- [ ] Timer pauses outside valid stop window
- [ ] Timer resumes inside valid stop window
- [ ] Resync does not repeatedly occur at same stop depth
- [ ] Any Resync log is investigated manually

---

### 2.4 DECO ladder verification

- [ ] Minimal DECO: `3m -> clear`
- [ ] Light DECO: `6m -> 3m -> clear`
- [ ] Moderate DECO: `9m -> 6m -> 3m -> clear`
- [ ] Heavier DECO: `12m -> 9m -> 6m -> 3m -> clear`
- [ ] Heavy DECO: `18m -> 15m -> 12m -> 9m -> 6m -> 3m -> clear`
- [ ] Raw ceiling deeper than 18m shows `CEIL >18m`
- [ ] `DECO.STOP 0m` is never displayed
- [ ] Current stop started event recorded
- [ ] Current stop completed event recorded
- [ ] Next stop started event recorded
- [ ] Final stop completed event recorded
- [ ] DECO cleared event recorded

---

### 2.5 DECO / Safety Stop priority verification

- [ ] DECO.STOP overrides Safety Stop
- [ ] Safety Stop does not appear during active DECO obligation
- [ ] No forced Safety Stop after DECO clears
- [ ] DECO clear returns to Normal phase correctly
- [ ] If diver later qualifies for Safety Stop separately, behavior is reviewed against policy

---

### 2.6 DECO alarm verification

- [ ] DECO entry emits one long alarm
- [ ] DECO entry does not emit repeated short triple beep
- [ ] DECO stop transitions do not emit unnecessary alarms
- [ ] DECO clear does not emit excessive alarm
- [ ] If DECO clears and a new DECO obligation appears later, long alarm occurs again
- [ ] NDL 10-second pre-warning does not exist

---

### 2.7 DECO violation / missed DECO verification

- [ ] Surfacing with incomplete DECO triggers missed DECO flow
- [ ] `EVENT_DECO_MISSED` recorded
- [ ] `EVENT_DECO_VIOLATION_SURFACED` recorded
- [ ] `EVENT_POST_VIOLATION_ADVISORY_STARTED` recorded
- [ ] 48-hour advisory starts
- [ ] Surface UI shows `DECO.VIOL`
- [ ] Surface UI alternates `DECO.VIOL` / `N-FLY` if both active
- [ ] No hard lockout occurs
- [ ] Re-entry during advisory continues decompression calculation
- [ ] Re-entry records `EVENT_DECO_REENTRY`
- [ ] DECO clear after re-entry records `EVENT_DECO_CLEARED_AFTER_REENTRY`
- [ ] Active violation clears after required DECO is completed
- [ ] Advisory remains until expiry
- [ ] Advisory ended event is recorded when applicable

---

### 2.8 Safety Stop verification

- [ ] Safety Stop starts after qualifying dive
- [ ] Safety Stop timer counts down
- [ ] Safety Stop pauses when out of range
- [ ] Safety Stop resumes when back in range
- [ ] Safety Stop completes normally
- [ ] Safety Stop skipped flow works
- [ ] Safety Stop skipped event recorded
- [ ] Safety Stop cancelled deep event recorded
- [ ] Safety Stop missed/skipped is not treated as DECO violation
- [ ] S.STOP SKIPPED display duration verified

---

### 2.9 Continuous / repetitive dive verification

- [ ] Surfacing shorter than continuous-dive threshold keeps same dive
- [ ] PostDive re-entry within grace window resumes continuous dive
- [ ] PostDive re-entry does not create duplicate log
- [ ] PostDive final close occurs only after grace window expires
- [ ] Surface re-entry after final close creates repetitive dive
- [ ] Repetitive dive retains tissue loading
- [ ] Repetitive dive increments dive count correctly
- [ ] Continuous dive sample logging remains continuous
- [ ] Continuous dive event logging remains continuous

---

### 2.10 UI verification

- [ ] Surface screen displays current date/time
- [ ] Surface screen displays battery status
- [ ] Surface screen displays GPS/BLE status
- [ ] Surface screen displays LAST / MAX / TMP
- [ ] Surface screen displays SURFACE interval
- [ ] Surface screen displays N-FLY remaining
- [ ] SAFE TO FLY appears after no-fly expires
- [ ] Dive normal screen displays NDL
- [ ] Dive normal screen displays TTS where applicable
- [ ] Safety Stop screen displays correct timer
- [ ] DECO.STOP screen displays correct stop depth
- [ ] DECO.STOP screen displays correct remaining seconds
- [ ] DECO.STOP screen displays HOLD / ASCEND / DESCEND correctly
- [ ] CEIL >18m warning is visible
- [ ] Battery low popup does not hide critical DECO/ceiling warnings for too long
- [ ] 128x64 OLED text does not overflow critical labels

---

## 3. High Priority TODO

These are the next major tasks after v1.3.7 verification.

### 3.1 Regression scenario set

- [ ] Add scenario for minimal 3m DECO
- [ ] Add scenario for 6m -> 3m DECO
- [ ] Add scenario for 9m -> 6m -> 3m DECO
- [ ] Add scenario for 12m -> 9m -> 6m -> 3m DECO
- [ ] Add scenario for 18m ladder DECO
- [ ] Add scenario for raw ceiling >18m
- [ ] Add scenario for repeated-depth stop stability
- [ ] Add scenario for current/oscillation around stop window
- [ ] Add scenario for timer rounding case
- [ ] Add scenario for premature-transition prevention
- [ ] Add scenario for missed DECO surfacing
- [ ] Add scenario for missed DECO re-entry
- [ ] Add scenario for Safety Stop normal completion
- [ ] Add scenario for Safety Stop skipped
- [ ] Add scenario for Safety Stop cancelled by deeper re-descent

---

### 3.2 Raw log inspection / debug tooling

- [ ] Add mock command to dump last log header
- [ ] Add mock command to dump sample count
- [ ] Add mock command to dump event count
- [ ] Add mock command to print DiveEvent list
- [ ] Add mock command to print first/last N samples
- [ ] Add log validation helper for magic/version/counts
- [ ] Add truncated/corrupt log detection
- [ ] Add optional CRC or footer policy discussion
- [ ] Add desktop script to parse `.bdc`
- [ ] Add desktop script to print `.bdc` summary

Suggested command names:

```text
log info
log events
log samples
log dump
log validate
```

---

### 3.3 UI priority cleanup

- [ ] Define centralized UI priority decision path
- [ ] Confirm priority order:
  - [ ] Critical hardware/system warning
  - [ ] PPO2 HIGH / MOD EXCEEDED
  - [ ] Ceiling violation
  - [ ] CEIL >18m
  - [ ] DECO.STOP
  - [ ] Missed DECO advisory
  - [ ] Safety Stop
  - [ ] Ascent-rate warning
  - [ ] NDL / normal dive
  - [ ] Surface / PostDive / Charging
- [ ] Ensure Battery Low popup does not obscure critical warning too long
- [ ] Define warning rotation policy when multiple warnings active
- [ ] Define persistent vs transient alert display durations
- [ ] Verify UI behavior on 128x64 OLED

---

### 3.4 Missed DECO / advisory persistence hardening

- [ ] Verify `activeDecoViolation_` restoration after reboot
- [ ] Verify `postViolationAdvisory_` restoration after reboot
- [ ] Verify `postViolationAdvisoryEndEpochSec_` restoration after reboot
- [ ] Verify re-entry count restoration after reboot
- [ ] Verify missed stop depth/remain restoration after reboot
- [ ] Decide whether NVS mirror is needed beyond last log
- [ ] Decide how advisory behaves if last log is cleared
- [ ] Add documentation warning that re-entry after missed DECO is not recommended
- [ ] Confirm no hard lockout policy remains documented

---

## 4. Medium Priority TODO

### 4.1 EAN32 / Nitrox scenarios

- [ ] Add EAN32 scenario support
- [ ] Add scenario gas override design
- [ ] Add generated scenario constants for gas override
- [ ] Verify FO2 = 32%
- [ ] Verify FN2 = 0.68
- [ ] Verify EAN32 MOD around 33.7m at ppO2 1.4
- [ ] Verify EAN32 NDL longer than EAN21 for same profile
- [ ] Verify EAN32 DECO behavior differs appropriately from EAN21
- [ ] Decide display label:
  - [ ] `AIR`
  - [ ] `EAN21`
  - [ ] `EAN32`
- [ ] Store runtime/configured gas in log
- [ ] Prepare for future BLE gas setting

Suggested scenario structure:

```json
{
  "gas": {
    "fo2Percent": 32,
    "ppO2MaxBar": 1.4
  }
}
```

---

### 4.2 MOD / ppO2 warning

- [ ] Decide MOD display policy for FO2 = 21%
- [ ] Decide MOD display policy for FO2 > 21%
- [ ] Add MOD exceeded detection
- [ ] Add ppO2 high detection
- [ ] Add warning text:
  - [ ] `PPO2 HIGH`
  - [ ] `MOD EXCEEDED`
  - [ ] `ASCEND`
- [ ] Add MOD exceeded event
- [ ] Add ppO2 high event
- [ ] Define alarm policy for MOD exceeded
- [ ] Define UI priority relative to DECO.STOP
- [ ] Add EAN32 MOD exceeded scenario
- [ ] Add EAN36 MOD exceeded scenario
- [ ] Verify warning does not hide DECO ceiling violation

---

### 4.3 BLE raw native log transfer

- [ ] Define BLE service UUIDs
- [ ] Define device info characteristic
- [ ] Define battery characteristic
- [ ] Define log list characteristic
- [ ] Define log metadata characteristic
- [ ] Define log download characteristic
- [ ] Define log transfer control characteristic
- [ ] Define chunk size
- [ ] Define offset-based read protocol
- [ ] Define transfer cancel behavior
- [ ] Define transfer checksum or length verification
- [ ] Define BLE connection power policy during transfer
- [ ] Ensure BLE is off during Dive state
- [ ] Ensure BLE access after Charging / Qi wake
- [ ] Implement raw `.bdc` transfer before XML transfer

---

### 4.4 Subsurface export tooling

- [ ] Create `tools/bdc_to_subsurface_xml.py`
- [ ] Parse `DiveLogHeader`
- [ ] Parse `DiveSample[]`
- [ ] Parse `DiveEvent[]`
- [ ] Export dive date/time
- [ ] Export duration
- [ ] Export max depth
- [ ] Export average depth
- [ ] Export temperature
- [ ] Export gas FO2
- [ ] Export sample time/depth/temp
- [ ] Export events into XML notes first
- [ ] Later investigate Subsurface profile event tags
- [ ] Add UDDF export option later
- [ ] Add CSV export fallback
- [ ] Test import into Subsurface
- [ ] Document known Subsurface limitations

---

### 4.5 XML / scenario parser

- [ ] Add optional Subsurface XML input parser
- [ ] Parse dive date/time
- [ ] Parse sample time
- [ ] Parse sample depth
- [ ] Parse sample temperature
- [ ] Parse gas FO2 where available
- [ ] Generate scenario JSON from XML
- [ ] Generate `generated_scenario.h` from parsed XML
- [ ] Validate generated profile in Wokwi
- [ ] Keep XML parser as development tool, not firmware feature

---

## 5. Low Priority / Future Work

### 5.1 Multi-log storage

- [ ] Replace single `/last_dive.bdc` with multi-log storage
- [ ] Define log filename scheme
- [ ] Define log index structure
- [ ] Define max log count
- [ ] Define storage rollover policy
- [ ] Define delete policy
- [ ] Define BLE log list mapping
- [ ] Define corrupted log recovery behavior
- [ ] Define storage full behavior

Possible file naming:

```text
/logs/dive_0001.bdc
/logs/dive_0002.bdc
/logs/index.bin
```

---

### 5.2 Runtime gas configuration

- [ ] Add NVS / Preferences storage for FO2
- [ ] Add BLE gas setting characteristic
- [ ] Add validation for FO2 21-40%
- [ ] Add setting lockout during active dive
- [ ] Add gas setting to boot log
- [ ] Add gas setting to dive log header
- [ ] Add gas label to UI
- [ ] Add gas setting export to Subsurface XML

---

### 5.3 Power management

- [ ] Define Light Sleep policy
- [ ] Define Deep Sleep policy
- [ ] Add `BDC_DISABLE_DEEP_SLEEP` config flag if needed
- [ ] Ensure tissue/advisory/time continuity is preserved
- [ ] Define wake sources
- [ ] Define Qi wake behavior
- [ ] Define charging wake behavior
- [ ] Define BLE transfer wake behavior
- [ ] Measure real hardware current
- [ ] Measure GPS off current
- [ ] Measure LCD off current
- [ ] Measure BLE advertising current
- [ ] Decide whether Deep Sleep is acceptable for v1.x
- [ ] Keep Deep Sleep disabled until continuity risks are resolved

---

### 5.4 Hardware prototype validation

- [ ] Confirm ST7567A LCD pinout
- [ ] Confirm LCD voltage
- [ ] Confirm LCD contrast control
- [ ] Confirm MS5837 I2C address
- [ ] Confirm MS5837 pull-up voltage
- [ ] Confirm MS5837 pressure range
- [ ] Confirm GPS module baud rate
- [ ] Confirm GPS cold-start behavior
- [ ] Confirm GPS antenna placement
- [ ] Confirm Qi charging board behavior
- [ ] Confirm charging termination
- [ ] Confirm power-path behavior
- [ ] Confirm protected 16340 fit
- [ ] Confirm actual battery capacity
- [ ] Confirm buzzer audibility inside waterproof housing
- [ ] Confirm waterproof enclosure constraints
- [ ] Confirm no-button wake strategy

---

## 6. Wokwi Scenario Test Matrix

### 6.1 Surface / PostDive

- [ ] Previous dive preload
- [ ] LAST date display
- [ ] MAX display
- [ ] TMP display
- [ ] SURFACE interval increases
- [ ] N-FLY decreases
- [ ] SAFE TO FLY appears
- [ ] GPS status display
- [ ] BLE advertising display
- [ ] BLE connected display
- [ ] Charging icon display
- [ ] FULL display
- [ ] Qi removal access window behavior

---

### 6.2 Normal dive / NDL

- [ ] NDL decreases during dive
- [ ] NDL reaches 0
- [ ] DECO obligation begins
- [ ] DECO entry long alarm occurs
- [ ] NDL 10-second pre-warning does not occur
- [ ] Normal TTS display remains plausible
- [ ] Ascent-rate graph/dot behavior remains stable

---

### 6.3 DECO.STOP margin tests

For each stop depth, verify shallow / hold / deep behavior.

#### 9m stop

- [ ] 8.3m -> DESCEND / timer pause
- [ ] 9.0m -> HOLD / timer run
- [ ] 10.2m -> ASCEND / timer run
- [ ] 11.0m -> ASCEND / timer pause

#### 6m stop

- [ ] 5.3m -> DESCEND / timer pause
- [ ] 6.0m -> HOLD / timer run
- [ ] 7.2m -> ASCEND / timer run
- [ ] 8.0m -> ASCEND / timer pause

#### 3m stop

- [ ] 2.3m -> DESCEND / timer pause
- [ ] 3.0m -> HOLD / timer run
- [ ] 3.7m -> HOLD / timer run
- [ ] 4.8m -> HOLD or boundary behavior as intended
- [ ] 5.0m -> ASCEND / timer pause

---

### 6.4 DECO timer rounding tests

- [ ] Required 0:20 -> displayed 0:20
- [ ] Required 0:21 -> displayed 0:40
- [ ] Required 1:00 -> displayed 1:00
- [ ] Required 1:01 -> displayed 1:20
- [ ] Required 1:20 -> displayed 1:20
- [ ] Required 1:21 -> displayed 1:40
- [ ] Displayed rounded time is enforced
- [ ] No transition before displayed countdown reaches zero

---

### 6.5 DECO violation tests

- [ ] Leave stop shallower than ceiling
- [ ] Ceiling violation event recorded
- [ ] Ceiling violation alarm repeats at expected interval
- [ ] Surface before completing DECO
- [ ] Missed DECO event recorded
- [ ] Post violation advisory started
- [ ] Surface advisory displayed
- [ ] Re-entry after violation
- [ ] DECO calculation continues
- [ ] Active violation clears after completing DECO
- [ ] Advisory remains after active violation clears

---

### 6.6 Log tests

- [ ] Dive with no special events saves samples
- [ ] Dive with Safety Stop saves safety events
- [ ] Dive with DECO saves DECO events
- [ ] Dive with ceiling violation saves violation events
- [ ] Dive with missed DECO saves missed/advisory events
- [ ] Re-entry saves re-entry events
- [ ] Log header reloads after reboot
- [ ] Time correction updates only header
- [ ] Samples/events survive time correction
- [ ] Overflow warning appears if sample buffer fills
- [ ] Overflow warning appears if event buffer fills

---

## 7. Documentation TODO

### 7.1 Must update for v1.3.7

- [ ] Rewrite `docs/DEVELOPMENT_v1.3.md` for v1.3.7 architecture/policy
- [x] Rewrite `docs/TODO.md` for v1.3.7 current status
- [ ] Update `docs/CHANGELOG.md` with DECO timer prediction changes
- [ ] Update `docs/LOG_FORMAT.md` status from v1.3.6-dev to v1.3.7-dev
- [ ] Update `docs/LOG_FORMAT.md` with BDC log version 3
- [ ] Update `docs/LOG_FORMAT.md` with full stop-duration prediction policy
- [ ] Update `docs/LOG_FORMAT.md` with conservative calculation depth policy
- [ ] Update `docs/LOG_FORMAT.md` with rounded timer enforcement policy
- [ ] Update `docs/LOG_FORMAT.md` with Resync diagnostic policy
- [ ] Update `docs/CHAT_CONTEXT_v1.3.md` with latest v1.3.7 decisions
- [ ] Update README if root README is stale
- [ ] Remove stale `v1.3.5-dev` references where no longer historical
- [ ] Remove stale `dev/v1.3.6` references where no longer historical
- [ ] Remove stale `dev/v7.x` references where no longer historical
- [ ] Check Markdown code block formatting

---

### 7.2 Policies that must be documented clearly

- [ ] Native `.bdc` log is source of truth
- [ ] XML/UDDF/CSV are future derived exports
- [ ] BLE should transfer raw native log first
- [ ] DECO.STOP always overrides Safety Stop
- [ ] No forced Safety Stop after DECO
- [ ] DECO timer predicts full stop duration
- [ ] DECO timer uses copied tissue state for prediction
- [ ] DECO timer uses `stopDepth + deep margin` calculation depth
- [ ] Displayed rounded timer is enforced
- [ ] Resync log is diagnostic and should be rare
- [ ] NDL 10-second pre-warning is intentionally not implemented
- [ ] DECO entry uses one long alarm
- [ ] DECO entry alarm repeats if DECO clears and later re-enters
- [ ] Missed DECO does not hard lockout device
- [ ] Re-entry after missed DECO continues calculation but is not recommended

---

## 8. Release Checklist for v1.3.7

Before tagging `v1.3.7`, complete the following.

### 8.1 Code / build

- [ ] `pio run -e wokwi` clean build passes
- [ ] Wokwi boot succeeds
- [ ] No stack overflow
- [ ] No unexpected reboot
- [ ] No compile warnings requiring immediate action
- [ ] Version string is final, not `-dev`, if release tag is created
- [ ] `BDC_LOG_VERSION` confirmed as `3`

---

### 8.2 Simulation

- [ ] Normal dive scenario passes
- [ ] Safety Stop scenario passes
- [ ] DECO scenario passes
- [ ] DECO timer full-duration scenario passes
- [ ] DECO timer rounding scenario passes
- [ ] DECO violation scenario passes
- [ ] Re-entry scenario passes
- [ ] Continuous dive scenario passes
- [ ] Repetitive dive scenario passes
- [ ] RTS correction scenario passes
- [ ] Battery low scenario passes
- [ ] Charging / BLE / GPS scenario passes

---

### 8.3 Logs

- [ ] Header saved
- [ ] Samples saved
- [ ] Events saved
- [ ] Header reload verified
- [ ] Event count verified
- [ ] Sample count verified
- [ ] RTS correction preserves payload
- [ ] Native log parsing strategy documented
- [ ] Existing logs with older format handled safely or explicitly unsupported

---

### 8.4 Documentation

- [ ] `CHANGELOG.md` updated
- [ ] `LOG_FORMAT.md` updated
- [ ] `DEVELOPMENT_v1.3.md` rewritten
- [ ] `TODO.md` rewritten
- [ ] `CHAT_CONTEXT_v1.3.md` updated
- [ ] README checked
- [ ] Safety notice visible
- [ ] Release notes prepared

---

### 8.5 Git

- [ ] Working tree clean
- [ ] No `.pio` files committed
- [ ] No temporary logs committed
- [ ] No generated junk files committed
- [ ] Commit messages clear
- [ ] Push to `dev/v1.3.7`
- [ ] Optional: create `v1.3.7` tag only after verification
- [ ] Optional: keep `v1.3.7-dev` until simulation checklist passes

---

## 9. Deferred / Non-goals for Current Stage

The following are intentionally not required for current firmware stage.

```text
Direct on-device Subsurface XML generation
Direct on-device UDDF generation
Direct upload into Subsurface via libdivecomputer protocol
Cloud synchronization
Multi-dive logbook management
Runtime gas switching during dive
Multi-gas decompression
Trimix
Compass
Real hardware waterproof validation
Real diving use
```

---

## 10. Current Next Recommended Work

Recommended order from this point:

```text
1. Rewrite docs/DEVELOPMENT_v1.3.md
2. Update docs/CHANGELOG.md
3. Update docs/LOG_FORMAT.md
4. Update docs/CHAT_CONTEXT_v1.3.md
5. Run full Wokwi v1.3.7 regression checklist
6. Add raw log dump / validation command
7. Add DECO regression scenarios
8. Start UI priority cleanup
9. Start BLE raw log transfer design
10. Start Subsurface XML export tool
```

---
```
