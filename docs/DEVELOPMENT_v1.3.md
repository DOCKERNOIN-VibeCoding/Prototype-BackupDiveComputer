# BackupDiveComputer_v1.3 개발 정의서

---
이 문서는 `BackupDiveComputer_v1.3`의 개발 방향, 현재 구현 상태, 핵심 정책, 앞으로 구현할 내용을 정리한 개발 문서이다.

코딩 경험이 많지 않은 개발자도 이해할 수 있도록 가능한 한 쉬운 표현을 사용한다.

현재 개발 브랜치:

```text
dev/v1.3
```

대상 프로젝트:

```text
BackupDiveComputer
Prototype-BackupDiveComputer
```

GitHub 저장소:

```text
https://github.com/DOCKERNOIN-VibeCoding/Prototype-BackupDiveComputer
```
---

# 1. 프로젝트 개요

## 1.1 프로젝트명

```text
BackupDiveComputer
```

## 1.2 현재 개발 버전

```text
v1.3
```

## 1.3 현재 개발 브랜치

```text
dev/v1.3
```

## 1.4 프로젝트 목표

이 프로젝트는 ESP32-S3 기반의 소형 백업 다이브 컴퓨터를 만드는 것을 목표로 한다.

주요 기능:

```text
수심 표시
수온 표시
다이빙 시간 표시
상승 속도 표시
안전정지 표시
감압 관련 정보 표시
배터리 상태 표시
충전 상태 표시
다이브 로그 저장
Surface interval 표시
No-Fly 시간 표시
GPS 위치 기록
BLE 또는 USB를 통한 로그 다운로드
Subsurface XML 호환
```

v1.3부터는 다음 정책을 명확히 반영한다.

```text
1. 버튼 없는 스탠드얼론 백업 다이브 컴퓨터
2. 레크리에이션 다이빙용 단일 가스 컴퓨터
3. 기본 가스 Air / EAN21
4. 향후 Nitrox 설정을 위한 FO2 구조 사전 반영
5. Bühlmann 계산은 config의 FO2에서 FN2를 계산해 사용
6. ppO2 max 1.4 bar 기준 MOD 계산
7. FO2 > 21%일 경우 다이빙 중 MOD 상시 표시
8. DECO.STOP은 18m, 15m, 12m, 9m, 6m, 3m ladder 사용
9. 전체 DecoPlan 표시는 하지 않고 현재 필요한 DECO.STOP 하나만 표시
10. 감압 위반 후에도 hard lockout 없이 계산과 경고를 계속 제공
```

---

# 2. 제품 컨셉

## 2.1 물리 버튼 없는 자동 작동 기기

이 기기는 일반 전자기기처럼 버튼을 누르고 켜고 끄는 제품이 아니다.

제품 조건:

```text
완전 밀폐형
스탠드얼론 기기
물리 파워 버튼 없음
물리 메뉴 버튼 없음
사용자 조작 없이 자동 작동
배터리가 방전되면 꺼짐
충전하면 다시 켜짐
Qi 충전 또는 밀폐형 충전 방식 고려
```

따라서 UI와 상태 전환은 모두 자동으로 동작해야 한다.

사용자가 버튼을 눌러 다음 화면으로 이동하거나, 메뉴에 들어가 gas를 바꾸거나, DecoPlan을 확인하는 구조는 사용할 수 없다.

---

## 2.2 백업용 다이브 컴퓨터

이 기기는 메인 다이브 컴퓨터를 대체하기 위한 고급 테크니컬 컴퓨터가 아니다.

목적:

```text
메인 컴퓨터 고장 시 최소한의 핵심 정보 제공
메인 컴퓨터 lockout 시 수심/시간/상승률/감압 정보를 계속 제공
단순하고 읽기 쉬운 경고 제공
작은 화면에서 현재 필요한 행동 지시 제공
```

따라서 기능 철학은 다음과 같다.

```text
복잡한 계획표보다 현재 행동 지시 우선
전체 DecoPlan보다 현재 DECO.STOP 우선
메뉴 조작보다 자동 상태 전환 우선
lockout보다 정보 제공 우선
```

---

# 3. 개발 환경

현재는 실제 하드웨어가 아니라 Wokwi 시뮬레이션 환경에서 개발 중이다.

개발 환경:

```text
PlatformIO
Wokwi
ESP32-S3
Arduino Framework
OLED 128x64
U8G2
JSON scenario
Python generator
```

주요 폴더:

```text
include/
src/
tools/
scenarios/
docs/
```

주요 파일:

```text
include/app.h
include/ui.h
include/config.h
src/app.cpp
src/ui.cpp
tools/generate_scenario.py
scenarios/previous_dive_surface.json
include/generated_scenario.h
platformio.ini
```

---

# 4. 현재 구현된 핵심 기능

## 4.1 Wokwi 시뮬레이션

Wokwi에서 ESP32-S3와 OLED 화면을 시뮬레이션한다.

시뮬레이션에서는 실제 센서 대신 가짜 센서값을 사용한다.

예:

```text
JSON 파일에 적힌 수심과 수온을 시간에 따라 재생한다.
```

---

## 4.2 Surface 화면

Surface 화면은 물 밖에 있을 때 보여주는 화면이다.

현재 Surface 화면은 다음 정보를 표시하는 방향으로 구현되어 있다.

```text
LAST
MAX
TMP
SURFACE
N-FLY
```

의미:

```text
LAST    마지막 다이빙 날짜
MAX     마지막 다이빙 최대 수심
TMP     마지막 다이빙 최저 수온
SURFACE 마지막 다이빙 후 경과 시간
N-FLY   비행 금지 남은 시간
```

---

## 4.3 LAST 표시 정책

기존에는 `LAST DIVE`라고 표시하려 했지만, OLED 화면이 작기 때문에 `LAST`로 줄였다.

LAST 값은 마지막 다이빙의 시작 날짜를 표시한다.

예:

```text
LAST 2025-01-01
```

LAST는 다이빙 종료 시간이 아니라 시작 시간을 기준으로 한다.

이유:

```text
로그북에서는 일반적으로 다이빙 날짜를 시작 날짜로 기록한다.
Subsurface XML도 dive date/time을 다이빙 시작 시각으로 기록한다.
다이빙 종료 시각은 시작 시각 + 다이빙 시간으로 계산 가능하다.
```

## 4.4 2026-05-07 기준 추가 구현 상태

현재 `dev/v1.3` 브랜치에는 v1.3 핵심 정책 중 일부가 코드로 반영되었다.

### Gas / FO2

`include/config.h`에 Air / Nitrox-ready 단일 gas 설정이 추가되었다.

현재 기본값:

```text
FO2 = 21%
ppO2 max = 1.4 bar
FO2 range = 21% ~ 40%
```

기본 gas는 Air / EAN21이다.

중요:

```text
Air는 EAN21이다.
EAN32는 Air가 아니다.
Bühlmann 계산에서 FN2를 하드코딩하지 않고 FO2에서 계산한다.
```

### Bühlmann FN2 연결

`src/buhlmann.cpp`는 기존의 고정 질소 비율 대신 다음 helper를 사용한다.

```text
getGasFO2()
getGasFN2()
```

이제 tissue loading, NDL, GF99, deco calculation, no-fly calculation에서 FO2 기반 FN2를 사용한다.

### MOD / ppO2 helper

다음 helper가 추가되었다.

```text
calculateMODMeters()
calculatePpO2Bar(depthM)
```

현재는 helper와 구조가 준비된 상태이며, FO2 >21%일 때 MOD를 dive UI에 상시 표시하는 작업은 다음 단계로 남아 있다.

### DECO.STOP ladder

