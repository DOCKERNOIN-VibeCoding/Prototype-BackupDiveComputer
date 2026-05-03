# BackupDiveComputer_v7.2 개발 정의서

> 본 문서는 지금까지의 개발 논의 내용을 기준으로 `BackupDiveComputer_v7.2` 버전의 개발 방향, 구조, UI 정책, 시나리오/로그 설계, 향후 구현 우선순위를 정리한 개발 정의서이다.  
> 이후 대화가 새로 시작되더라도, 이 문서를 제시하면 바로 이어서 개발을 진행할 수 있도록 작성한다.

---

# 1. 버전 정의

## 프로젝트명

```text
BackupDiveComputer
```

## 기준 버전

```text
BackupDiveComputer_v7.2
```

## v7.2의 핵심 목표

`BackupDiveComputer_v7.2`는 기존 Wokwi 기반 다이브컴퓨터 시뮬레이션 프로젝트에 다음 개발 방향을 적용하는 버전이다.

1. Surface 화면 구조 확정
2. 이전 다이브 로그 기반 Surface 정보 표시
3. 시나리오와 실제 로그 구조의 방향성 통일
4. 내부 로그는 compact하게 저장
5. Export/Import 시 Subsurface XML 호환 구조 사용
6. 시뮬레이션 입력도 장기적으로 Subsurface XML 기반으로 전환
7. BLE 로그 다운로드 및 Subsurface 호환성을 고려한 로그 설계
8. UI는 작은 OLED 화면에서 가독성을 최우선으로 설계

---

# 2. 현재 프로젝트 전제

## 개발 환경

```text
PlatformIO
Wokwi
ESP32-S3
Arduino Framework
OLED 128x64
U8G2
```

## 주요 폴더 구조

```text
include/
  app.h
  config.h
  mock_services.h
  sim_sensor.h
  ui.h
  generated_scenario.h

src/
  app.cpp
  mock_services.cpp
  sim_sensor.cpp
  ui.cpp
  main.cpp

tools/
  generate_scenario.py

scenarios/
  current.json
  previous_dive_surface.json
  fast_ascent_30m.json
  safety_stop_skip.json
  deco_30m_long.json

platformio.ini
```

## 향후 v7.2에서 추가 또는 확장 예정인 구조

```text
include/
  log_format.h
  log_storage.h
  log_export_xml.h
  scenario.h

src/
  log_storage.cpp
  log_export_xml.cpp

tools/
  generate_scenario.py
  bdc_to_subsurface_xml.py
  subsurface_xml_to_bdc.py

scenarios/
  previous_dive_surface.xml
  fast_ascent.xml
  deco_dive.xml

logs/
  dive_0001.bdc

exports/
  dive_0001.xml
```

---

# 3. v7.2의 핵심 설계 원칙

## 3.1 내부 저장과 외부 호환 포맷 분리

v7.2부터 로그 시스템은 다음 원칙을 따른다.

```text
내부 저장:
  compact binary 또는 compact record 형태

외부 Export/Import:
  Subsurface XML 호환 형태

시뮬레이션:
  장기적으로 Subsurface XML을 입력으로 사용
```

즉, 다이브컴퓨터 내부에서는 XML을 직접 저장하지 않는다.

이유:

1. XML은 용량이 크다.
2. ESP32 저장공간에 비효율적이다.
3. XML 파싱 비용이 크다.
4. BLE 전송 시 데이터량이 커진다.
5. Surface 화면이나 계산 로직에는 XML 전체 구조가 필요하지 않다.

따라서 내부적으로는 compact log를 저장하고, Export 시점에만 XML로 변환한다.

---

## 3.2 실제 다이브 로그 구조와 시나리오 구조 통일

기존 시나리오에서는 다음처럼 단순 값을 넣었다.

```json
"surfaceIntervalSec": 5040,
"noFlyRemainSec": 41520
```

하지만 이는 실제 프로그램 동작 방식과 다르다.

실제 다이브컴퓨터는 다음 정보를 저장해야 한다.

```text
다이빙 시작 날짜/시간
다이빙 지속 시간
최대 수심
최저 수온
프로파일 샘플
출수 시점의 No-Fly 계산값
GPS 위치
가스 정보
GF 설정
이벤트 로그
```

