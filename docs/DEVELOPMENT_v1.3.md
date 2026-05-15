# BackUpDiveAssistant v1.3 Development Notes

> Current branch: `dev/v1.3.7`  
> Firmware version: `v1.3.7-dev`  
> Native log format version: `3`  
> Last updated: 2026-05-15  
> Status: Experimental prototype / Not for real diving

---

## 0. Safety Notice

BackUpDiveAssistant is an experimental backup dive assistant / backup dive computer prototype.

```text
This project must not be used for real diving.
All decompression, alarm, UI, logging, BLE, and hardware behavior must be
verified against appropriate dive-computer safety requirements before any
real-world use.
```

The current firmware is intended for:

```text
1. Algorithm prototyping
2. UI behavior validation
3. Wokwi simulation
4. Native log format development
5. Future BLE / export workflow design
```

It is not intended for:

```text
1. Actual decompression planning
2. Real diving
3. Life-support decision making
4. Replacing a certified dive computer
```

---

## 1. Project Scope

BackUpDiveAssistant is designed as a small, buttonless, standalone backup dive assistant.

The intended product concept is:

```text
Primary role:
    Backup display and logger for recreational diving information

Primary user assumption:
    The diver already has a main dive computer

Design preference:
    Conservative, simple, low-interaction, low-alarm behavior
```

The current scope is:

```text
1. Single gas recreational diving support
2. Air / EAN21 default gas
3. Nitrox-ready internal structure
4. Bühlmann ZH-L16 style tissue model
5. Gradient Factor support
6. NDL display
7. DECO.STOP display
8. Safety Stop display
9. Missed DECO advisory
10. Native binary dive log
11. Wokwi simulation
12. Future BLE raw log transfer
13. Future Subsurface-compatible export
```

Out of current scope:

```text
1. Multi-gas decompression
2. Trimix
3. CCR
4. Compass
5. Certified real-world diving use
6. On-device XML generation
7. Direct Subsurface/libdivecomputer protocol compatibility
```

---

## 2. Naming Policy

The project name is:

```text
BackUpDiveAssistant
```

Older documents or code comments may still contain historical references to:

```text
BackupDiveComputer
```

Those references should be gradually updated unless they are intentionally preserved as historical notes.

Preferred terminology:

```text
Project name:        BackUpDiveAssistant
Native log format:   BDC native log / .bdc
Firmware role:       backup dive assistant
```

The native log magic/version naming may remain `BDC_*` for compatibility unless a future log format migration is intentionally planned.

---

## 3. Current Development Baseline

Current development baseline:

```text
Branch:              dev/v1.3.7
Firmware version:    v1.3.7-dev
Native log version:  3
Primary simulator:   Wokwi
Target MCU:          ESP32-S3
Display prototype:   128x64 OLED in Wokwi
Storage prototype:   LittleFS / RAM fallback
```

Major completed work in `v1.3.7-dev`:

```text
1. Native log format upgraded to v3
2. DiveSample persistence implemented
3. DiveEvent persistence implemented
4. Header + samples + events save path implemented
5. RTS/GPS header-only correction preserves samples/events
6. DECO timer full-duration prediction implemented
7. Conservative DECO timer calculation depth implemented
8. Rounded DECO timer enforcement implemented
9. DECO transition logging cleaned up
10. DECO entry long alarm policy implemented
```

---

## 4. Hardware / Simulation Target

Current development target:

```text
ESP32-S3
Wokwi simulation
OLED 128x64 display prototype
Mock depth / temperature sensor
Mock battery
Mock GPS
Mock BLE state
Mock buzzer
LittleFS or RAM fallback log storage
```

Future hardware direction:

```text
MCU:             ESP32-S3 class
Depth sensor:    MS5837 class pressure sensor
Display:         ST7567A or similar low-power LCD
Battery:         Protected 16340 Li-ion candidate
Charging:        Qi wireless charging candidate
Connectivity:    BLE
GPS:             Optional surface/post-dive location/time source
Buttons:         Ideally none
```

Hardware constraints that influence firmware design:

```text
1. Small screen
2. No buttons
3. Limited power budget
4. Limited storage
5. Limited RAM
6. BLE should be disabled during diving
7. GPS should be disabled during diving
8. Logging must be robust against power loss
9. UI must be simple and glanceable
```

---

## 5. Runtime State Model

The firmware uses a high-level system state model.

Expected system states:

```text
Surface
Dive
PostDive
Charging
```

### 5.1 Surface

Surface mode is used when the device is not diving.

Responsibilities:

```text
1. Show current date/time
2. Show battery state
3. Show GPS/BLE status
4. Show last dive summary
5. Show surface interval
6. Show no-fly time
7. Show DECO violation advisory if active
8. Allow GPS limited search
9. Allow BLE access depending on policy
10. Detect dive start
```

### 5.2 Dive

Dive mode is active after confirmed descent.

Responsibilities:

```text
1. Read depth and temperature
2. Update tissue loading
3. Calculate NDL / DECO
4. Display normal dive / Safety Stop / DECO.STOP
5. Record samples
6. Record events
7. Detect violations
8. Detect surfacing
9. Disable BLE/GPS
```

### 5.3 PostDive

PostDive is a tentative surfacing state.

Purpose:

```text
1. Avoid prematurely closing a dive log
2. Allow short re-entry to continue same dive
3. Show post-dive information
4. Finalize log only after grace period expires
```

Policy:

```text
Dive -> PostDive:
    tentative end

PostDive -> Dive:
    continuous dive resumed

PostDive -> Surface:
    final log close
```

### 5.4 Charging

Charging state is entered when Qi / charging condition is detected.

Responsibilities:

```text
1. Show charging UI
2. Allow BLE advertising/access window
3. Try GPS if needed
4. Keep user-visible wake behavior predictable
```

---

## 6. Dive Phase Model

Inside `SystemState::Dive`, the dive phase is separated from the system state.

Expected dive phases:

```text
Normal
SafetyStop
Deco
```

### 6.1 Normal

Normal phase displays:

```text
Depth
Dive time
NDL
TTS estimate
Max depth
Temperature
Ascent-rate indicator
Battery
```

Normal phase may transition to:

```text
SafetyStop
Deco
PostDive / Surface detection path
```

### 6.2 SafetyStop

Safety Stop is a recreational stop helper.

It is not decompression.

Safety Stop may be:

```text
Started
Paused
Completed
Skipped
Cancelled by deeper re-descent
```

### 6.3 Deco

Deco phase is active when Bühlmann model indicates a decompression obligation.

Deco phase displays:

```text
DECO.STOP depth
Remaining stop time
TTS
HOLD / ASCEND / DESCEND instruction
Ceiling-related warnings
```

Deco phase always has priority over Safety Stop.

---

## 7. Gas Policy

Current gas model:

```text
Single gas
Default gas: Air / EAN21
FO2 range: 21% to 40%
ppO2 max default: 1.40 bar
```

Important definitions:

```text
Air = EAN21
EAN32 is not Air
FO2 is oxygen fraction percentage
FN2 is calculated from FO2
```

Current default:

```cpp
DIVE_GAS_FO2_PERCENT = 21
DIVE_GAS_PPO2_MAX_BAR = 1.40f
```

Policy:

```text
1. FN2 must not be hardcoded.
2. Bühlmann calculations must use FN2 derived from FO2.
3. Gas setting is currently compile-time.
4. Future BLE/app runtime gas setting may be added.
5. Gas used for a dive must be recorded in the native log.
```

---

## 8. Bühlmann / Gradient Factor Policy

The firmware uses a Bühlmann-style tissue model with 16 nitrogen compartments.

Current model characteristics:

```text
ZH-L16 style N2 half-times
N2 a/b coefficients
Single inert gas: nitrogen
FO2-derived FN2
Gradient Factor support
```

Default GF policy:

```text
GF Low:   40
GF High:  85
```

Current recommendation:

```text
Keep default GF as 40/85.
Do not raise GF Low merely to compensate for conservative timer display.
```

Rationale:

```text
1. GF settings are decompression conservatism settings.
2. Timer calculation policy is a display/enforcement policy.
3. Changing GF to compensate for timer policy creates confusing behavior.
4. Future user presets may support 50/85, but default should remain conservative.
```

Possible future presets:

```text
Conservative: 40/85
Balanced:     50/85
User custom:  configurable later
```

---

## 9. DECO.STOP Ladder Policy

Supported DECO.STOP ladder:

```text
18m -> 15m -> 12m -> 9m -> 6m -> 3m
```

The firmware displays only the currently required stop.

It does not display the full future decompression plan.

Policy:

```text
1. Raw ceiling is calculated from tissue state.
2. Raw ceiling is mapped to the next appropriate ladder stop.
3. Only one active DECO.STOP is shown.
4. `DECO.STOP 0m` must never be shown.
5. If raw ceiling is deeper than max supported stop, show `CEIL >18m`.
```

Final stop depth policy:

```text
DECO_LAST_STOP_DEPTH_M may be configured as 3 or 6.
```

Expected ladders:

```text
If final stop is 3m:
    18 -> 15 -> 12 -> 9 -> 6 -> 3 -> surface

If final stop is 6m:
    18 -> 15 -> 12 -> 9 -> 6 -> surface
```

---

## 10. DECO.STOP UI Action Policy

DECO.STOP UI uses action text:

```text
DESCEND
HOLD
ASCEND
```

Unicode arrows are avoided.

Reason:

```text
Future LCD hardware may not support font glyphs for Unicode arrows.
Use ASCII text and manually drawn triangle icons instead.
```

Stop margin policy:

```text
Shallow margin: 0.6m
Deep margin:    1.8m
```

For a 3m stop:

```text
depth < 2.4m       -> DESCEND / timer pause
2.4m to 4.8m       -> HOLD or valid stop window / timer may run
depth > 4.8m       -> ASCEND / timer pause
```

Important behavior:

```text
At 3.7m during a 3m DECO.STOP:
    show HOLD
    allow timer to continue
```

This prevents misleading `ASCEND` display while the diver is still inside the valid stop window.

---

## 11. DECO Timer Prediction Policy

This is a safety-critical v1.3.7 policy.

### 11.1 Problem fixed in v1.3.7

Previous behavior could show repeated short timer fragments:

```text
DECO.STOP 12m 0:20
countdown reaches 0:00
model still requires 12m
timer resets to 0:20
repeats multiple times
```

This was unacceptable because the user should see the expected total stop duration from the beginning.

Correct behavior:

```text
If the current stop requires 60 seconds:
    show 1:00

If the current stop requires 80 seconds:
    show 1:20

Do not show repeated 0:20 fragments.
```

---

### 11.2 Full stop-duration prediction

DECO timer must predict the full required duration for the current stop.

Policy:

```text
1. Copy current tissue state.
2. Simulate future off-gassing on the copy.
3. Do not mutate live tissue state during prediction.
4. Simulate in 20-second steps.
5. Continue until the model allows next shallower stop or DECO clear.
6. Round up to the next 20-second interval.
7. Display that rounded value as the stop timer.
```

The timer represents:

```text
Predicted time required at current stop before transition is allowed.
```

It does not represent:

```text
A short polling interval.
A minimum 20-second fragment.
A UI-only countdown detached from the model.
```

---

### 11.3 Conservative calculation depth

Displayed stop depth remains the nominal ladder depth.

Example:

```text
Displayed: DECO.STOP 12m
```

Timer prediction uses conservative calculation depth:

```text
effectiveCalcDepthM = stopDepthM + DECO_STOP_DEEP_MARGIN_M
```

Examples:

```text
18m stop -> calculate at 19.8m
15m stop -> calculate at 16.8m
12m stop -> calculate at 13.8m
9m stop  -> calculate at 10.8m
6m stop  -> calculate at 7.8m
3m stop  -> calculate at 4.8m
```

Rationale:

```text
1. Avoid timer jitter from depth oscillation.
2. Avoid using instantaneous depth that may fluctuate due to current or buoyancy.
3. Keep UI stable.
4. Preserve conservative behavior for a backup assistant.
```

---

### 11.4 20-second rounding policy

DECO timer is rounded upward to 20-second units.

Formula:

```cpp
roundedSec = ((requiredSec + 20 - 1) / 20) * 20;
```

Examples:

```text
required 0:20 -> display 0:20
required 0:21 -> display 0:40
required 1:00 -> display 1:00
required 1:01 -> display 1:20
required 1:20 -> display 1:20
required 1:21 -> display 1:40
```

The rounding is intentionally conservative.

---

### 11.5 Displayed rounded timer is authoritative

Once a rounded timer is displayed, it becomes the minimum enforced stop duration.

Important rule:

```text
The firmware must not transition to the next stop or clear DECO
before the displayed local countdown reaches zero.
```

Stop transition requires both:

```text
1. Model allows next shallower stop or DECO clear
2. Displayed local countdown has reached zero
```

Example:

```text
Model-required time: 1:01
Displayed timer:     1:20

Correct behavior:
    stay for 1:20

Incorrect behavior:
    move to next stop at 1:01
```