v1.3 DECO.STOP ladder가 추가되었다.

```text
18m -> 15m -> 12m -> 9m -> 6m -> 3m
```

raw ceiling은 이 ladder 중 현재 필요한 stop으로 매핑된다.

raw ceiling이 18m보다 깊은 경우에는 18m stop으로 거짓 안내하지 않고 다음과 같이 표시한다.

```text
CEIL >18m
HOLD DEPTH
```

### DECO UI

DECO.STOP 행동 지시는 다음으로 정리되었다.

```text
ASCEND
HOLD
DESCEND
```

화살표는 Unicode 문자가 아니라 `u8g2.drawTriangle()`으로 직접 그린다.

이유:

```text
실제 예정 하드웨어는 font chip이 없는 ST7567 계열 그래픽 LCD일 수 있으므로,
특수문자나 Unicode 표시를 신뢰하지 않는다.
```

### 알람 / 비프

현재 비프 호출 정책:

```text
LOW BATTERY 표시 시 짧게 1회
빠른 상승 위험 시 3회 연속 경고
```

LOW BATTERY 팝업 주기:

```text
10분마다 2초 표시
```

Wokwi 테스트 명령:

```text
beep test
```

# 4.5 현재 기준 다음 개발 단계

우선순위는 아래 순서가 좋습니다.

1. **빌드 확인**
   - `pio run -e wokwi`
   - 또는 VS Code PlatformIO Build 버튼 사용

2. **UI 확인**
   - DECO 상태에서 `ASCEND`, `DESCEND`, `HOLD` 표시 확인
   - `CEIL >18m` 표시 확인
   - 화살표 삼각형이 깨지지 않는지 확인

3. **Buzzer 확인**
   - Wokwi serial monitor에서:
     ```text
     beep test
     ```
   - 결과 : 테스트 완료. Wokwi 버저 정상 작동됨.

4. **MOD 표시 구현**
   - FO2 > 21%일 때 dive 화면에 `EAN32`, `MOD 33m` 같은 표시 추가

5. **MOD 초과 경고 구현**
   - 현재 수심이 MOD보다 깊으면:
     ```text
     PPO2 HIGH
     ASCEND
     ```

6. **missed deco / 48h advisory 상태 머신 구현**
   - `activeDecoViolation`
   - `postViolationAdvisory`
   - `advisoryEndEpochSec`
   - Surface 화면에 `MISSED DECO / NO DIVE 48H`

7. **re-entry tissue state 처리 검증**
   - 감압 위반 후 재입수해도 hard lockout 하지 않음
   - tissue state 유지
   - DECO.STOP 재계산 계속

---

---

# 5. v1.2에서 도입된 epoch 기반 Surface 계산

v1.2에서는 Surface interval과 No-Fly 계산을 실제 제품 구조에 맞게 epoch 기반으로 바꾸었다.

## 5.1 기존 방식의 문제

기존 시나리오는 Surface interval과 No-Fly 시간을 직접 넣는 방식이었다.

예:

```json
"surfaceIntervalSec": 5040,
"noFlyRemainSec": 41520
```

이 방식은 시뮬레이션에서는 편하지만 실제 제품과 맞지 않는다.

실제 다이브 컴퓨터는 시간이 계속 흐르기 때문에 Surface interval과 No-Fly remain은 계속 변해야 한다.

---

## 5.2 새로운 방식

v1.2부터 실제 제품 방식에 맞게 epoch 기반으로 계산한다.

저장하는 값:

```text
마지막 다이빙 시작 시각
마지막 다이빙 시간
마지막 다이빙 종료 시각
No-Fly 종료 시각
```

계산하는 값:

```text
Surface interval
No-Fly remain
```

---

## 5.3 Surface interval 계산

```text
마지막 다이빙 종료 시각 = 마지막 다이빙 시작 시각 + 다이빙 시간
Surface interval = 현재 시각 - 마지막 다이빙 종료 시각
```

예:

```text
마지막 다이빙 시작: 10:00
다이빙 시간: 40분
마지막 다이빙 종료: 10:40
현재 시각: 12:00

Surface interval = 1시간 20분
```

---

## 5.4 No-Fly 계산

```text
No-Fly remain = No-Fly 종료 시각 - 현재 시각
```

예:

```text
No-Fly 종료 시각: 22:00
현재 시각: 12:00

N-FLY = 10시간
```

현재 시각이 No-Fly 종료 시각을 지났다면:

```text
N-FLY = SAFE TO FLY
```

---

# 6. 주요 변수 설명

## 6.1 Surface 관련 변수

`app.cpp`와 `app.h`에서 사용하는 주요 변수:

```cpp
lastDiveStartEpochSec_
lastDiveDurationSec_
lastDiveEndEpochSec_
lastDiveMaxDepthM_
lastDiveMinTempC_
noFlyEndEpochSec_
```

쉽게 말하면:

```text
lastDiveStartEpochSec_  마지막 다이빙 시작 시간
lastDiveDurationSec_    마지막 다이빙 진행 시간
lastDiveEndEpochSec_    마지막 다이빙 종료 시간
lastDiveMaxDepthM_      마지막 다이빙 최대 수심
lastDiveMinTempC_       마지막 다이빙 최저 수온
noFlyEndEpochSec_       비행 금지 종료 시간
```

---

## 6.2 시간 계산 함수

중요 함수:

```cpp
getCurrentEpochSec()
getSurfaceIntervalSec()
getNoFlyRemainSec()
```

역할:

```text
getCurrentEpochSec()
  현재 시각을 epoch 초 단위로 가져온다.

getSurfaceIntervalSec()
  마지막 다이빙 종료 후 몇 초가 지났는지 계산한다.

getNoFlyRemainSec()
  비행 금지 시간이 몇 초 남았는지 계산한다.
```

---

# 7. 시나리오 파일 구조

현재 시뮬레이션은 JSON 파일을 사용한다.

대표 파일:

```text
scenarios/previous_dive_surface.json
```

이 파일에는 다음 정보가 들어간다.

```text
시나리오 이름
설명
시작 시각
타임존
이전 다이빙 정보
GPS 정보
수심/수온 변화 포인트
향후 gas 설정 정보
```

예상 구조:

```json
{
  "name": "Previous Dive Surface Test",
  "description": "Surface screen test with previous dive preload",

  "time": {
    "startEpoch": 1735689600,
    "tzOffsetMin": 540
  },

  "gas": {
    "fo2Percent": 21,
    "ppO2MaxBar": 1.4
  },

  "preload": {
    "enabled": true,
    "diveCount": 3,

    "lastDive": {
      "startEpoch": 1735682100,
      "durationSec": 2460,
      "maxDepthM": 28.6,
      "minTempC": 22.0,
      "noFlyMinutesAtEnd": 776
    },

    "gps": {
      "valid": true,
      "lat": 33.4996,
      "lon": 126.5312,
      "place": "Jeju Test Site"
    }
  },

  "points": [
    { "t": 0, "depth": 0.0, "temp": 24.0 },
    { "t": 120, "depth": 0.0, "temp": 24.0 },
    { "t": 240, "depth": 12.0, "temp": 23.0 },
    { "t": 600, "depth": 12.0, "temp": 22.0 },
    { "t": 1080, "depth": 0.0, "temp": 24.0 }
  ]
}
```

---

# 8. generate_scenario.py의 역할

`tools/generate_scenario.py`는 시뮬레이션을 위한 변환 도구이다.

이 파일은 ESP32 안에서 실행되지 않는다.

실행 위치:

```text
개발 PC
PlatformIO 빌드 전 단계
```

역할:

```text
1. JSON 시나리오 파일을 읽는다.
2. 값이 올바른지 검사한다.
3. include/generated_scenario.h 파일을 만든다.
4. C++ 코드가 읽을 수 있는 상수로 변환한다.
```

예:

```text
JSON의 startEpoch
  ↓
SCENARIO_START_EPOCH
```

```text
JSON의 lastDive.durationSec
  ↓
SCENARIO_PRELOAD_LAST_DURATION_SEC
```

향후 gas 설정도 다음처럼 변환할 수 있다.

```text
JSON의 gas.fo2Percent
  ↓
SCENARIO_GAS_FO2_PERCENT
```

```text
JSON의 gas.ppO2MaxBar
  ↓
SCENARIO_GAS_PPO2_MAX_BAR
```

---

## 8.1 generated_scenario.h는 직접 수정하지 않는다

`include/generated_scenario.h` 파일 맨 위에는 보통 다음과 같은 의미의 주석이 있다.

```text
Auto-generated file. Do not edit manually.
```

뜻:

```text
자동 생성 파일이니 직접 수정하지 말 것
```

수정해야 할 경우:

```text
JSON 파일을 수정한다.
다시 빌드한다.
generated_scenario.h가 자동으로 바뀐다.
```

---

# 9. 실제 제품에서는 어떻게 바뀌는가

현재 시뮬레이션에서는 이전 다이빙 정보를 JSON에서 가져온다.

실제 제품에서는 JSON을 사용하지 않는다.

실제 제품에서는 다음 흐름이 된다.

```text
다이빙 시작
  ↓
센서로 수심/수온 기록
  ↓
다이빙 중 조직 질소량 계산
  ↓
필요 시 NDL / DECO.STOP / MOD 표시
  ↓
다이빙 종료
  ↓
로그를 내부 저장소에 저장
  ↓
다음 부팅 또는 Surface 화면에서 마지막 로그를 읽음
  ↓
LAST / MAX / TMP / SURFACE / N-FLY 표시
```

즉, 데이터 출처만 바뀐다.

```text
현재 시뮬레이션:
  JSON → generated_scenario.h → app.cpp

실제 제품:
  내부 저장소 로그 → app.cpp
```

하지만 계산 로직은 유지한다.

---

# 10. Recreational Single-Gas Air/Nitrox-Ready 정책

## 10.1 기본 방향

BackupDiveComputer는 테크니컬 다이빙용 컴퓨터가 아니라 레크리에이션 다이빙용 백업 컴퓨터이다.

지원하는 gas model:

```text
Single active gas only
Air / EAN21
Nitrox / EANx 구조는 향후 지원 준비
```

지원하지 않는 항목:

```text
Multi-gas switching
Double tank gas management
Trimix
Helium
CCR
Bailout gas
Deco gas switching
```

---

## 10.2 기본 가스

기본 gas는 Air / EAN21이다.

```text
Air / EAN21 = 산소 21%, 질소 79%
```

중요:

```text
EAN32는 Air가 아니다.
EAN32는 산소 32%, 질소 68%인 Nitrox이다.
```

만약 실제로 Air를 마시는데 컴퓨터가 EAN32로 계산하면, 컴퓨터는 질소 흡수를 실제보다 적게 계산한다.

그 결과:

```text
NDL을 실제보다 길게 표시할 수 있음
DECO.STOP 시간을 실제보다 짧게 표시할 수 있음
안전상 위험함
```

따라서 기본값은 반드시 Air / EAN21로 둔다.

---

## 10.3 config.h gas 설정

`include/config.h`에 gas 설정 상수를 둔다.

권장 기본값:

```cpp
#ifndef DIVE_GAS_FO2_PERCENT
#define DIVE_GAS_FO2_PERCENT 21
#endif

#ifndef DIVE_GAS_PPO2_MAX_BAR
#define DIVE_GAS_PPO2_MAX_BAR 1.40f
#endif

#define DIVE_GAS_FO2_MIN_PERCENT 21
#define DIVE_GAS_FO2_MAX_PERCENT 40
```

주의:

```text
DIVE_GAS_FO2_PERCENT는 산소분압이 아니다.
DIVE_GAS_FO2_PERCENT는 산소분율이다.
```

용어:

```text
FO2  = 산소 비율
FN2  = 질소 비율
ppO2 = 산소 부분압
```

예:

```text
Air / EAN21:
FO2 = 0.21
FN2 = 0.79

EAN32:
FO2 = 0.32
FN2 = 0.68
```

---

# 11. Bühlmann 알고리즘과 FO2 / FN2 연결

## 11.1 핵심 원칙

Bühlmann 알고리즘에서 질소 비율 0.79를 하드코딩하지 않는다.

반드시 `config.h`의 FO2 설정을 읽고, 그 값에서 FN2를 계산한다.

예상 코드:

```cpp
float fo2 = DIVE_GAS_FO2_PERCENT / 100.0f;
float fn2 = 1.0f - fo2;
```

계산된 `fn2`를 다음에 사용한다.

```text
tissue nitrogen loading
NDL calculation
decompression ceiling calculation
DECO.STOP duration calculation
surface interval residual nitrogen tracking
No-Fly / desaturation calculation
```

---

## 11.2 향후 Nitrox 설정 구조

현재 기기에는 버튼이 없고 앱도 아직 없기 때문에, 사용자가 기기에서 직접 Nitrox 비율을 변경할 수 없다.

현재 단계:

```text
FO2는 config.h에서 compile-time 설정
기본값은 21%
시뮬레이션 scenario에서 override 가능하도록 구조 검토
```

향후 단계:

```text
앱에서 FO2 설정
NVS / Preferences에 저장
부팅 시 저장된 FO2 로드
다이빙 로그에 FO2 저장
Subsurface XML export에 gas 정보 포함
```

---

# 12. MOD / ppO2 정책

## 12.1 기준 ppO2

최대 산소 부분압 기준은 다음으로 한다.

```text
ppO2 max = 1.4 bar
```

---

## 12.2 MOD 계산식

MOD는 Maximum Operating Depth이다.

계산식:

```text
MOD(m) = ((ppO2Max / FO2) - 1.0) * 10
```

예: EAN32

```text
FO2 = 0.32
ppO2Max = 1.4 bar

MOD = ((1.4 / 0.32) - 1.0) * 10
MOD ≈ 33.7m
```

예: EAN36

```text
FO2 = 0.36
ppO2Max = 1.4 bar

MOD = ((1.4 / 0.36) - 1.0) * 10
MOD ≈ 28.8m
```

---

## 12.3 MOD 표시 정책

FO2가 21%보다 클 경우, 즉 Nitrox 설정일 경우 다이빙 중 MOD를 항상 표시한다.

예:

```text
EAN32
MOD 33m
```

Air / EAN21의 경우 MOD는 일반적인 레크리에이션 범위보다 깊기 때문에, 기본 화면에서 생략하거나 낮은 우선순위로 둘 수 있다.

---

## 12.4 MOD 초과 경고

현재 수심이 계산된 MOD보다 깊어지면 높은 우선순위 경고를 표시한다.

예:

```text
PPO2 HIGH
ASCEND
```

또는:

```text
MOD EXCEEDED
ASCEND
```

이 경고는 컴퓨터 기능을 잠그지 않는다.

즉, 다음 정보는 계속 제공한다.

```text
현재 수심
다이빙 시간
상승 속도
NDL
DECO.STOP
배터리
로그 기록
```

---

# 13. DECO.STOP ladder 정책

## 13.1 기본 철학

BackupDiveComputer는 전체 DecoPlan 표를 보여주지 않는다.

이유:

```text
버튼이 없다.
메뉴 조작이 없다.
백업용 컴퓨터이다.
화면이 작다.
현재 필요한 행동 지시가 더 중요하다.
```