Surface 시간은 저장하는 값이 아니라 계산하는 값이다.

```cpp
lastDiveEndEpochSec = lastDiveStartEpochSec + lastDiveDurationSec;
surfaceIntervalSec = currentEpochSec - lastDiveEndEpochSec;
```

따라서 v7.2에서는 시나리오도 실제 로그 구조에 맞춘다.

---

# 4. Surface 화면 정의

## 4.1 Surface 화면이 표시되는 조건

Surface 화면은 다음 조건에서 표시되어야 한다.

1. Qi 충전 중
2. Qi 충전기에 짧은 터치로 Wake 모드 진입
3. 출수 후 Surface 모드 진입

별도의 단순 Charging 화면보다, 충전 중에도 Surface 정보 화면을 보여주는 것을 기본 방향으로 한다.

---

## 4.2 Surface 화면 표시 정보

### 상단 바

상단 바에는 다음 정보를 표시한다.

```text
현재 날짜·시간
GPS 신호 수신 여부
배터리 상태
충전 상태
```

예시:

```text
2026/04/30 10:24     G [BAT]
```

GPS 표시는 다음 기준을 따른다.

```text
G : GPS 유효
- : GPS 없음 또는 미수신
```

---

### 본문 정보

Surface 화면 본문에는 다음 정보를 표시한다.

```text
LAST     YYYY-MM-DD
MAX      28.6m
TMP      22C
SURFACE  01:24
N-FLY    11:32
```

No-Fly가 종료된 경우:

```text
N-FLY    SAFE TO FLY
```

Surface 시간이 24시간을 초과한 경우:

```text
SURFACE  125 days 08 H
```

---

## 4.3 LAST 항목 정의

기존 `LAST DIVE` 표시는 화면 폭 문제로 인해 사용하지 않는다.

v7.2부터는 다음처럼 표시한다.

```text
LAST
```

LAST 값은 날짜만 표시한다.

```text
YYYY-MM-DD
```

예:

```text
LAST     2026-04-30
```

## 4.4 LAST 기준

`LAST`는 마지막 다이빙의 **시작 날짜**를 기준으로 표시한다.

이유:

1. Subsurface XML의 `<dive date='' time=''>`는 다이빙 시작 시각 기준이다.
2. 일반 로그북에서 다이브 날짜는 시작 날짜 기준이다.
3. 출수 시각은 `start + duration`으로 계산 가능하다.
4. Surface 시간은 출수 시각 기준으로 별도 계산한다.

따라서:

```cpp
LAST 표시 = lastDiveStartEpochSec_ 기준 YYYY-MM-DD
SURFACE 계산 = currentEpochSec - lastDiveEndEpochSec_
```

---

# 5. Surface 화면 UI 정책

## 5.1 OLED 화면 정책

현재 확정된 Surface 화면 레이아웃은 가독성이 좋고 정보가 화면에 꽉 차므로, 앞으로도 이 구성을 기준으로 한다.

기본 원칙:

1. 작은 128x64 화면에서는 큰 폰트를 우선한다.
2. 정보는 핵심 항목만 표시한다.
3. 줄 간격은 좁게 유지하되 겹치지 않게 한다.
4. 4x6 등 작은 폰트는 보조 정보용으로만 사용한다.
5. 본문은 큰 폰트 중심으로 구성한다.
6. 화면 전체를 최대한 활용한다.

---

## 5.2 Surface 본문 폰트 기준

현재 최적 구성:

```text
본문 폰트: u8g2_font_6x10_tf 계열
줄 간격: 11 px
```

권장 Y 좌표:

```cpp
y1 = 19;
y2 = 30;
y3 = 41;
y4 = 52;
y5 = 63;
```

본문 라벨 예:

```text
LAST
MAX
TMP
SURFACE
N-FLY
```

---

## 5.3 향후 Dive/Deco/Safety 화면에도 적용할 UI 원칙

Surface 화면에서 확정된 UI 철학을 다른 화면에도 적용한다.

```text
큰 폰트
명확한 정보 계층
상단 상태바
핵심 정보만 표시
화면 여백 최소화
시인성 우선
```

---

# 6. 충전 상태 UI 정의

## 6.1 충전 중 표시

충전 중에는 Surface 화면 상단의 배터리 아이콘을 깜빡인다.