This preserves the conservative rounding policy.

---

### 11.6 Resync policy

Resync exists only as a diagnostic safety net.

Normal operation should not require repeated Resync.

Acceptable:

```text
One rare Resync due to timing/model mismatch
```

Suspicious:

```text
Repeated Resync at the same stop depth
Repeated 0:20 reset behavior
Large model remain restored after countdown reached zero
```

The `Resync` Serial log is intentionally kept visible.

Reason:

```text
If Resync occurs repeatedly, it indicates a condition that should be reviewed.
```

---

## 12. DECO Stop Event Policy

DECO events are safety-critical and must be persisted.

Important rule:

```text
DECO_STOP_COMPLETED must not be logged merely because the countdown reached zero.
```

Correct completion condition:

```text
1. Displayed timer has completed
2. Model allows transition to shallower stop or DECO clear
```

Expected event flow:

```text
EVENT_DECO_REQUIRED
EVENT_DECO_STOP_STARTED
EVENT_DECO_STOP_COMPLETED
EVENT_DECO_STOP_STARTED
EVENT_DECO_STOP_COMPLETED
EVENT_DECO_CLEARED
```

For re-entry after violation:

```text
EVENT_DECO_REENTRY
EVENT_DECO_STOP_STARTED
EVENT_DECO_STOP_COMPLETED
EVENT_DECO_CLEARED_AFTER_REENTRY
```

Serial log policy:

```text
buhlmann.cpp:
    no repetitive calculation logs

app.cpp:
    log only meaningful state transitions
```

Recommended Serial events:

```text
[DECO] Required ...
[DECO] Stop started ...
[DECO] Stop completed ...
[DECO] Stop changed deeper ...
[DECO] Cleared ...
[DECO] Resync ...       diagnostic only
```

---

## 13. Safety Stop Policy

Safety Stop is separate from DECO.STOP.

Safety Stop policy:

```text
1. Trigger after qualifying depth exposure.
2. Run in configured Safety Stop zone.
3. Pause outside zone.
4. Complete after required time.
5. Can be skipped if diver remains too shallow.
6. Can be cancelled if diver resumes deeper diving.
```

Safety Stop must never override DECO.STOP.

Important rule:

```text
If DECO obligation exists:
    show DECO.STOP
    do not switch to Safety Stop
```

After DECO clears:

```text
Do not force a separate mandatory Safety Stop.
```

Rationale:

```text
1. DECO stop supersedes recreational Safety Stop.
2. UI should not confuse users with another mandatory stop after DECO.
3. Backup assistant should avoid unnecessary additional alarm/stop prompts.
```

---

## 14. DECO Violation / Missed DECO Policy

BackUpDiveAssistant does not use hard lockout.

Policy:

```text
1. If diver surfaces before completing required DECO, set active violation.
2. Start 48-hour post-violation advisory.
3. Continue showing useful information.
4. Do not lock the device.
5. If diver re-enters, continue tissue-based DECO calculation.
6. If required DECO is later completed, clear active violation.
7. Advisory may remain until its expiry.
```

Important warning:

```text
Re-entry after missed DECO is not recommended.
The firmware provides information but does not imply safety.
```

Surface UI during advisory:

```text
DECO.VIOL
N-FLY
remaining advisory time
```

Re-entry behavior:

```text
1. No hard lockout
2. Tissue state retained
3. DECO.STOP recalculated
4. Re-entry count increments
5. Re-entry event recorded
6. Cleared-after-reentry event recorded if completed
```

---

## 15. Alarm Policy

BackUpDiveAssistant should minimize unnecessary alarms.

General alarm philosophy:

```text
1. Avoid nuisance alarms.
2. Alert only meaningful safety transitions.
3. Keep alarm patterns simple.
4. Do not repeatedly alarm for non-critical conditions.
```

Current alarm policy:

```text
Battery low:
    short low-priority beep

Fast ascent:
    triple warning beep

Ceiling violation:
    repeated warning at controlled interval

Missed DECO:
    negative alert pattern

DECO entry:
    one long alarm

NDL 10-second warning:
    intentionally not implemented
```

DECO entry alarm policy:

```text
When DECO obligation first appears:
    emit one long alarm

If DECO clears and a new DECO obligation appears later:
    emit one long alarm again
```

Reason:

```text
A new DECO obligation is an important state transition even if it occurs
more than once during the same dive.
```