따라서 Shearwater나 OSTC4처럼 전체 감압표를 보여주는 것이 아니라, 상업용 컴퓨터에서 확인되는 3m 단위 감압 구조만 가져온다.

---

## 13.2 지원 DECO.STOP ladder

DECO.STOP은 아래 6단계 수심을 기준으로 한다.

```text
18m
15m
12m
9m
6m
3m
```

가장 깊은 지원 DECO.STOP은 18m이다.

즉, 일반적인 레크리에이션 백업 컴퓨터로서 다음 범위 안에서만 필수 감압 정지를 안내한다.

```text
18m → 15m → 12m → 9m → 6m → 3m
```

---

## 13.3 모든 감압 다이빙이 18m부터 시작하는 것은 아님

감압 부담이 작으면 얕은 stop만 생성한다.

예:

```text
가벼운 감압:
3m
```

또는:

```text
6m → 3m
```

중간 정도의 감압:

```text
9m → 6m → 3m
```

조금 더 큰 감압:

```text
12m → 9m → 6m → 3m
```

큰 감압:

```text
15m → 12m → 9m → 6m → 3m
```

가장 큰 ladder 사용:

```text
18m → 15m → 12m → 9m → 6m → 3m
```

즉, 항상 18m부터 시작하는 것이 아니라, 현재 tissue nitrogen loading과 raw decompression ceiling에 따라 첫 번째 필요한 stop을 선택한다.

---

## 13.4 raw ceiling과 DECO.STOP 매핑

Bühlmann 계산에서 나오는 ceiling은 실제 수학적 감압 천장이다.

이 값은 연속적인 수심값이다.

예:

```text
raw ceiling = 8.4m
raw ceiling = 10.7m
raw ceiling = 2.6m
```

하지만 다이버에게는 이런 연속값을 그대로 지시하지 않는다.

대신, 다음 고정 ladder 중 하나로 매핑한다.

```text
18m
15m
12m
9m
6m
3m
```

매핑 기준:

```text
ceiling <= 3m   → first stop 3m
ceiling <= 6m   → first stop 6m
ceiling <= 9m   → first stop 9m
ceiling <= 12m  → first stop 12m
ceiling <= 15m  → first stop 15m
ceiling <= 18m  → first stop 18m
```

예:

```text
raw ceiling 2.4m  → DECO.STOP 3m
raw ceiling 5.1m  → DECO.STOP 6m
raw ceiling 8.7m  → DECO.STOP 9m
raw ceiling 11.2m → DECO.STOP 12m
raw ceiling 14.6m → DECO.STOP 15m
raw ceiling 17.3m → DECO.STOP 18m
```

---

## 13.5 18m보다 깊은 ceiling 처리

BackupDiveComputer는 레크리에이션 다이빙용 백업 컴퓨터이다.

따라서 일반적으로 18m보다 깊은 mandatory DECO.STOP을 생성하지 않는다.

지원하는 가장 깊은 DECO.STOP은 다음과 같다.

```text
18m
```

하지만 계산상 raw ceiling이 18m보다 깊게 나오는 경우가 있을 수 있다.

예:

```text
raw ceiling = 19.5m
raw ceiling = 21.0m
```

이 경우 컴퓨터가 단순히 다음처럼 표시하면 안 된다.

```text
DECO.STOP
18m
```

왜냐하면 실제 ceiling이 18m보다 깊다면, 18m는 이미 너무 얕은 수심일 수 있기 때문이다.

따라서 이 경우에는 별도 경고 상태로 처리한다.

예상 표시:

```text
CEIL >18m
HOLD
```

또는:

```text
DECO.CEIL
>18m
DO NOT ASCEND
```

이 상태에서는 18m stop을 안전한 것으로 표시하지 않는다.

계산을 계속 진행하다가 raw ceiling이 18m 이하가 되면 그때부터 정상 DECO.STOP ladder로 진입한다.

예:

```text
raw ceiling 17.8m
↓
DECO.STOP
18m
```

이 정책의 목적은 다음과 같다.

```text
지원 ladder는 18m부터 시작한다.
하지만 18m보다 깊은 ceiling을 18m stop으로 속이지 않는다.
```

---

## 13.6 전체 DecoPlan은 표시하지 않음

BackupDiveComputer는 전체 감압표를 표시하지 않는다.

표시하지 않는 예:

```text
18m 1'
15m 1'
12m 2'
9m  3'
6m  5'
3m  8'
TTS 24'
```

이런 전체 계획표는 Shearwater나 OSTC4 같은 버튼/메뉴가 있는 컴퓨터에서는 유용하지만, 이 프로젝트의 제품 전제와 맞지 않는다.

우리 기기는:

```text
버튼 없음
메뉴 없음
작은 OLED
백업용
자동 작동
```

이므로 전체 계획표보다 현재 필요한 행동 지시가 더 중요하다.

따라서 화면에는 항상 현재 수행해야 할 DECO.STOP 하나만 표시한다.

예:

```text
DECO.STOP
12m 1:00
ASCEND
```

12m stop 완료 후:

```text
DECO.STOP
9m 1:00
ASCEND
```

9m stop 완료 후:

```text
DECO.STOP
6m 2:00
ASCEND
```

마지막 stop 완료 후:

```text
DECO CLEAR
SURFACE OK
```

---

# 14. DECO.STOP 실행 정책

## 14.1 상태 흐름

DECO.STOP은 다음 상태 흐름으로 진행한다.

```text
DECO_REQUIRED
↓
FIRST_STOP_SELECTED
↓
CURRENT_DECO_STOP_ACTIVE
↓
STOP_WINDOW_ENTERED
↓
STOP_TIMER_RUNNING
↓
STOP_COMPLETED
↓
NEXT_STOP_SELECTED
↓
DECO_CLEAR
```

각 단계의 의미:

```text
DECO_REQUIRED
  감압 의무가 발생한 상태

FIRST_STOP_SELECTED
  raw ceiling을 기준으로 첫 번째 stop 수심을 선택한 상태

CURRENT_DECO_STOP_ACTIVE
  현재 안내 중인 DECO.STOP이 있는 상태

STOP_WINDOW_ENTERED
  다이버가 stop 수심 범위 안으로 들어온 상태

STOP_TIMER_RUNNING
  감압정지 시간이 실제로 감소하는 상태

STOP_COMPLETED
  현재 stop을 완료한 상태

NEXT_STOP_SELECTED
  다음 얕은 stop이 선택된 상태

DECO_CLEAR
  모든 필수 감압정지를 완료한 상태
```

---

## 14.2 현재 stop만 표시

화면에는 전체 stop list를 표시하지 않는다.

항상 현재 필요한 stop 하나만 표시한다.

예:

```text
DECO.STOP
9m 1:00
ASCEND
```

stop window 안에 들어오면:

```text
DECO.STOP
9m 0:59
HOLD
```

너무 얕게 올라가면:

```text
DECO.STOP
9m 0:59
DOWN!
```

stop 완료 후:

```text
STOP DONE
NEXT 6m
```

잠시 후 자동으로 다음 stop 표시:

```text
DECO.STOP
6m 2:00
ASCEND
```

---

## 14.3 stop window

감압정지 타이머는 다이버가 stop window 안에 있을 때만 감소한다.

예:

```text
DECO.STOP 9m
```

이라면 다이버가 9m 부근의 허용 범위 안에 있을 때만 감압정지 시간이 줄어든다.

권장 초기 기준:

```text
DECO_STOP_WINDOW_M = 0.6m
```

예:

```text
9m stop의 허용 범위:
약 9.6m ~ 8.4m
```