```text
charge on
battery 75
```

예상 동작:

```text
배터리 아이콘 점멸
```

---

## 6.2 충전 완료 표시

충전 중이고 배터리가 100%이면, 배터리 아이콘과 `FULL` 문자를 번갈아 표시한다.

```text
charge on
battery 100
```

예상 동작:

```text
[BAT] ↔ FULL
```

---

## 6.3 충전기 분리 시

충전기를 분리하면 일반 Surface 화면으로 복구한다.

```text
charge off
```

예상 동작:

```text
일반 배터리 아이콘 고정 표시
```

---

## 6.4 관련 설정값

`config.h`에 다음 값이 존재하거나 추가되어야 한다.

```cpp
#define BATTERY_FULL_THRESHOLD_PCT 100
#define CHARGING_BLINK_INTERVAL_MS 500UL
#define CHARGING_FULL_BLINK_INTERVAL_MS 1000UL
```

---

# 7. 배터리 Low 팝업 정의

## 7.1 기본 정책

시나리오에서 배터리 값은 변경하지 않는다.  
배터리 값은 기본 프로그램 또는 mock command를 통해 제어한다.

GF 설정값도 시나리오에서 가져오지 않는다.  
GF는 `config.h`의 기본값과 규정을 사용한다.

---

## 7.2 Battery Low 팝업 조건

배터리가 10% 이하이면 경고 팝업을 표시한다.

```cpp
BATTERY_LOW_THRESHOLD_PCT = 10
```

동작:

```text
배터리 10% 이하
2분 간격
2초 동안 BATTERY LOW 팝업 표시
```

관련 값:

```cpp
BATTERY_LOW_THRESHOLD_PCT
BATTERY_LOW_POPUP_INTERVAL_MS
BATTERY_LOW_POPUP_DURATION_MS
```

예상 값:

```cpp
#define BATTERY_LOW_THRESHOLD_PCT 10
#define BATTERY_LOW_POPUP_INTERVAL_MS 120000UL
#define BATTERY_LOW_POPUP_DURATION_MS 2000UL
```

---

## 7.3 Wokwi 테스트 명령

```text
battery 9
```

예상:

```text
[CMD] battery 9
[MOCK_PWR] battery 9%
[WARN] BATTERY LOW: 9%
```

화면:

```text
BATTERY LOW
```

복구:

```text
battery 50
```

---

# 8. 상승속도 그래프 UI 정의

## 8.1 Dot Pattern의 의미

상승속도 그래프에서 Dot Pattern은 현재 속도가 위험영역에 들어갔을 때만 보이는 것이 아니다.

Dot Pattern은 **위험영역을 나타내는 고정 배경**이다.

따라서 상승 중이 아니어도, 하강 중이어도, 정지 중이어도 항상 보여야 한다.

---

## 8.2 목적

다이버가 상승 중 그래프를 보면서 현재 상승속도가 위험영역에 가까워지는지 실시간으로 확인하기 위함이다.

즉:

```text
상승속도 바가 Dot Pattern 영역에 닿으려 하면
상승 속도를 줄인다.
```

---

## 8.3 상승속도 그래프 기준

예상 설정:

```cpp
ASC_GRAPH_MIN_RATE_MPM = 0.0f
ASC_GRAPH_MAX_RATE_MPM = 18.0f
ASC_DOT_START_RATE_MPM = 10.0f
```

표시 원칙:

```text
0~10 m/min     : Safe 영역
10~18 m/min    : Dot Pattern 위험 영역
```

Dot Pattern은 항상 배경으로 표시한다.

---

# 9. GF 설정 정책

## 9.1 GF는 시나리오에서 가져오지 않는다

GF Low/High는 시나리오 파일에서 설정하지 않는다.

GF 기본값과 범위 제한은 `config.h`를 따른다.

예:

```cpp
#define DEFAULT_GF_LOW 40
#define DEFAULT_GF_HIGH 85
```

---

## 9.2 MockServices 초기화

`MockServices::begin()`에서 GF 기본값은 `config.h` 기준으로 초기화한다.

```cpp
gfLow_ = DEFAULT_GF_LOW;
gfHigh_ = DEFAULT_GF_HIGH;
```

---

## 9.3 Wokwi 테스트 명령