The rejected policy:

```text
NDL <= 10 seconds:
    alarm 3 times
```

Reason for rejection:

```text
1. Backup assistant should minimize alarms.
2. Main dive computer likely already handles NDL warning.
3. DECO entry itself is the more meaningful transition.
```

---

## 16. Native Log Persistence Policy

Native log format version:

```text
BDC_LOG_VERSION = 3
```

Native file structure:

```text
[ DiveLogHeader ]
[ DiveSample sampleCount entries ]
[ DiveEvent  eventCount entries ]
```

Native log is the source of truth.

Policy:

```text
1. Store compact native binary log on device.
2. Do not store Subsurface XML directly on device.
3. BLE should transfer raw native `.bdc` logs first.
4. XML/UDDF/CSV should be generated externally later.
```

Rationale:

```text
1. Native binary format is smaller.
2. Native binary format is easier to write safely.
3. XML generation is memory-heavy.
4. XML compatibility may change.
5. Native logs can be re-exported into multiple formats later.
```

---

## 17. DiveLogHeader Policy

`DiveLogHeader` stores dive-level metadata.

Important fields include:

```text
magic
version
headerSize
diveNumber
timeStatus
timeSessionId
bootCount
bootElapsedStartSec
bootElapsedEndSec
startEpochSec
endEpochSec
durationSec
noFlyEndEpochSec
gasFo2Percent
ppO2MaxCentiBar
decoViolation
postViolationAdvisory
reentryCount
missedStopDepthCm
missedStopRemainSec
advisoryEndEpochSec
maxDepthCm
avgDepthCm
minTempDeciC
sampleCount
eventCount
gpsLatE7
gpsLonE7
gpsValid
```

Required compatibility fields:

```text
magic
version
headerSize
sampleCount
eventCount
```

Import/export tools must validate:

```text
1. magic
2. version
3. header size
4. sample count
5. event count
6. file length
```

---

## 18. DiveSample Policy

`DiveSample` records time-series profile data.

Current expected structure:

```cpp
struct DiveSample {
    uint32_t timeSec;
    int16_t depthCm;
    int16_t tempDeciC;
    int16_t ndlOrTtsMin;
};
```

Units:

```text
timeSec:
    seconds since dive start

depthCm:
    depth in centimeters

tempDeciC:
    temperature in deci-degrees Celsius

ndlOrTtsMin:
    NDL minutes in Normal phase
    TTS minutes in Deco phase
```

Policy:

```text
1. Export tools must use timeSec.
2. Export tools must not assume a fixed sample interval.
3. Future versions may add ascent rate, ppO2, ceiling, stop depth, or GF99.
4. Any binary layout change requires log version increment.
```

---

## 19. DiveEvent Policy

`DiveEvent` records important state transitions and warnings.

Current expected structure:

```cpp
struct DiveEvent {
    uint32_t timeSec;
    uint8_t type;
    int32_t value;
};
```

Event time:

```text
timeSec is relative to dive start.
```

Event value:

```text
value is event-specific.
```

Examples:

```text
DECO_REQUIRED:
    stop depth in cm

DECO_STOP_STARTED:
    stop depth in cm

DECO_STOP_COMPLETED:
    stop depth in cm

CEILING_EXCEEDED:
    ceiling depth in cm

ASCENT_RATE_WARN:
    ascent rate scaled value

LOW_BATTERY:
    battery percentage
```

Policy:

```text
1. Events are persisted in native log.
2. Events must be useful for post-dive review.
3. Events must support future XML/UDDF export.
4. New event type may not require log version bump if layout is unchanged.
5. Changing event value interpretation should be documented.
```

---

## 20. Time / RTS / GPS Correction Policy

The firmware supports relative and corrected time.

Time status policy:

```text
RelativeOnly:
    no valid epoch time at dive start

TimeSynced:
    valid time available at dive start

TimeCorrected:
    originally relative log corrected later using RTS
```

RTS correction uses:

```text
bootCount
timeSessionId
bootElapsedStartSec
bootElapsedEndSec
GPS-acquired epoch time
```

Correction rules:

```text
1. Only correct logs from same bootCount.
2. Only correct logs from same timeSessionId.
3. Do not correct if boot/session mismatch.
4. Header-only update must preserve samples/events.
```

Important v1.3.7 rule:

```text
RTS/GPS correction must not overwrite DiveSample[] or DiveEvent[] payload.
```

---

