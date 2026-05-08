
# CHANGELOG
---
본 문서는 BackupDiveComputer 프로젝트의 주요 변경 사항을 기록한다.
---

## Unreleased

### Added
- Add `docs/HARDWARE_PROTOTYPE.md` to document the real prototype hardware specification, ESP32-S3 pin map, ST7567A SPI LCD wiring, MS5837 wiring, GPS wiring, battery/charging policy, and open hardware validation items.
- Add `docs/HARDWARE_PROTOTYPE.json` for machine-readable hardware pin map, wiring table, components, power rails, and validation checklist.


## dev/v1.2 - Current

### 2026-05-06

#### 감압 위반 후 재입수 / 48시간 advisory 정책 정리

BackupDiveComputer의 감압 위반 후 동작 정책을 정리했다.

기존 검토 내용:

```text
일부 레크리에이션 컴퓨터는 감압정지 미완료 출수 후 24~48시간 lockout을 사용한다.
Shearwater 계열은 감압정지 위반 후에도 hard lockout 하지 않고 정보를 계속 제공한다.
Suunto Nautic 계열은 algorithm deviation 후에도 lock하지 않으며,
필요한 decompression stop이 clear되거나 48시간이 지나면 경고 상태가 해제되는 구조를 가진다.
```

BackupDiveComputer v1.3 개발 정책:

```text
hard lockout은 사용하지 않는다.
감압 계산과 DECO.STOP 안내는 계속 제공한다.
감압정지 미완료 출수 시 activeDecoViolation을 설정한다.
48시간 postViolationAdvisory를 시작한다.
재입수 시 DECO.STOP을 다시 계산하고 표시한다.
재입수 후 필요한 감압정지를 모두 완료하면 activeDecoViolation은 clear한다.
위반 이력과 advisory는 로그와 Surface UI에 남긴다.
```

UI 방향:

```text
표면:
MISSED DECO / NO DIVE ADVISED / 48H

재입수 중:
VIOL + DECO.STOP + ASCEND/HOLD/DOWN
```

이 정책은 백업용 다이브 컴퓨터의 목적에 맞게,
메인 컴퓨터가 잠기거나 제한되더라도 필요한 감압 정보를 계속 제공하기 위한 것이다.
```
```

### 2026-05-05

현재 개발 브랜치:

```text
dev/v1.2
```

최근 확인된 주요 커밋:

```text
a6d4a87 Clean up v1.2 scenario files
138c1b1 Fix remaining v1.2 preload compile issues
5ab55c1 Fix v1.2 epoch preload build errors
2f6216e Refactor surface preload to epoch-based previous dive model
```

---

## v1.2 주요 변경 사항

### 1. Surface 화면 데이터 모델 변경

기존에는 Surface 화면에 필요한 값을 시나리오에서 직접 넣는 방식이었다.

기존 개념:

```text
surfaceIntervalSec
noFlyRemainSec
```

v1.2에서는 실제 다이브컴퓨터 동작에 맞게 epoch 기반 구조로 변경했다.

새 개념:

```text
lastDiveStartEpochSec_
lastDiveDurationSec_
lastDiveEndEpochSec_
noFlyEndEpochSec_
```

Surface interval 계산:

```text
현재 시각 - 마지막 다이빙 종료 시각
```

No-Fly remaining 계산:

```text
No-Fly 종료 시각 - 현재 시각
```

---

### 2. 이전 다이브 preload 구조 변경

`scenarios/previous_dive_surface.json`에서 이전 다이빙 정보를 실제 로그와 유사한 구조로 저장하도록 변경했다.

주요 preload 정보:

```text
startEpoch
durationSec
maxDepthM
minTempC
noFlyMinutesAtEnd
GPS latitude / longitude / place
```

이를 통해 시뮬레이션에서도 실제 제품과 유사하게 Surface 화면을 표시할 수 있다.

---

### 3. generated_scenario.h 자동 생성 개선

`tools/generate_scenario.py`는 JSON 시나리오 파일을 읽어 C++에서 사용할 수 있는 헤더 파일을 자동 생성한다.

생성 파일:

```text
include/generated_scenario.h
```

주요 생성 값:

```text
SCENARIO_START_EPOCH
SCENARIO_TZ_OFFSET_MIN
SCENARIO_PRELOAD_ENABLED
SCENARIO_PRELOAD_LAST_START_EPOCH
SCENARIO_PRELOAD_LAST_DURATION_SEC
SCENARIO_PRELOAD_LAST_END_EPOCH
SCENARIO_PRELOAD_LAST_MAX_DEPTH_M
SCENARIO_PRELOAD_LAST_MIN_TEMP_C
SCENARIO_PRELOAD_NO_FLY_END_EPOCH
SCENARIO_PRELOAD_GPS_VALID
SCENARIO_PRELOAD_GPS_LAT
SCENARIO_PRELOAD_GPS_LON
SCENARIO_PRELOAD_GPS_PLACE
SCENARIO_POINTS[]
```

주의:

```text
generated_scenario.h는 자동 생성 파일이므로 직접 수정하지 않는다.
```

---

### 4. Surface UI 표시 개선

Surface 화면에서 다음 정보를 표시하는 방향으로 정리했다.

```text
LAST
MAX
TMP
SURFACE
N-FLY
```

각 항목의 의미:

```text
LAST    마지막 다이빙 시작 날짜
MAX     마지막 다이빙 최대 수심
TMP     마지막 다이빙 최저 수온
SURFACE 마지막 다이빙 종료 후 경과 시간
N-FLY   비행 금지 남은 시간
```

LAST는 `LAST DIVE`가 아니라 `LAST`로 표시한다.

LAST 값은 날짜만 표시한다.

```text
YYYY-MM-DD
```

---

### 5. app.cpp epoch 기반 계산 반영

`src/app.cpp`에 다음 계산 로직이 반영되었다.

```cpp
getSurfaceIntervalSec()
getNoFlyRemainSec()
getCurrentEpochSec()
getSimEpochSec()
```

Surface 화면과 PostDive 화면에서 No-Fly 시간을 epoch 기반으로 계산하도록 수정했다.

---

### 6. ui.cpp / ui.h 수정

`include/ui.h`와 `src/ui.cpp`에서 Surface 화면 함수 인자와 표시 형식을 정리했다.

Surface UI는 다음 값을 전달받아 화면에 표시한다.

```text
currentEpochSec
tzOffsetMin
batteryPct
gpsValid
charging
chargeFull
lastDiveStartEpochSec
lastMaxDepthM
lastMinTempC
surfaceIntervalSec
noFlyRemainSec
```

---

### 7. platformio.ini 및 Wokwi 빌드 설정 유지

Wokwi 시뮬레이션용 빌드 플래그를 유지한다.

```ini
-D WOKWI_SIMULATION
-D ARDUINO_USB_CDC_ON_BOOT=1
-D CORE_DEBUG_LEVEL=3
```

시나리오 자동 생성 스크립트:

```ini
extra_scripts = pre:tools/generate_scenario.py
```

---

### 8. 불필요한 임시 파일 정리

빌드 결과 로그나 임시 파일은 Git에 올리지 않는 방향으로 정리한다.

예:

```text
pio run -e esp32s3_sim.txt
logs/
exports/
.pio/
.pio*
```

---

## v1.1 Baseline

v1.1은 v1.2 리팩토링 전 기준 버전이다.

포함 기능:

```text
ESP32-S3 / Wokwi simulation
OLED 128x64 UI prototype
Scenario JSON based simulated dive profile
Surface screen prototype
Battery low popup logic
Charging indicator behavior
GF setting mock command
Manual depth and profile control commands
Ascent-rate graph with fixed risk-zone dot pattern
```

---

## v1.2 이후 계획

다음 기능은 v1.2 이후 계속 개발한다.

```text
Compact internal dive log format
Dive log storage
Subsurface XML export
Subsurface XML import or scenario parser
BLE log download
Real product power management
GPS/BLE/display low-power control
Time sync failure handling
timeSessionId based log correction
```

## v1.3 - Current

### 2026-05-07

#### FO2 기반 gas 설정 및 Nitrox-ready 구조 추가

v1.3에서 기본 gas를 Air / EAN21로 명확히 정의하고, 향후 Nitrox 지원을 위한 FO2 기반 구조를 코드에 반영했다.

적용 내용:

```text
DIVE_GAS_FO2_PERCENT = 21
DIVE_GAS_PPO2_MAX_BAR = 1.40f
DIVE_GAS_FO2_MIN_PERCENT = 21
DIVE_GAS_FO2_MAX_PERCENT = 40
```

중요 정책:

```text
Air = EAN21
EAN32는 Air가 아님
Bühlmann 계산에서 FN2를 하드코딩하지 않음
config.h의 FO2에서 FN2를 계산해 사용
```

수정 파일:

```text
include/config.h
include/buhlmann.h
src/buhlmann.cpp
```