```text
gf 35 75
```

정상 예상:

```text
[CMD] gf 35 75
[MOCK_BLE] GF update requested 35/75
[DECO] GF set 35/75
```

오류 테스트:

```text
gf 95 40
```

예상:

```text
GF range/order error
```

---

# 10. 시나리오 설계 방향

## 10.1 기존 JSON 시나리오

현재는 JSON 기반 시나리오를 사용한다.

예:

```text
scenarios/current.json
scenarios/previous_dive_surface.json
scenarios/fast_ascent_30m.json
```

`platformio.ini`에서 선택:

```ini
custom_scenario = scenarios/current.json
```

빌드 전 스크립트:

```ini
extra_scripts = pre:tools/generate_scenario.py
```

---

## 10.2 v7.2 이후 방향

v7.2에서는 JSON 시나리오를 즉시 제거하지 않는다.  
대신 XML 시나리오를 지원하도록 확장한다.

`generate_scenario.py`는 다음을 지원해야 한다.

```python
if scenario_path.endswith(".json"):
    load_json_scenario()
elif scenario_path.endswith(".xml"):
    load_subsurface_xml_scenario()
```

---

## 10.3 장기 목표

시나리오는 Subsurface XML 구조를 입력으로 사용할 수 있어야 한다.

예:

```text
scenarios/previous_dive_surface.xml
scenarios/fast_ascent.xml
scenarios/deco_dive.xml
```

이 XML에서 실제 다이빙 로그 정보를 읽어 시뮬레이션에 적용한다.

---

# 11. Subsurface XML 호환 방향

## 11.1 Subsurface XML 기본 구조

Subsurface XML은 일반적으로 다음 구조를 가진다.

```xml
<divelog program='subsurface' version='3'>
  <settings>
  </settings>

  <divesites>
    <site uuid='...' name='...'>
    </site>
  </divesites>

  <dives>
    <dive number='1'
          date='2026-04-30'
          time='09:00:00'
          duration='46:00 min'
          divesiteid='...'>
      <cylinder ... />

      <divecomputer model='BackupDiveComputer' deviceid='BDC-0001'>
        <sample time='0:00 min' depth='0.0 m' temp='25.0 C' />
        <sample time='0:30 min' depth='3.5 m' temp='24.8 C' />
      </divecomputer>
    </dive>
  </dives>
</divelog>
```

---

## 11.2 우리가 우선 지원해야 할 XML 항목

Subsurface XML 전체를 완벽히 지원할 필요는 없다.  
우선 다음 항목을 지원한다.

### Dive 기본 정보

```xml
<dive
  number='1'
  date='2026-04-30'
  time='09:00:00'
  duration='46:00 min'
/>
```

### Dive profile sample

```xml
<sample time='0:00 min' depth='0.0 m' temp='25.0 C' />
<sample time='1:00 min' depth='5.0 m' temp='24.5 C' />
```

### 위치 정보

```xml
<site uuid='...' name='Jeju'>
```

GPS 좌표는 Subsurface XML 형식에 맞춰 추후 확정한다.

### 장비/가스 정보

```xml
<cylinder size='11.1 l'
          workpressure='207.0 bar'
          start='200.0 bar'
          end='80.0 bar'
          o2='21.0%' />
```

---

## 11.3 XML에서 계산해야 할 값

XML에 직접 surface interval을 넣지 않는다.

XML에서 가져오는 값:

```text
dive date
dive time
duration
sample depth
sample temp
```

계산값:

```cpp
lastDiveStartEpochSec = parse(date + time);
lastDiveDurationSec = parse(duration);
lastDiveEndEpochSec = lastDiveStartEpochSec + lastDiveDurationSec;
surfaceIntervalSec = currentEpochSec - lastDiveEndEpochSec;
```

최대 수심:

```cpp
lastDiveMaxDepthM = max(sample.depth)
```

최저 수온:

```cpp
lastDiveMinTempC = min(sample.temp)
```

---

# 12. generated_scenario.h 방향

`generate_scenario.py`는 JSON 또는 XML을 읽어 다음 header를 생성한다.

예상 구조:

```cpp
#pragma once

#include <stdint.h>

#define SCENARIO_NAME "previous_dive_surface"
#define SCENARIO_START_EPOCH 1777510800UL
#define SCENARIO_TZ_OFFSET_MIN 540

#define SCENARIO_HAS_PREVIOUS_DIVE 1

#define SCENARIO_LAST_DIVE_START_EPOCH 1777500000UL
#define SCENARIO_LAST_DIVE_DURATION_SEC 2760UL
#define SCENARIO_LAST_DIVE_END_EPOCH 1777502760UL

#define SCENARIO_LAST_DIVE_MAX_DEPTH_M 28.6f
#define SCENARIO_LAST_DIVE_MIN_TEMP_C 22.0f

#define SCENARIO_NO_FLY_END_EPOCH 1777545960UL

#define SCENARIO_INITIAL_DEPTH_M 0.0f
#define SCENARIO_INITIAL_TEMP_C 25.0f
#define SCENARIO_INITIAL_SURFACE_PRESSURE_BAR 1.01325f

struct ScenarioPoint {
    uint32_t tSec;
    float depthM;
    float tempC;
};

static const ScenarioPoint SCENARIO_POINTS[] = {
    { 0,    0.0f, 25.0f },
    { 60,   5.0f, 24.5f },
    { 120, 13.7f, 24.0f },
    { 2400, 5.0f, 22.0f },
    { 2760, 0.0f, 22.5f },
};

#define SCENARIO_POINT_COUNT 5
```

주의:

```text
generated_scenario.h에는 GF 값과 배터리 값이 들어가면 안 된다.
```

---

# 13. 앱 내부 Surface 관련 변수 정의

`DiveComputerApp`에는 다음 변수들이 필요하다.

```cpp
uint32_t lastDiveStartEpochSec_;
uint32_t lastDiveDurationSec_;
uint32_t lastDiveEndEpochSec_;

float lastDiveMaxDepthM_;
float lastDiveMinTempC_;

uint32_t noFlyEndEpochSec_;

uint32_t surfaceIntervalOffsetSec_; // 향후 제거 가능
```

v7.2의 목표는 `surfaceIntervalOffsetSec_` 같은 직접 지정 방식보다, epoch 기반 계산으로 정리하는 것이다.

---

## 13.1 현재 시각 계산

`app.cpp`에는 다음 함수가 있어야 한다.

```cpp
uint32_t DiveComputerApp::getCurrentEpochSec() const {
    return SCENARIO_START_EPOCH + getSimEpochSec();
}
```

`getSimEpochSec()`는 시뮬레이션 시작 후 경과 시간을 반환한다.

---

## 13.2 Surface interval 계산

```cpp
uint32_t DiveComputerApp::getSurfaceIntervalSec() const {
    uint32_t now = getCurrentEpochSec();

    if (lastDiveEndEpochSec_ == 0 || now < lastDiveEndEpochSec_) {
        return 0;
    }

    return now - lastDiveEndEpochSec_;
}
```

---

## 13.3 No-Fly remain 계산

```cpp
uint32_t DiveComputerApp::getNoFlyRemainSec() const {
    uint32_t now = getCurrentEpochSec();

    if (noFlyEndEpochSec_ == 0 || now >= noFlyEndEpochSec_) {
        return 0;
    }

    return noFlyEndEpochSec_ - now;
}
```

---

# 14. 다이빙 종료 시 로그 요약 저장 정책

`endDive()`에서는 다음 값을 반드시 갱신해야 한다.

```cpp
lastDiveStartEpochSec_ = diveStartEpochSec_;
lastDiveDurationSec_ = diveDurationSec;
lastDiveEndEpochSec_ = getCurrentEpochSec();

lastDiveMaxDepthM_ = currentDiveMaxDepthM_;
lastDiveMinTempC_ = currentDiveMinTempC_;

noFlyEndEpochSec_ = lastDiveEndEpochSec_ + calculatedNoFlySec;
```

향후에는 이 정보를 compact log header로 저장한다.

---

# 15. 이전 다이브 preload 정책

## 15.1 기존 방식

기존 preload는 다음처럼 surface interval을 직접 넣는 방식이었다.

```json
"surfaceIntervalSec": 5040,
"noFlyRemainSec": 41520
```

v7.2에서는 이 방식을 폐기 방향으로 본다.

---

## 15.2 v7.2 권장 방식