## 21. GPS / BLE / Charging Policy

General policy:

```text
During Dive:
    GPS off
    BLE off

During Surface:
    GPS limited search may run
    BLE depends on access window / connection

During PostDive:
    GPS limited search may run
    BLE generally off unless policy allows

During Charging:
    GPS may search
    BLE advertising may run
```

BLE access window:

```text
After Charging / Qi wake:
    BLE remains accessible for configured window

If BLE connected:
    keep BLE active

If entering Dive:
    BLE off immediately
```

GPS policy:

```text
1. GPS is useful for RTS/time correction.
2. GPS is useful for dive site location.
3. GPS should not run during Dive.
4. GPS search attempts are limited to avoid power drain.
```

---

## 22. UI Priority Policy

Recommended in-dive UI priority:

```text
1. Critical hardware/system warning
2. PPO2 HIGH / MOD EXCEEDED
3. Ceiling violation
4. CEIL >18m
5. DECO.STOP
6. Missed DECO / DECO violation advisory
7. Safety Stop
8. Ascent-rate warning
9. NDL / normal dive
10. Surface / PostDive / Charging
```

Important rule:

```text
DECO.STOP must always win over Safety Stop.
```

Battery low popup policy:

```text
Battery low may be shown periodically,
but must not obscure critical DECO or ceiling warnings for too long.
```

Future work:

```text
A centralized UI priority manager should be implemented.
```

---

## 23. Continuous Dive / Repetitive Dive Policy

Short surfacing should not always close a dive.

Policy:

```text
Dive -> temporary surfacing:
    start surface detection

If re-entry occurs before final close:
    continue same dive

If PostDive grace window expires:
    finalize log
    enter Surface

If dive starts again after Surface:
    repetitive dive
```

Continuous dive:

```text
1. Same dive count
2. Same log
3. Samples continue
4. Events continue
5. Tissue state naturally continues
```

Repetitive dive:

```text
1. New dive count
2. New log
3. Tissue state retained
4. Surface interval calculated
```

---

## 24. Storage / Stack Safety Policy

v1.3.7 added runtime buffers:

```text
DiveSample samples[...]
DiveEvent events[...]
```

These arrays are large enough to affect stack safety.

Important rule:

```text
Do not create temporary DiveRuntime objects on the loopTask stack.
```

Correct reset pattern:

```text
resetDiveRuntime()
memset global/runtime member
initialize required fields
```

Avoid:

```cpp
dive_ = DiveRuntime();
```

Reason:

```text
A temporary DiveRuntime can cause loop task stack overflow on ESP32.
```

If future buffers grow:

```text
1. Re-check stack usage.
2. Consider heap allocation.
3. Consider static/global storage.
4. Consider streaming log writes instead of full RAM buffer.
```

---

## 25. Serial Logging Policy

Serial logs should support debugging without overwhelming output.

Policy:

```text
buhlmann.cpp:
    should be mostly pure calculation
    avoid repetitive Serial.printf inside calculateDeco()

app.cpp:
    may log state transitions
    may log important events
    may log diagnostic Resync
```

Good logs:

```text
[DECO] Required ...
[DECO] Stop started ...
[DECO] Stop completed ...
[DECO] Cleared ...
[ALARM] Ceiling violation ...
[DIVE] final close ...
[LOG] saved ...
```

Avoid:

```text
Repeated per-loop calculation logs
Repeated unchanged remainSec logs
Repeated tissue simulation logs
```

Exception:

```text
Resync log is intentionally kept visible.
```

---

## 26. Wokwi Verification Policy

Wokwi simulation is the primary current verification environment.

Minimum verification before release tag:

```text
1. Clean build
2. Boot without stack overflow
3. Normal dive
4. Safety Stop
5. DECO entry
6. DECO ladder transition
7. DECO timer rounding
8. DECO timer no premature transition
9. Missed DECO
10. Re-entry after missed DECO
11. Native log save
12. Native log reload
13. RTS correction preserving payload
14. BLE/GPS/Charging mock behavior
```

Important DECO timer regression checks:

```text
1. No repeated 0:20 reset
2. Full stop duration displayed
3. Rounded timer enforced
4. No transition before local timer zero
5. Resync is rare
```

---

## 27. Subsurface / Export Policy

Native `.bdc` log is the source of truth.

Future export flow:

```text
BackUpDiveAssistant firmware
    -> native .bdc log

BLE or file extraction
    -> raw native log

External tool
    -> Subsurface XML
    -> UDDF
    -> CSV
```