---

#### Bühlmann FN2 하드코딩 제거

기존 Bühlmann 계산에서 사용하던 질소 비율 하드코딩 값을 제거하고, `DIVE_GAS_FO2_PERCENT`에서 계산한 FN2를 사용하도록 변경했다.

적용된 helper:

```text
getGasFO2()
getGasFN2()
calculateMODMeters()
calculatePpO2Bar(depthM)
```

적용 범위:

```text
tissue nitrogen loading
NDL calculation
GF99 calculation
decompression calculation
No-Fly calculation
```

---

#### DECO.STOP ladder 및 CEIL >18m 처리 추가

v1.3 DECO.STOP ladder를 코드에 반영했다.

지원 ladder:

```text
18m -> 15m -> 12m -> 9m -> 6m -> 3m
```

raw ceiling이 18m보다 깊은 경우에는 18m stop으로 잘못 안내하지 않고 별도 경고 상태로 처리한다.

표시 정책:

```text
CEIL >18m
HOLD DEPTH
```

수정 파일:

```text
include/config.h
include/buhlmann.h
src/buhlmann.cpp
include/app.h
src/app.cpp
include/ui.h
src/ui.cpp
```

---

#### DECO.STOP 행동 지시 UI 개선

기존 `TOO SHALLOW` 표시를 제거하고, 더 직관적인 행동 지시로 변경했다.

새 표시:

```text
ASCEND
DESCEND
HOLD
```

화살표는 Unicode 문자를 사용하지 않고 `u8g2.drawTriangle()`로 직접 그린다.

이유:

```text
실제 예정 LCD/ST7567 계열 그래픽 LCD는 font chip이 없을 수 있음
Unicode 화살표가 폰트에 따라 표시되지 않을 수 있음
따라서 ASCII 텍스트 + 직접 그린 삼각형 아이콘을 사용
```

---

#### 배터리 경고 및 비프 정책 수정

LOW BATTERY 팝업 주기를 2분에서 10분으로 변경했다.

```text
기존: 2분마다 2초 표시
변경: 10분마다 2초 표시
```

적용 값:

```text
BATTERY_LOW_POPUP_INTERVAL_MS = 600000UL
BATTERY_LOW_POPUP_DURATION_MS = 2000UL
```

LOW BATTERY 표시 시 짧은 비프 1회를 호출하도록 변경했다.

```text
ALARM_FREQ_BATTERY_LOW = 900
beep(ALARM_FREQ_BATTERY_LOW, 80)
```

---

#### 빠른 상승 경고 비프 3회 적용

상승 속도가 위험 기준 이상일 때 단일 비프 대신 3회 연속 경고음을 호출하도록 변경했다.

정책:

```text
18m/min 이상 빠른 상승 시
삐-삐-삐 3회 경고
```

관련 함수:

```text
beepTripleWarning()
```

---

#### Wokwi buzzer 테스트 및 Serial Monitor 설정 개선

Wokwi 시뮬레이션에서 buzzer 테스트를 위해 mock command를 추가했다.

새 명령:

```text
beep test
```

예상 출력:

```text
[MOCK_BUZZER] beep test
```

또한 Wokwi serial monitor 입력을 쉽게 하기 위해 `platformio.ini`에 monitor port를 추가했다.

```ini
monitor_port = rfc2217://localhost:4000
```

Wokwi buzzer 설정도 `smooth` mode로 정리했다.

수정 파일:

```text
src/mock_services.cpp
diagram.json
platformio.ini
```

---

#### 현재 남은 확인 사항

```text
로컬 환경에서 pio run -e wokwi 빌드 확인 필요
Wokwi/VS Code 환경에서 buzzer 실제 음성 출력 확인 필요 (DONE. 정상출력 확인 완료!)
missed deco / re-entry / 48h advisory 상태 머신 및 로그 이벤트는 추가 구현 필요
```

### Version renaming policy

기존 개발 중 v7.x 형태로 사용하던 버전명을 v1.x 체계로 재정리한다.

정리 기준:

```text
old v7.1-baseline → v1.0-baseline
old dev/v7.2      → dev/v1.2
old dev/v7.3      → dev/v1.3
```

# 기존 v7.x 브랜치 삭제는 나중에

새 브랜치가 잘 만들어지고, 빌드가 되는지 확인한 다음에만 삭제하세요.

삭제는 선택입니다.

## 원격 브랜치 삭제

```bash
git push origin --delete dev/v7.3
```