시나리오 preload는 실제 로그처럼 다음 정보를 가져야 한다.

```json
"lastDive": {
  "startEpoch": 1777500000,
  "durationSec": 2760,
  "maxDepthM": 28.6,
  "minTempC": 22.0,
  "noFlyMinutesAtEnd": 720
}
```

또는 XML에서는:

```xml
<dive number='1'
      date='2026-04-30'
      time='09:00:00'
      duration='46:00 min'>
  <divecomputer model='BackupDiveComputer'>
    <sample time='0:00 min' depth='0.0 m' temp='25.0 C' />
    <sample time='20:00 min' depth='28.6 m' temp='22.0 C' />
    <sample time='46:00 min' depth='0.0 m' temp='22.5 C' />
  </divecomputer>
</dive>
```

---

# 16. Compact Log 저장 구조

## 16.1 DiveLogHeader

향후 `include/log_format.h`에 다음 구조를 정의한다.

```cpp
#pragma once

#include <stdint.h>

#define BDC_LOG_MAGIC 0x4244434CUL // 'BDCL'
#define BDC_LOG_VERSION 1

struct DiveLogHeader {
    uint32_t magic;
    uint16_t version;

    uint32_t diveNumber;

    uint32_t startEpochSec;
    uint32_t durationSec;
    uint32_t endEpochSec;

    uint16_t maxDepthCm;
    uint16_t avgDepthCm;
    int16_t  minTempDeciC;

    uint16_t sampleCount;
    uint16_t eventCount;

    uint16_t noFlyMinutesAtEnd;
    uint16_t surfacePressureMbar;

    uint8_t  gfLow;
    uint8_t  gfHigh;

    uint8_t  gasO2Pct;
    uint8_t  gasHePct;

    int32_t  latitudeE7;
    int32_t  longitudeE7;
    uint8_t  gpsValid;

    uint16_t siteNameOffset;
    uint16_t noteOffset;
};
```

---

## 16.2 DiveSample

```cpp
struct DiveSample {
    uint16_t timeSec;
    uint16_t depthCm;
    int16_t  tempDeciC;
    uint16_t pressureDeciBar;
};
```

예:

```text
18.35 m  -> 1835
22.4 C   -> 224
190.5bar -> 1905
```

탱크 압력이 없으면:

```cpp
pressureDeciBar = 0xFFFF;
```

---

## 16.3 DiveEvent

```cpp
enum DiveEventType : uint8_t {
    EVENT_DIVE_START,
    EVENT_DIVE_END,
    EVENT_ASCENT_RATE_WARN,
    EVENT_SAFETY_STOP_START,
    EVENT_SAFETY_STOP_DONE,
    EVENT_DECO_START,
    EVENT_DECO_CLEARED,
    EVENT_GAS_CHANGE,
    EVENT_BATTERY_LOW,
};

struct DiveEvent {
    uint16_t timeSec;
    uint8_t  type;
    int16_t  value;
};
```

---

# 17. XML Export 정책

## 17.1 내부 compact log에서 XML로 변환

Export 시 다음 흐름을 따른다.

```text
logs/dive_0001.bdc
    ↓
bdc_to_subsurface_xml
    ↓
exports/dive_0001.xml
```

---

## 17.2 Export XML 예시

```xml
<divelog program='BackupDiveComputer' version='1'>
  <settings>
  </settings>

  <divesites>
    <site uuid='bdc-site-0001' name='Jeju'>
    </site>
  </divesites>

  <dives>
    <dive number='1'
          date='2026-04-30'
          time='09:00:00'
          duration='46:00 min'
          divesiteid='bdc-site-0001'>

      <cylinder size='11.1 l'
                workpressure='207.0 bar'
                start='200.0 bar'
                end='80.0 bar'
                o2='21.0%' />

      <divecomputer model='BackupDiveComputer'
                    deviceid='BDC-0001'>

        <sample time='0:00 min' depth='0.0 m' temp='25.0 C' />
        <sample time='0:30 min' depth='3.5 m' temp='24.8 C' />
        <sample time='1:00 min' depth='7.0 m' temp='24.5 C' />

      </divecomputer>
    </dive>
  </dives>
</divelog>
```

---

# 18. BLE 로그 다운로드 방향

## 18.1 장기 목표