다만 실제 구현에서는 너무 얕은 쪽을 더 엄격하게 처리할 수 있다.

중요한 원칙:

```text
stop보다 깊은 것은 보통 더 보수적이다.
stop보다 얕은 것은 ceiling violation 가능성이 있다.
```

따라서 UI는 다음처럼 동작한다.

```text
현재 수심이 stop보다 많이 깊음:
ASCEND

현재 수심이 stop window 안:
HOLD

현재 수심이 stop보다 너무 얕음:
DOWN!
```

---

## 14.4 stop 완료 조건

현재 stop은 다음 shallower stop으로 상승해도 안전해졌을 때 완료된다.

예:

```text
현재 stop = 18m
다음 stop = 15m
완료 조건 = ceiling <= 15m
```

전체 조건:

```text
18m stop 완료 조건 = ceiling <= 15m
15m stop 완료 조건 = ceiling <= 12m
12m stop 완료 조건 = ceiling <= 9m
9m stop 완료 조건  = ceiling <= 6m
6m stop 완료 조건  = ceiling <= 3m
3m stop 완료 조건  = surface allowed
```

마지막 3m stop의 완료 조건은 수면으로 상승해도 허용되는 상태이다.

즉:

```text
raw ceiling <= 0m
```

또는 Bühlmann/GF 기준으로 수면 도달이 허용되는 상태이다.

---

## 14.5 stop duration은 고정값이 아님

DECO.STOP의 수심은 ladder에서 선택되지만, 시간은 고정값이 아니다.

잘못된 방식:

```text
18m는 항상 1분
15m는 항상 1분
12m는 항상 1분
9m는 항상 1분
6m는 항상 1분
3m는 항상 3분
```

올바른 방식:

```text
현재 tissue nitrogen loading을 기준으로
각 stop에서 얼마나 머물러야 다음 stop으로 갈 수 있는지 계산한다.
```

따라서 같은 9m stop이라도 상황에 따라 시간이 달라질 수 있다.

예:

```text
가벼운 감압:
9m 1:00

더 큰 감압:
9m 3:00

더 오래 머문 다이빙:
9m 5:00
```

---

## 14.6 stop remaining time 예측

다이버에게 남은 시간을 보여주기 위해, 현재 tissue state를 복사하여 예측 계산을 수행한다.

예상 함수:

```cpp
uint16_t estimateStopRemainSec(
    const TissueState& currentTissues,
    float stopDepthM,
    float nextStopDepthM,
    const GasConfig& gas,
    const GradientFactorConfig& gf
);
```

동작:

```text
1. 현재 tissue state를 복사한다.
2. 현재 stopDepthM에 머문다고 가정한다.
3. 10초 또는 30초 단위로 tissue off-gassing을 시뮬레이션한다.
4. raw ceiling이 nextStopDepthM 이하가 되는 시점을 찾는다.
5. 그 시간을 현재 stop의 remainSec으로 표시한다.
```

예:

```text
현재 stop = 9m
다음 stop = 6m
예측 결과 = 130초

표시:
DECO.STOP
9m 2:10
HOLD
```

표시 단위는 OLED 가독성을 위해 1분 단위로 올림 처리할 수 있다.

예:

```text
130초 → 3:00
70초  → 2:00
20초  → 1:00
```

---

## 14.7 stop 진행 중 재계산

DECO.STOP 시간은 고정 countdown이 아니다.

실제 tissue state는 계속 변하고, 다이버의 실제 수심도 계속 변한다.

따라서 stop 중에도 주기적으로 remain time을 재계산한다.

권장 재계산 간격:

```text
10초
```

또는:

```text
30초
```

초기 구현에서는 10초 단위를 권장한다.

주의:

```text
남은 시간이 갑자기 증가하거나 튀는 현상이 생길 수 있다.
```

따라서 UI 안정성을 위해 smoothing 정책을 검토할 수 있다.

예:

```text
남은 시간이 크게 증가할 때는 즉시 반영
남은 시간이 아주 작게 흔들릴 때는 표시 안정화
```

---

## 14.8 더 깊은 stop으로 upgrade 가능

다이버가 아직 실제 stop을 시작하지 않았다면, 계산 결과에 따라 더 깊은 stop으로 upgrade할 수 있다.

예:

처음에는:

```text
DECO.STOP
6m 1:00
ASCEND
```

이후 깊은 수심에서 더 오래 머물러 ceiling이 깊어지면:

```text
DECO.STOP
9m 1:00
ASCEND
```

으로 변경할 수 있다.

이것은 생리학적으로 가능하며, 조직 질소량이 증가했기 때문이다.

하지만 이미 어떤 stop을 수행 중이라면, shallower stop으로 갑자기 jump하지 않는다.

원칙:

```text
아직 stop 시작 전:
  더 깊은 stop으로 upgrade 가능

현재 stop 수행 중:
  현재 stop 완료 전 shallower stop으로 jump 금지

안전상 더 깊은 stop이 필요해진 경우:
  더 깊은 stop으로 재조정 가능
```

---

# 15. DECO.STOP UI 정책

## 15.1 기본 표시

감압 필요 상태에서는 `DECO.STOP`이라는 용어를 유지한다.

이 용어는 다이버에게 다음 의미를 명확히 전달한다.

```text
이 정지는 선택적 안전정지가 아니다.
필수 감압정지이다.
```

기본 표시:

```text
DECO.STOP
9m 1:00
ASCEND
```

---

## 15.2 상태별 표시

### stop으로 이동 중

```text
DECO.STOP
12m 1:00
ASCEND
```

의미:

```text
12m까지 상승하라.
아직 stop window에 들어가지 않았다.
```

---

### stop window 안

```text
DECO.STOP
12m 0:58
HOLD
```

의미:

```text
현재 수심을 유지하라.
감압정지 시간이 감소 중이다.
```

---

### 너무 얕음

```text
DECO.STOP
12m 0:58
DOWN!
```

의미:

```text
너무 얕다.
즉시 더 깊게 내려가라.
감압정지 타이머는 pause된다.
```

---

### stop 완료

```text
STOP DONE
NEXT 9m
```

의미:

```text
현재 stop은 완료되었다.
다음 stop은 9m이다.
```

---

### 모든 stop 완료

```text
DECO CLEAR
SURFACE OK
```

의미:

```text
필수 감압정지는 완료되었다.
수면 상승이 허용된다.
```

---

## 15.3 S-STOP과 DECO.STOP 분리

Safety Stop과 DECO.STOP은 절대 혼동하면 안 된다.

```text
S-STOP:
  선택적 안전정지

DECO.STOP:
  필수 감압정지
```

DECO.STOP이 필요한 다이빙에서는 S-STOP보다 DECO.STOP이 우선이다.

DECO.STOP 진행 중에는 S-STOP을 표시하지 않는다.

---

# 16. 감압 위반 후 재입수 / 48시간 advisory 정책

## 16.1 기본 철학

BackupDiveComputer는 백업용 다이브 컴퓨터이다.

따라서 메인 다이브 컴퓨터가 감압 위반으로 잠기거나 제한되더라도, BackupDiveComputer는 수심, 시간, 상승률, 감압 정보를 계속 제공해야 한다.

이 프로젝트에서는 감압정지를 완료하지 못하고 출수한 경우에도 컴퓨터 기능을 완전히 잠그는 hard lockout은 사용하지 않는다.

대신 다음 정책을 사용한다.

```text
No hard lockout
DECO.STOP 계산 계속
감압 위반 상태 추적
재입수 시 남은 감압 의무 계산
감압 완료 시 active violation clear
48시간 advisory/log 유지
```

---