Firmware should not prioritize on-device XML generation.

Reasons:

```text
1. XML is large.
2. XML generation is memory-intensive.
3. XML write is more fragile.
4. XML escaping increases firmware complexity.
5. Native log can support multiple export formats.
```

Future tool:

```text
tools/bdc_to_subsurface_xml.py
```

Initial XML event strategy:

```text
Export DiveEvent[] into notes first.
Later investigate native Subsurface profile event tags.
```

---

## 28. Known Limitations

Current known limitations:

```text
1. Not validated against certified dive computer behavior.
2. No real hardware pressure sensor integration yet.
3. No real hardware LCD integration yet.
4. No real BLE service implementation yet.
5. No Subsurface exporter yet.
6. No multi-log storage yet.
7. No runtime gas setting yet.
8. No MOD / ppO2 warning UI yet.
9. No centralized UI priority manager yet.
10. Limited automated regression test coverage.
11. Wokwi simulation may not represent real hardware timing/power behavior.
12. No real waterproof enclosure validation.
```

Algorithm limitations:

```text
1. Single gas only
2. Nitrogen only
3. No helium
4. No multi-gas switch
5. Simplified TTS assumptions
6. Prototype-level DECO implementation
```

---

## 29. Current High-Priority Development Direction

Recommended next development order:

```text
1. Finish v1.3.7 documentation update
2. Run full Wokwi regression verification
3. Add raw native log dump / validation commands
4. Add DECO regression scenarios
5. Clean up UI priority manager
6. Verify missed DECO / advisory persistence
7. Add EAN32 scenario support
8. Add MOD / ppO2 warning policy
9. Design BLE raw log transfer
10. Build Subsurface XML export tool
```

---

## 30. Release Policy for v1.3.7

`v1.3.7-dev` should remain a development version until:

```text
1. Clean build passes
2. Wokwi boot is stable
3. No stack overflow
4. DECO timer behavior is verified
5. DECO transitions are verified
6. Native log v3 save/load is verified
7. RTS correction preserves samples/events
8. Documentation is updated
9. Working tree is clean
```

Only after that should a non-dev tag be considered:

```text
v1.3.7
```

Before tagging:

```text
1. Update FW_VERSION if needed
2. Update CHANGELOG
3. Update LOG_FORMAT
4. Update DEVELOPMENT_v1.3
5. Update TODO
6. Update CHAT_CONTEXT
7. Confirm no temporary files committed
8. Push branch
9. Create tag only after verification
```

---

## 31. Summary of Critical Rules

The following rules should not be broken without deliberate redesign.

```text
1. BackUpDiveAssistant is experimental and not for real diving.
2. Native .bdc log is the source of truth.
3. BLE should transfer raw native log before XML support.
4. DECO.STOP always overrides Safety Stop.
5. No forced Safety Stop after DECO completion.
6. DECO timer must predict full stop duration.
7. DECO timer must not show repeated 20-second fragments.
8. Timer calculation uses copied tissue state.
9. Timer calculation uses stopDepth + deep margin.
10. Displayed rounded timer must be enforced.
11. Stop transition requires model permission and local timer zero.
12. Resync log should be rare and investigated if repeated.
13. DECO entry uses one long alarm.
14. NDL 10-second pre-warning is intentionally not implemented.
15. DECO entry alarm repeats if DECO clears and later re-enters.
16. Missed DECO does not cause hard lockout.
17. Re-entry after missed DECO continues calculation but is not recommended.
18. Large runtime buffers must not be placed on stack.
19. RTS header correction must preserve samples/events.
20. Subsurface XML is a future derived export, not primary storage.
```

---

## BLE Device Name Policy

The BLE advertising name shall use the following format:

```text
BUDA-XXXX
```

XXXX is a short stable device suffix.

For ESP32 prototype firmware, the suffix is derived from the last two bytes of the ESP32 Bluetooth MAC address.

For future custom hardware or production boards, the suffix may instead be derived from a factory-provisioned serial number, stored device ID, or another stable unique identifier.

The BLE advertising name is intended for user recognition during scanning. It must not be the only long-term identity used by companion apps.


Example:

```text
Bluetooth MAC: A4:CF:12:34:A1:B2
BLE name:      BUDA-A1B2
```

This allows multiple BackUpDiveAssistant devices to appear with distinguishable names during BLE scanning.
```