Bluetooth를 통해 로그를 다운로드할 수 있어야 한다.  
이 로그는 다이브로그북 프로그램, 특히 Subsurface와 호환 가능해야 한다.

---

## 18.2 BLE 전송 방식 후보

### 방식 A: compact log 전송

```text
다이브컴퓨터 -> 앱/PC: .bdc compact log
앱/PC -> Subsurface XML 변환
```

장점:

```text
전송량 작음
기기 부담 작음
빠름
```

단점:

```text
전용 변환 앱 또는 PC tool 필요
```

---

### 방식 B: XML 직접 전송

```text
다이브컴퓨터 내부에서 XML 생성
BLE로 XML 전송
```

장점:

```text
Subsurface 호환 직관적
사용자에게 이해 쉬움
```

단점:

```text
전송량 큼
메모리 부담 큼
XML 생성 로직 필요
```

---

## 18.3 v7.2 권장 방향

초기에는 다음 방식을 권장한다.

```text
내부 저장: compact .bdc
BLE 전송: compact 우선
PC/app export: Subsurface XML 변환
```

나중에 여유가 있으면 기기 자체 XML export 기능을 추가한다.

---

# 19. Wokwi Serial Monitor 명령

현재 또는 유지해야 할 mock command:

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

---

## 19.1 Serial 입력 처리 주의사항

Wokwi Serial Monitor에서는 Enter 입력이 `\n` 또는 `\r\n`으로 들어올 수 있다.

따라서 `mock_services.cpp`의 Serial command parser는 반드시 다음을 모두 처리해야 한다.

```cpp
if (c == '\r' || c == '\n') {
    processCommand();
}
```

`\r`만 처리하면 Wokwi에서 명령이 입력되지 않는 것처럼 보일 수 있다.

---

# 20. 현재까지 발생했던 주요 오류와 예방책

## 20.1 ui.cpp drawBattery not declared

오류:

```text
drawBattery not declared
```

원인:

```text
drawTopBarSurface()에서 drawBattery()를 호출하지만,
drawBattery() 정의가 아래에 있어 forward declaration이 없었음.
```

해결:

```cpp
static void drawBattery(int x, int y, uint8_t pct);
```

를 `ui.cpp` 상단에 추가하거나, `drawBattery()` 정의를 호출보다 위로 이동한다.

---

## 20.2 getCurrentEpochSec undefined reference

오류:

```text
undefined reference to DiveComputerApp::getCurrentEpochSec() const
```

해결:

`app.h`에 선언되어 있으면 `app.cpp`에 반드시 구현한다.

```cpp
uint32_t DiveComputerApp::getCurrentEpochSec() const {
    return SCENARIO_START_EPOCH + getSimEpochSec();
}
```

또한 `app.cpp`에서 다음 include 필요:

```cpp
#include "generated_scenario.h"
```

---

## 20.3 generated_scenario.h에 GF/Battery 포함 금지

시나리오 generator에서 다음 값은 생성하지 않는다.

```text
SCENARIO_INITIAL_GF_LOW
SCENARIO_INITIAL_GF_HIGH
SCENARIO_INITIAL_BATTERY_PCT
```

GF는 `config.h`에서, 배터리는 프로그램 기본값 또는 mock command로 관리한다.

---

# 21. 구현 우선순위

## 1단계: 현재 코드 안정화

목표:

```text
현재 JSON 시나리오 기반 빌드 안정화
Surface UI 정상 표시
Battery Low 팝업 정상 동작
Charging 표시 정상 동작
Serial command 정상 동작
```

확인 명령:

```bash
pio run -e wokwi
```

---

## 2단계: Surface 변수 구조 정리

목표:

```text
lastDiveStartEpochSec_
lastDiveDurationSec_
lastDiveEndEpochSec_
lastDiveMaxDepthM_
lastDiveMinTempC_
noFlyEndEpochSec_
```

를 중심으로 Surface 계산을 정리한다.

기존의 직접 surface interval preload 방식은 점진적으로 제거한다.

---

## 3단계: JSON preload 구조 변경

기존:

```json
"surfaceIntervalSec": 5040
```

변경:

```json
"lastDive": {
  "startEpoch": 1777500000,
  "durationSec": 2760,
  "maxDepthM": 28.6,
  "minTempC": 22.0,
  "noFlyMinutesAtEnd": 720
}
```