## 16.2 참고한 상업용 컴퓨터 정책

### Shearwater 계열

Shearwater는 감압정지 위반 후에도 컴퓨터를 잠그지 않는 정책을 사용한다.

핵심 방향:

```text
No lockout for violating deco stops
명확한 경고 제공
컴퓨터 기능은 계속 유지
판단은 훈련받은 다이버에게 맡김
```

이 방식은 백업용 컴퓨터에 중요하다.

이유:

```text
비상 상황에서 컴퓨터가 잠기면 수심, 시간, 감압 정보가 사라질 수 있다.
백업용 컴퓨터는 어떤 상황에서도 정보를 제공해야 한다.
```

### Suunto Nautic 계열

Suunto Nautic은 algorithm deviation 발생 후에도 lock하지 않고, 원래 decompression plan을 계속 표시한다.

또한 required decompression stops가 clear되거나 48시간이 지나면 경고 상태가 해제되는 구조를 가진다.

이 프로젝트는 이 부분을 참고한다.

---

## 16.3 BackupDiveComputer 최종 정책

BackupDiveComputer는 다음 혼합 정책을 사용한다.

```text
Shearwater-style:
  hard lockout 없음
  감압 계산 계속
  정보 제공 계속

Suunto-style:
  감압 미해결 상태 추적
  재입수 후 필요한 감압정지를 완료하면 active violation clear

BackupDiveComputer-specific:
  위반 이력은 로그에 영구 저장
  48시간 동안 표면 화면에 advisory 유지
```

---

## 16.4 상태 구분

감압 위반 후 상태는 두 가지로 나눈다.

```text
activeDecoViolation
postViolationAdvisory
```

### activeDecoViolation

현재 감압 의무가 해결되지 않은 상태이다.

발생 조건 예:

```text
DECO.STOP이 필요한 상태에서
감압정지를 완료하지 않고
수면으로 올라와 다이빙이 종료됨
```

이 상태에서는 재입수 시 DECO.STOP을 다시 계산하고 표시해야 한다.

재입수 후 필요한 감압정지를 모두 완료하면:

```text
activeDecoViolation = false
```

로 변경할 수 있다.

### postViolationAdvisory

감압 위반 이력이 있으므로 주의가 필요한 상태이다.

이 상태는 activeDecoViolation이 clear되어도 바로 사라지지 않는다.

권장 유지 시간:

```text
48 hours
```

표면 화면에서는 다음과 같은 경고를 표시한다.

```text
DIVE WARN
47:58
DECO VIOL
```

또는:

```text
NO DIVE ADVISED
47:58
```

---

## 16.5 표면 화면 동작

감압정지를 완료하지 못하고 출수하면 표면 화면에 강한 경고를 표시한다.

예:

```text
MISSED DECO
NO DIVE
48H
```

또는:

```text
DECO VIOL
NO DIVE ADVISED
48H
```

이 표시는 사용자가 다음 다이빙을 하지 않도록 강하게 경고하기 위한 것이다.

하지만 컴퓨터 기능 자체는 잠그지 않는다.

---

## 16.6 재입수 시 동작

48시간 advisory 중에 사용자가 다시 입수한 경우, 이미 물속에 있으므로 `NO DIVE` 문구보다 실제 행동 지시가 우선이다.

재입수 시에는 다음 정보를 계속 제공한다.

```text
현재 수심
다이빙 시간
상승 속도
DECO.STOP
ceiling warning
MOD / ppO2 warning
배터리
```

화면 예:

```text
VIOL
DECO.STOP
9m 3:00
ASCEND
```

정지 수심에 도착하면:

```text
VIOL
DECO.STOP
9m 2:59
HOLD
```

너무 얕게 올라가면:

```text
VIOL
DECO.STOP
9m
DOWN!
```

---

## 16.7 감압 완료 시 동작

재입수 후 필요한 감압정지를 모두 완료하면 현재 감압 미해결 상태는 clear한다.

```text
activeDecoViolation = false
```

하지만 위반 이력은 로그에 남긴다.

또한 48시간 advisory는 계속 유지할 수 있다.

예:

```text
DECO CLEARED
DIVE WARN 46H
```

이유:

```text
재입수 후 감압 계산이 clear되었다고 해도
의학적으로 완전히 안전하다는 뜻은 아니다.
장비는 치료 장비가 아니며,
DAN 등은 감압 위반 후 재입수를 일반적으로 권장하지 않는다.
```

따라서 BackupDiveComputer는 다음 균형을 유지한다.

```text
감압 계산은 계속 제공한다.
필요한 DECO.STOP은 안내한다.
하지만 위반 이력과 주의 경고는 유지한다.
```

---

## 16.8 로그 저장 정책

감압 위반과 재입수 감압 처리 과정은 compact log event로 저장해야 한다.

필수 이벤트:

```text
EVENT_DECO_REQUIRED
EVENT_DECO_STOP_STARTED
EVENT_DECO_STOP_COMPLETED
EVENT_DECO_CEILING_VIOLATED
EVENT_DECO_MISSED
EVENT_DECO_VIOLATION_SURFACED
EVENT_DECO_REENTRY
EVENT_DECO_CLEARED_AFTER_REENTRY
EVENT_POST_VIOLATION_ADVISORY_STARTED
EVENT_POST_VIOLATION_ADVISORY_ENDED
```

로그에는 다음 정보도 포함하는 것이 좋다.

```text
violationStartEpochSec
violationEndEpochSec
advisoryEndEpochSec
missedStopDepthM
missedStopRemainSec
reentryCount
clearedAfterReentry
```

---

## 16.9 주의

이 기능은 재입수를 권장하기 위한 기능이 아니다.

목적은 다음이다.

```text
이미 재입수한 다이버에게 필요한 정보를 계속 제공한다.
메인 컴퓨터가 잠겼을 때 백업 컴퓨터 역할을 유지한다.
감압 미해결 상태를 추적하고, 가능한 경우 계산상 clear 상태를 판단한다.
```

의학적 판단이나 재입수 여부는 다이버의 훈련, 현장 상황, 응급 절차, DAN 또는 의료기관 지침을 따라야 한다.

---

# 17. Safety Stop 정책

## 17.1 S-STOP은 선택적 안전정지

Safety Stop은 DECO.STOP과 다르다.

```text
S-STOP:
  선택적 안전정지

DECO.STOP:
  필수 감압정지
```

S-STOP은 일반적인 무감압 다이빙 후 3m~6m 부근에서 수행하는 안전정지이다.

DECO.STOP이 필요한 경우에는 S-STOP을 별도로 표시하지 않는다.

---

## 17.2 S-STOP PAUSED / SKIPPED 정책

출수 또는 너무 얕은 수심으로 이동해 safety stop window를 벗어난 경우:

```text
S-STOP PAUSED
```

를 표시하고, 일정 시간 동안 다이버가 안전정지 수심으로 돌아오기를 기다린다.

기다리는 시간 내에 돌아오지 않으면:

```text
S-STOP SKIPPED
```

를 표시한다.

그 후 약 30초 정도 후에 Surface mode로 전환한다.

기존 논의에서 유지할 숫자:

```text
3.0m
0.6m
30 seconds
```

---

# 18. UI 우선순위

버튼 없는 자동 컴퓨터이므로 화면 우선순위가 중요하다.

권장 우선순위:

```text
1. 생명/안전 관련 즉시 경고
2. DECO.STOP 행동 지시
3. MOD / ppO2 경고
4. 상승 속도 경고
5. S-STOP
6. NDL / 일반 dive 정보
7. 배터리 / 충전 정보
8. Surface 정보
```

예:

```text
DECO.STOP과 S-STOP이 동시에 조건을 만족하면:
DECO.STOP 표시

MOD 초과와 일반 NDL 정보가 동시에 있으면:
MOD 초과 경고 우선

감압 위반 후 재입수 중이면:
NO DIVE 문구보다 DECO.STOP 행동 지시 우선
```

---

# 19. 로그 저장 방향

## 19.1 내부 저장은 compact format 사용

실제 제품 내부에는 XML을 저장하지 않는다.

이유:

```text
XML은 용량이 크다.
ESP32 저장공간에 부담이 된다.
BLE로 보내기에도 크다.
파싱이 복잡하다.
```

따라서 내부 저장은 compact binary format으로 한다.

---

## 19.2 향후 추가할 파일

예정 파일:

```text
include/log_format.h
include/log_storage.h
src/log_storage.cpp
```

---

## 19.3 로그에 저장할 정보

다이빙 로그에는 다음 정보가 들어가야 한다.

```text
다이빙 번호
시작 시각
종료 시각
다이빙 시간
최대 수심
평균 수심
최저 수온
샘플 개수
이벤트 개수
No-Fly 종료 시각
GPS 위치
timeSessionId
timeStatus
gas FO2
ppO2 max
deco violation flag
post violation advisory 정보
```

---

## 19.4 이벤트 로그

이벤트 후보:

```text
EVENT_DECO_REQUIRED
EVENT_DECO_STOP_STARTED
EVENT_DECO_STOP_COMPLETED
EVENT_DECO_CEILING_VIOLATED
EVENT_DECO_MISSED
EVENT_DECO_VIOLATION_SURFACED
EVENT_DECO_REENTRY
EVENT_DECO_CLEARED_AFTER_REENTRY
EVENT_POST_VIOLATION_ADVISORY_STARTED
EVENT_POST_VIOLATION_ADVISORY_ENDED
EVENT_MOD_EXCEEDED
EVENT_PPO2_HIGH
EVENT_SAFETY_STOP_PAUSED
EVENT_SAFETY_STOP_SKIPPED
```

---

# 20. 시간 동기화 정책

## 20.1 GPS 시간이 없을 수 있다

실내 다이빙장이나 수영장에서는 GPS 시간이 안 잡힐 수 있다.

이 경우 실제 날짜/시간을 모른다.

하지만 다이빙이 몇 분 동안 진행되었는지는 알 수 있다.

따라서 다음 정보를 저장한다.

```text
startEpochSec = 0
endEpochSec = 0
startSessionMs
endSessionMs
durationSec
timeSessionId
timeStatus = RelativeOnly
```

---

## 20.2 나중에 GPS 시간이 잡히면 보정

다이빙 후 밖으로 나가거나 충전 중에 GPS 시간이 잡히면 로그 시간을 보정할 수 있다.

단, 조건이 있다.

```text
로그의 timeSessionId와 현재 timeSessionId가 같아야 한다.
```

같으면:

```text
millis 또는 session elapsed time 기준으로 실제 시간을 역산할 수 있다.
```

다르면:

```text
시간 연결이 끊긴 것이므로 보정하지 않는다.
```

---

## 20.3 timeStatus

로그 시간 상태는 다음처럼 구분한다.

```cpp
enum LogTimeStatus {
    TimeSynced,
    RelativeOnly,
    TimeCorrected,
    SyncFailed
};
```

의미:

```text
TimeSynced
  다이빙 당시 실제 시간이 이미 있었다.

RelativeOnly
  실제 시간은 없고 경과 시간만 있다.

TimeCorrected
  나중에 GPS/BLE 시간으로 보정했다.

SyncFailed
  보정할 수 없다.
```

---

## 20.4 48시간 advisory와 시간

48시간 advisory는 가능하면 epoch time을 기준으로 계산한다.

```text
advisoryEndEpochSec = violationEpochSec + 48 hours
```

만약 epoch time이 없다면 상대 시간 기반으로 유지한다.

예:

```text
advisoryRemainSec = 48h - elapsedSinceViolation
```

GPS 시간이 나중에 잡히면 advisory 종료 시각을 보정할 수 있다.

---

# 21. bootCount와 timeSessionId

## 21.1 bootCount

bootCount는 몇 번 부팅했는지를 기록하는 값이다.

용도:

```text
진단용
디버깅용
리셋 횟수 확인용
```

하지만 로그 시간 보정 기준으로는 부족하다.

---

## 21.2 timeSessionId

timeSessionId는 시간이 이어지는 세션을 구분하는 값이다.

용도:

```text
GPS 시간이 없던 로그를 나중에 보정할 수 있는지 판단
```

추천:

```text
bootCount는 유지하되,
로그 시간 보정에는 timeSessionId를 사용한다.
```

---

# 22. 전원 관리 정책

## 22.1 기본 방향

이 제품은 물리 전원 버튼이 없으므로, 완전히 꺼지는 방식보다는 저전력 대기 상태를 유지하는 방향이 적합하다.

기본 대기 상태:

```text
화면 OFF
GPS OFF
BLE OFF
Wi-Fi OFF
ESP32 Light Sleep
필요한 센서만 저전력 유지
```

---

## 22.2 GPS 사용 시점

GPS는 전력 소모가 크므로 항상 켜두면 안 된다.

사용 시점:

```text
출수 직후
충전 중
필요한 경우 시간 동기화 시도
```

대기 중:

```text
GPS OFF
```

---

## 22.3 BLE 사용 시점

BLE도 대기 중에는 끄는 것이 좋다.

사용 시점:

```text
충전 중
로그 다운로드 중
설정 변경 중
```

대기 중:

```text
BLE OFF
```

---

## 22.4 화면 사용 시점

화면은 전력 소모가 큰 부품이다.

켜지는 시점:

```text
다이빙 중
충전 중
Qi 터치로 잠깐 Wake
경고 표시 필요 시
```

대기 중:

```text
Display OFF 또는 Power Save
```

---

## 22.5 Deep Sleep 사용 여부

현재 방향:

```text
기본 대기 모드는 Light Sleep
Deep Sleep은 가능하면 사용하지 않음
```

이유:

```text
Deep Sleep 후에는 millis()가 초기화될 수 있다.
시간 연속성이 끊길 수 있다.
GPS 시간이 없는 로그 보정이 어려워진다.
tissue/advisory continuity가 복잡해진다.
```

단, 배터리가 매우 부족한 경우에는 예외적으로 Deep Sleep 또는 보호 셧다운을 사용할 수 있다.

---

# 23. 배터리 시간 예상

500mAh 배터리를 사용할 경우 실제 사용 시간은 평균 전류에 따라 달라진다.

계산식:

```text
사용 가능 시간 = 배터리 용량 / 평균 전류
```

예를 들어 실사용 가능 용량을 약 400mAh로 보면:

```text
0.5mA 평균 전류 → 약 33일
1mA 평균 전류   → 약 16일
2mA 평균 전류   → 약 8일
3mA 평균 전류   → 약 5.5일
5mA 평균 전류   → 약 3.3일
10mA 평균 전류  → 약 1.6일
```

현재 설계처럼 GPS, BLE, 화면을 대부분 꺼두면 대기 시간이 크게 늘어난다.

단, 실제 지속 시간은 반드시 실제 하드웨어에서 전류 측정으로 확인해야 한다.

---

# 24. BLE / 앱 연동 준비

현재 기기는 버튼이 없으므로 향후 설정 변경은 앱을 통해 수행한다.

BLE는 기본적으로 항상 켜두지 않는다.

권장 정책:

```text
충전 중 BLE 활성화
로그 다운로드 중 BLE 활성화
설정 변경 중 BLE 활성화
대기 중 BLE OFF
```

