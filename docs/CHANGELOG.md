
# CHANGELOG
---
본 문서는 BackupDiveComputer 프로젝트의 주요 변경 사항을 기록한다.
---

## dev/v7.2 - Current

### 2026-05-05

현재 개발 브랜치:

```text
dev/v7.2
```

최근 확인된 주요 커밋:

```text
a6d4a87 Clean up v7.2 scenario files
138c1b1 Fix remaining v7.2 preload compile issues
5ab55c1 Fix v7.2 epoch preload build errors
2f6216e Refactor surface preload to epoch-based previous dive model
```

---

## v7.2 주요 변경 사항

### 1. Surface 화면 데이터 모델 변경

기존에는 Surface 화면에 필요한 값을 시나리오에서 직접 넣는 방식이었다.

기존 개념:

```text
surfaceIntervalSec
noFlyRemainSec
```

v7.2에서는 실제 다이브컴퓨터 동작에 맞게 epoch 기반 구조로 변경했다.

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

## v7.1 Baseline

v7.1은 v7.2 리팩토링 전 기준 버전이다.

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

## v7.2 이후 계획

다음 기능은 v7.2 이후 계속 개발한다.

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