---

## 4단계: generate_scenario.py XML 입력 지원

목표:

```text
Subsurface XML을 읽어 generated_scenario.h 생성
```

지원 항목:

```text
dive date
dive time
duration
sample time
sample depth
sample temp
max depth
min temp
```

---

## 5단계: Compact Log Format 추가

추가 파일:

```text
include/log_format.h
include/log_storage.h
src/log_storage.cpp
```

초기 목표:

```text
다이빙 종료 시 compact log header 생성
sample array 저장 구조 준비
```

---

## 6단계: XML Export Tool 추가

추가 파일:

```text
tools/bdc_to_subsurface_xml.py
```

목표:

```text
compact .bdc 로그를 Subsurface XML로 변환
```

---

## 7단계: BLE 다운로드 설계

초기 방향:

```text
BLE로 compact log 전송
PC/app에서 XML 변환
```

향후:

```text
기기 자체 XML export 지원 여부 검토
```

---

# 22. v7.2 기준 완료 조건

`BackupDiveComputer_v7.2`는 다음 조건을 만족하면 1차 완료로 본다.

## 필수 완료 조건

1. Wokwi 빌드 성공
2. Surface 화면에서 다음 표시 정상
   - LAST
   - MAX
   - TMP
   - SURFACE
   - N-FLY
3. LAST는 `YYYY-MM-DD`만 표시
4. SURFACE는 마지막 다이빙 종료시각 기준 계산
5. N-FLY는 `noFlyEndEpochSec` 기준 계산
6. 충전 중 배터리 아이콘 점멸
7. 충전 완료 시 `FULL` 번갈아 표시
8. 충전기 분리 시 일반 화면 복구
9. Battery Low 팝업 정상 동작
10. GF는 `config.h` 기준 사용
11. 시나리오에서 배터리/GF 값 제거
12. Serial command 정상 입력

---

## 확장 완료 조건

1. JSON preload가 실제 로그 구조 기반으로 변경됨
2. Subsurface XML scenario 입력 지원
3. generated_scenario.h가 XML에서 생성 가능
4. compact log format 초안 추가
5. XML export tool 초안 추가

---

# 23. 새 대화 시작 시 전달할 요약

새 대화에서 이어서 개발할 때는 다음 요약을 함께 전달하면 된다.

```text
BackupDiveComputer_v7.2 개발 중입니다.

핵심 방향:
- 내부 로그는 XML이 아니라 compact binary/record로 저장합니다.
- Export/Import 시 Subsurface XML 호환 구조를 사용합니다.
- 시나리오도 장기적으로 Subsurface XML을 입력으로 사용합니다.
- Surface 화면은 LAST, MAX, TMP, SURFACE, N-FLY를 표시합니다.
- LAST는 LAST DIVE가 아니라 LAST로 표시하고, 값은 YYYY-MM-DD만 표시합니다.
- Surface 시간은 시나리오에 직접 넣지 않고, 마지막 다이빙 startEpoch + duration으로 endEpoch를 계산한 뒤 currentEpoch와 비교해 계산합니다.
- No-Fly도 noFlyEndEpochSec 기준으로 계산합니다.
- GF는 config.h에서 가져오며, 시나리오에 포함하지 않습니다.
- 배터리 값도 시나리오에 포함하지 않습니다.
- Battery Low는 10% 이하에서 2분 간격으로 2초 팝업 표시합니다.
- 충전 중에는 배터리 아이콘을 깜빡이고, 100% 충전 완료 시 FULL과 배터리 아이콘을 번갈아 표시합니다.
- 상승속도 그래프의 Dot Pattern은 위험영역 배경으로 항상 표시되어야 합니다.
- Surface UI는 128x64 OLED에서 큰 폰트, 11px 줄간격, 화면을 꽉 채우는 현재 레이아웃을 기준으로 합니다.
```

---

# 24. 최종 개발 방향 한 줄 요약

```text
BackupDiveComputer_v7.2는 실제 다이브컴퓨터 로그 구조를 기준으로 Surface 표시, 시뮬레이션, 로그 저장, BLE 다운로드, Subsurface XML Export를 하나의 일관된 구조로 통합하는 버전이다.
```