향후 앱에서 설정할 항목:

```text
FO2
ppO2 max 설정 여부
시간 동기화
로그 다운로드
장치 정보 확인
```

FO2 설정 흐름:

```text
앱에서 EAN32 선택
↓
BLE로 FO2 = 32 전송
↓
NVS / Preferences 저장
↓
다음 다이빙부터 FO2 = 32로 Bühlmann 계산
↓
로그에 FO2 = 32 기록
```

---

# 25. Subsurface XML 호환 방향

## 25.1 내부 저장과 외부 호환 분리

내부 저장:

```text
compact binary log
```

외부 호환:

```text
Subsurface XML
```

---

## 25.2 Export 흐름

향후 목표:

```text
내부 로그
  ↓
XML 변환 도구
  ↓
Subsurface XML
  ↓
Subsurface에서 import
```

예정 도구:

```text
tools/bdc_to_subsurface_xml.py
```

---

## 25.3 XML에 포함할 정보

Subsurface XML export에는 다음 정보를 포함하는 방향으로 한다.

```text
dive date/time
duration
sample time/depth/temp
GPS site
gas FO2
cylinder/gas 기본값
DECO.STOP event
MOD warning event
감압 위반 event
```

---

# 26. XML scenario parser

향후에는 Subsurface XML 파일을 시뮬레이션 입력으로 사용할 수 있게 한다.

예정 기능:

```text
XML에서 dive date/time 읽기
XML에서 duration 읽기
XML에서 sample time/depth/temp 읽기
XML에서 gas FO2 읽기 검토
XML에서 maxDepth/minTemp 계산
generated_scenario.h 생성
```

---

# 27. PlatformIO 빌드 설정

`platformio.ini`에서 Wokwi 시뮬레이션 환경은 다음 빌드 플래그를 사용한다.

```ini
-D WOKWI_SIMULATION
-D ARDUINO_USB_CDC_ON_BOOT=1
-D CORE_DEBUG_LEVEL=3
```

시나리오 자동 생성:

```ini
extra_scripts = pre:tools/generate_scenario.py
```

시나리오 선택:

```ini
custom_scenario = scenarios/previous_dive_surface.json
```

---

# 28. 빌드 방법

VS Code에서 PlatformIO가 설치되어 있다면:

```text
PlatformIO 아이콘 클릭
PROJECT TASKS
wokwi
General
Build
```

터미널에서:

```bash
pio run -e wokwi
```

PowerShell에서 `pio` 명령을 못 찾으면:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e wokwi
```

성공하면 다음과 비슷한 문구가 나온다.

```text
SUCCESS
```

---

# 29. GitHub 업로드 방법

문서를 수정한 뒤 다음 순서로 업로드한다.

```bash
git status
```

수정된 파일 확인 후:

```bash
git add docs/CHANGELOG.md docs/CHAT_CONTEXT_v1.3.md docs/DEVELOPMENT_v1.3.md docs/TODO.md
```

커밋:

```bash
git commit -m "Clean up v1.3 docs and clarify FO2 MOD policy"
```

푸시:

```bash
git push origin dev/v1.3
```

---

# 30. 현재 완료된 항목

```text
Surface 화면 표시 항목 정리
LAST / MAX / TMP / SURFACE / N-FLY 구조 정리
LAST를 마지막 다이빙 시작 날짜 기준으로 정리
epoch 기반 Surface interval 계산 반영
epoch 기반 No-Fly remain 계산 반영
previous_dive_surface.json preload 구조 정리
generate_scenario.py 자동 생성 구조 개선
generated_scenario.h에 preload constants 추가
ui.h Surface 함수 인자 정리
ui.cpp Surface 표시 형식 정리
app.cpp PostDive No-Fly 계산 정리
Safety Stop skipped flow before dive end 수정
감압 위반 후 재입수 / 48시간 advisory 정책 문서화
DECO.STOP 18/15/12/9/6/3 ladder 정책 문서화
Air/Nitrox-ready single gas 정책 문서화
MOD / ppO2 1.4 bar 정책 문서화
```

---

# 31. 앞으로 개발할 항목

```text
include/config.h에 FO2 / ppO2 설정 상수 추가
Bühlmann 계산에서 FN2 하드코딩 제거
FO2 기반 tissue nitrogen loading 구조 추가
NDL 계산에 FN2 반영
MOD 계산 helper 추가
FO2 > 21%일 때 MOD 상시 표시
MOD 초과 warning 추가
DECO.STOP ladder 코드 반영
DECO.STOP remain time 예측 계산 추가
감압 위반 / 재입수 상태 구조 추가
compact log format 구현
log_format.h 추가
log_storage.h / log_storage.cpp 추가
다이빙 종료 시 내부 로그 저장
부팅 시 마지막 로그 읽기
timeSessionId 기반 시간 보정
GPS 시간 동기화 상태 처리
BLE 로그 다운로드
앱 기반 FO2 설정 구조 준비
Subsurface XML export tool
Subsurface XML scenario parser
실제 하드웨어 전류 측정
GPS/BLE/display 전원 제어 검증
Light Sleep 전력 측정
배터리 부족 시 보호 동작 구현
```

---

# 32. v1.3 완료 기준

## 32.1 문서 완료 기준

```text
DEVELOPMENT_v1.3.md가 v1.3 기준으로 정리됨
TODO.md가 v1.3 기준으로 정리됨
CHAT_CONTEXT_v1.3.md가 최신 정책을 반영함
CHANGELOG.md에 v1.3 변경점 기록됨
Markdown 코드블록 오류 없음
중복 TODO 항목 없음
GitHub dev/v1.3에 push 완료
```

---

## 32.2 코드 완료 기준

```text
pio run -e wokwi 성공
Wokwi 시뮬레이션 실행 성공
Surface 화면 정상 표시
LAST/MAX/TMP/SURFACE/N-FLY 정상 표시
JSON preload 정상 동작
No-Fly epoch 계산 정상
Surface interval epoch 계산 정상
Battery Low 팝업 정상
Charging UI 정상
config.h에 FO2 / ppO2 설정 구조 추가
Bühlmann 계산에서 FN2 하드코딩 제거 준비
MOD 계산 helper 추가
FO2 > 21%일 때 MOD 표시 구조 준비
DECO.STOP ladder 정책 코드 반영 준비
감압 위반 / 재입수 상태 구조 반영 준비
```

---

# 33. 최종 방향 요약

BackupDiveComputer_v1.3의 개발 방향은 다음과 같다.

```text
BackupDiveComputer는 버튼 없는 스탠드얼론 백업 다이브 컴퓨터이다.

기본 gas는 Air / EAN21이다.
하지만 향후 Nitrox 지원을 위해 FO2 설정 구조를 미리 둔다.
Bühlmann 계산은 config.h의 FO2에서 FN2를 계산해 사용해야 한다.
ppO2 max는 1.4 bar 기준이다.
FO2 > 21%이면 다이빙 중 MOD를 항상 표시해야 한다.

DECO.STOP은 전체 DecoPlan 표를 보여주지 않는다.
대신 18m, 15m, 12m, 9m, 6m, 3m ladder 중
현재 필요한 DECO.STOP 하나만 표시한다.
각 stop의 시간은 실시간 tissue nitrogen loading으로 계산한다.

감압 위반 후에도 hard lockout은 하지 않는다.
재입수 시 DECO.STOP 계산을 계속 제공한다.
단, 48시간 advisory와 위반 이력은 유지한다.

이 프로젝트는 테크니컬 다이빙 컴퓨터가 아니라
레크리에이션 다이빙용 단일 가스 백업 컴퓨터이다.
```
