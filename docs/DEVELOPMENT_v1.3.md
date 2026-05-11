# BackupDiveComputer v1.3 Development Definition

이 문서는 `BackupDiveComputer` 프로젝트의 `dev/v1.3` 개발 기준 문서이다.

현재 문서 기준 스냅샷:

```text
v1.3.5-dev
```

현재 개발 브랜치:

```text
dev/v1.3
```

GitHub Repository:

```text
https://github.com/DOCKERNOIN-VibeCoding/Prototype-BackupDiveComputer
```

---

# 0. Safety Notice

이 프로젝트는 실험용 소프트웨어/하드웨어 프로토타입이다.

```text
실제 다이빙에서 사용하면 안 된다.
실제 다이빙 안전 판단에 사용하면 안 된다.
주 컴퓨터, 보조 컴퓨터, 백업 컴퓨터로 실제 사용하면 안 된다.
감압 계산, NDL, 상승 속도 경고, Safety Stop, DECO.STOP, No-Fly 계산은
검증/인증/의학적 평가를 받은 것이 아니다.
```

본 프로젝트의 목적은 다음에 한정된다.

```text
소프트웨어 개발
UI 프로토타이핑
Wokwi 시뮬레이션
알고리즘 구조 실험
하드웨어 프로토타입 설계
```

---

# 1. 프로젝트 개요

## 1.1 프로젝트명

```text
BackupDiveComputer
```

## 1.2 현재 개발 버전

```text
v1.3.5-dev
```

`v1.3.5-dev`는 정식 안정판이 아니라 `v1.3` 개발 중간 스냅샷이다.

## 1.3 현재 개발 브랜치

```text
dev/v1.3
```

## 1.4 제품 목표

이 프로젝트는 ESP32-S3 기반의 소형 백업 다이브 컴퓨터를 만드는 것을 목표로 한다.

주요 기능 목표:

```text
수심 표시
수온 표시
다이빙 시간 표시
상승 속도 표시
NDL 표시
Safety Stop 표시
DECO.STOP 표시
Surface interval 표시
No-Fly 표시
배터리 상태 표시
충전 상태 표시
다이브 로그 저장
GPS 위치 기록
BLE 또는 USB를 통한 로그 다운로드
Subsurface XML export 준비
```

---

# 2. 제품 철학

## 2.1 버튼 없는 스탠드얼론 장치

BackupDiveComputer는 버튼 없는 자동 작동형 장치로 설계한다.

제품 조건:

```text
완전 밀폐형
물리 전원 버튼 없음
물리 메뉴 버튼 없음
사용자 조작 없이 자동 상태 전환
충전하면 켜짐
배터리가 방전되면 꺼짐
Qi 충전 또는 밀폐형 충전 구조 고려
```

따라서 UI와 상태 전환은 모두 자동이어야 한다.

금지 또는 비권장:

```text
버튼으로 화면 넘기기
버튼으로 gas 변경
버튼으로 DecoPlan 확인
버튼으로 메뉴 진입
```

---

## 2.2 백업용 다이브 컴퓨터

이 장치는 메인 다이브 컴퓨터를 대체하는 고급 테크니컬 컴퓨터가 아니다.

목적:

```text
메인 컴퓨터 고장 시 최소한의 핵심 정보 제공
메인 컴퓨터 lockout 시에도 수심/시간/감압 정보를 계속 제공
작은 화면에서 현재 필요한 행동 지시 제공
복잡한 계획표보다 즉시 필요한 정보 우선
```

핵심 철학:

```text
lockout보다 정보 제공 우선
전체 DecoPlan보다 현재 DECO.STOP 우선
메뉴보다 자동 판단 우선
복잡한 표시보다 읽기 쉬운 경고 우선
```

# 2.3 하드웨어 제어 소프트웨어의 주요 구동 원칙
- 다이빙컴퓨터가 Qi 충전중일 떄
    A. GPS를 켜서 위치 및 시간 정보를 얻을때까지 재시도 한다.
    B. BLE를 켜서 켜서 스마트폰의 앱과의 연결을 항시 준비한다.
    C. Qi 충전기와 분리시에는 GPS와 BLE 모두 off 한다.

- 다이빙이 끝나고 출수 한 후
    A. 출수한 후, GPS를 켜서 위치 및 시간 정보를 얻기 위해 시도한다. (30초씩 6회 시도 후, 실패시 off)

- 다이빙컴퓨터를 잠깐 Qi 충전기에 갖다 대었을 때
    A. Sleep모드에서 꺠어나 Surface모드로 진입한다. (진입시 Splash 화면 잠시 표시)
    B. 출수시와 동일하게, Sleep모드에서 꺠어나 Surface모드로 진입시 GPS 수신을 시도 한다. (30초씩 6회 시도 후, 실패시 off)

- 다이빙 로그기록 시간 업데이트 방법
    A. 현재 다이빙컴퓨터가 RTS (Real Time Sync)가 이루어진 상태라면, 다이빙 중 실제 시간을 반영하여 다이브 로그에 실제 시간 기록
    B. 만약 RTS가 되지 않은 상태에서 다이빙을 마쳤을 때, 다이빙 출수 후 Surface모드에서 GPS를 통해 RTS가 이루어지면 직전 다이빙 기록을 검토하여 실제 시간으로 수정하여 기록합니다.
    C. 다이빙 컴퓨터는 전원이 꺼졌다가 다시 켜질때마다 부팅 카운터를 기록하고, 부팅 후 지난 시간을 항시 기록한다.
      C-1. 다이빙 컴퓨터에 현재 RTS가 이루어지지 않은 상태일 경우, 다이브로그에는 현재 다이브컴퓨터의 '부팅카운터'와 '부팅 후 지난 시간'을 별도 필드에 저장한다.
      C-2. 다이빙 컴퓨터가 RTS가 이루어질 경우, 이전 다이브로그를 확인하여, 로그내 '부팅카운터'가 현재 다이브컴퓨터의 '부팅카운터'가 일치할 경우에, 부팅후 지난 시간을 현재시간을 기준 역산하여 다이브 로그에 실제 시간을 계산하여 다이빙 시간 필드에 실제 시간으로 정정하여 기록한다.
      C-3. 만약 '부팅 카운터'가 다르다면, 이는 해당 로그를 작성한 시점이 다이빙컴퓨터가 새로 재부팅된 이전의 기록이므로 정확한 시간을 역산할 수 없으므로 (컴퓨터가 어느시간만큼 꺼져있었는지 추정할 수 없으므로) 해당 로그의 다이빙 시간은 수정하지 않는다.

---

# 3. 개발 환경

현재 개발은 Wokwi 시뮬레이션과 PlatformIO를 기준으로 한다.

```text
PlatformIO
Wokwi
ESP32-S3
Arduino Framework
OLED 128x64
U8G2
JSON scenario
Python scenario generator
LittleFS / RAM fallback log storage
```

주요 폴더:

```text
include/
src/
docs/
tools/
scenarios/
```

주요 파일:

```text
include/app.h
include/buhlmann.h
include/config.h
include/log_format.h
include/log_storage.h
include/ui.h

src/app.cpp
src/buhlmann.cpp
src/log_storage.cpp
src/mock_services.cpp
src/sim_sensor.cpp
src/ui.cpp

tools/generate_scenario.py
scenarios/*.json
platformio.ini
```

---

# 4. 현재 v1.3.5-dev 구현 상태 요약

`v1.3.5-dev` 기준으로 다음 항목이 코드에 반영되어 있다.

## 4.1 Firmware version

`include/config.h`:

```cpp
#define FW_NAME     "BackupDiveComputer"
#define FW_VERSION  "v1.3.5-dev"
```

---

## 4.2 Surface 화면

Surface 화면은 다음 정보를 표시한다.

```text
LAST
MAX
TMP
SURFACE
N-FLY
```

의미:

```text
LAST    마지막 다이빙 시작 날짜
MAX     마지막 다이빙 최대 수심
TMP     마지막 다이빙 최저 수온
SURFACE 마지막 다이빙 종료 후 경과 시간
N-FLY   비행 금지 남은 시간
```

감압 위반 advisory가 있는 경우에도 Surface 정보를 막지 않는다.

Surface 화면 마지막 줄은 다음 정보를 교대로 표시한다.

```text
N-FLY
DECO.VIOL
```

교대 주기:

```text
2초
```

---

## 4.3 Epoch 기반 Surface 계산

Surface interval과 No-Fly는 epoch 기반으로 계산한다.

주요 변수:

```cpp
lastDiveStartEpochSec_
lastDiveDurationSec_
lastDiveEndEpochSec_
lastDiveMaxDepthM_
lastDiveMinTempC_
noFlyEndEpochSec_
```

주요 함수:

```cpp
getCurrentEpochSec()
getSurfaceIntervalSec()
getNoFlyRemainSec()
```

계산:

```text
Surface interval = 현재 epoch - 마지막 다이빙 종료 epoch
No-Fly remain    = No-Fly 종료 epoch - 현재 epoch
```

---

## 4.4 Compact log preload

부팅 또는 Surface 진입 시 저장된 마지막 로그가 있으면 이를 먼저 사용한다.

현재 구조:

```text
logStorage.loadLastDive()
↓
DiveLogHeader 로드
↓
Surface 화면 LAST/MAX/TMP/N-FLY 복원
```

저장소:

```text
실제 하드웨어: LittleFS
Wokwi simulation: RAM fallback 가능
```

현재 저장 수준:

```text
DiveLogHeader 저장/로드 구현됨
샘플 전체 저장은 아직 미완성
이벤트 영구 저장은 아직 미완성
```

---

# 5. Air / Nitrox-ready 단일 gas 정책

## 5.1 기본 gas

기본 gas는 Air / EAN21이다.

```text
Air = EAN21
FO2 = 21%
FN2 = 79%
```

중요:

```text
EAN32는 Air가 아니다.
EAN32는 산소 32% Nitrox이다.
```

---

## 5.2 지원 범위

현재 프로젝트는 레크리에이션 단일 gas 컴퓨터이다.

지원:

```text
Single gas
Air / EAN21
향후 Nitrox / EANx 준비
FO2 21% ~ 40%
```

지원하지 않음:

```text
Multi-gas switching
Trimix
Heliox
CCR
pSCR
Bailout gas planning
Double tank gas management
Technical deco gas switch
```

---

## 5.3 config.h gas 설정

`include/config.h`:

```cpp
#ifndef DIVE_GAS_FO2_PERCENT
#define DIVE_GAS_FO2_PERCENT       21
#endif

#ifndef DIVE_GAS_PPO2_MAX_BAR
#define DIVE_GAS_PPO2_MAX_BAR      1.40f
#endif

#define DIVE_GAS_FO2_MIN_PERCENT   21
#define DIVE_GAS_FO2_MAX_PERCENT   40
```

compile-time range check:

```cpp
#if DIVE_GAS_FO2_PERCENT < DIVE_GAS_FO2_MIN_PERCENT
#error "DIVE_GAS_FO2_PERCENT must be >= 21"
#endif

#if DIVE_GAS_FO2_PERCENT > DIVE_GAS_FO2_MAX_PERCENT
#error "DIVE_GAS_FO2_PERCENT must be <= 40"
#endif
```

주의:

```text
DIVE_GAS_FO2_PERCENT는 산소분압이 아니다.
DIVE_GAS_FO2_PERCENT는 산소 비율이다.
```

---

# 6. Bühlmann / NDL / FN2 정책

## 6.1 기본 원칙

Bühlmann 계산에서 질소 비율 0.79를 하드코딩하지 않는다.

대신 FO2에서 FN2를 계산한다.

```text
FO2 = DIVE_GAS_FO2_PERCENT / 100.0
FN2 = 1.0 - FO2
```

적용 범위:

```text
tissue nitrogen loading
NDL calculation
GF99 calculation
decompression ceiling calculation
DECO.STOP duration calculation
No-Fly calculation
surface off-gassing
```

---

## 6.2 구현된 helper

`src/buhlmann.cpp` 기준으로 다음 helper 구조가 있다.

```text
getGasFO2()
getGasFN2()
calculateMODMeters()
calculatePpO2Bar(depthM)
```

---

## 6.3 주의 사항

현재 Bühlmann 구현은 개발용 프로토타입이다.

```text
의학적 검증 없음
상업용 다이브 컴퓨터와 일치 보장 없음
실제 다이빙 안전 판단 금지
```

---

# 7. MOD / ppO2 정책

## 7.1 ppO2 기준

```text
ppO2 max = 1.4 bar
```

## 7.2 MOD 계산식

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
MOD ≈ 28.8m
```

## 7.3 현재 구현 상태

구현됨:

```text
MOD 계산 helper
ppO2 계산 helper
FO2 range structure
```

아직 남은 작업:

```text
FO2 > 21%일 때 dive 화면에 MOD 상시 표시
MOD 초과 시 PPO2 HIGH / MOD EXCEEDED 경고
MOD warning event log
```

---

# 8. DECO.STOP ladder 정책

## 8.1 용어

화면 표시 용어는 다음을 사용한다.

```text
DECO.STOP
```

Safety Stop과 구분한다.

```text
S-STOP     = 선택적 안전정지
DECO.STOP = 필수 감압정지
```

---

## 8.2 지원 ladder

v1.3 DECO.STOP ladder:

```text
18m
15m
12m
9m
6m
3m
```

코드 기준:

```cpp
#define DECO_MAX_STOP_DEPTH_M 18.0f
```

---

## 8.3 raw ceiling → DECO.STOP 매핑

Bühlmann 계산에서 나온 raw ceiling을 고정 ladder 중 하나로 매핑한다.

```text
ceiling <= 3m   → DECO.STOP 3m
ceiling <= 6m   → DECO.STOP 6m
ceiling <= 9m   → DECO.STOP 9m
ceiling <= 12m  → DECO.STOP 12m
ceiling <= 15m  → DECO.STOP 15m
ceiling <= 18m  → DECO.STOP 18m
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

## 8.4 CEIL >18m 처리

raw ceiling이 18m보다 깊으면 `DECO.STOP 18m`로 표시하지 않는다.

이유:

```text
raw ceiling이 18m보다 깊다면 18m는 이미 너무 얕은 수심일 수 있다.
```

표시:

```text
CEIL >18m
HOLD DEPTH
```

의미:

```text
18m stop으로 상승하라는 뜻이 아니다.
아직 상승하면 안 된다.
계산상 ceiling이 18m 이하로 내려올 때까지 HOLD해야 한다.
```

이벤트:

```text
EVENT_CEIL_GT_18M
```

---

# 9. DECO.STOP margin 정책

## 9.1 v1.3.5-dev margin

v1.3.5-dev부터 DECO.STOP 허용 범위는 대칭 ±0.6m가 아니다.

코드 기준:

```cpp
#define DECO_STOP_SHALLOW_MARGIN_M   0.6f
#define DECO_STOP_HOLD_MARGIN_M      0.6f
#define DECO_STOP_DEEP_MARGIN_M      1.8f
```

의미:

```text
얕은 쪽 margin: 0.6m
HOLD 표시 범위: stop depth ±0.6m
깊은 쪽 timer 허용 margin: +1.8m
```

---

## 9.2 이유

얕은 쪽은 ceiling violation 위험이 있으므로 엄격하게 처리한다.

```text
stop보다 얕음 = 위험
```

깊은 쪽은 상대적으로 보수적이므로 더 넓은 margin을 허용한다.

```text
stop보다 약간 깊음 = 비효율적일 수 있으나 일반적으로 더 보수적
```

다만 stop보다 너무 깊으면 타이머는 멈춘다.

---

## 9.3 9m stop 예시

`DECO.STOP 9m` 기준:

```text
depth < 8.4m
  → 너무 얕음
  → DESCEND
  → timer pause

8.4m <= depth <= 9.6m
  → HOLD range
  → HOLD
  → timer run

9.6m < depth <= 10.8m
  → deep margin range
  → ASCEND
  → timer run

depth > 10.8m
  → 너무 깊음
  → ASCEND
  → timer pause
```

---

## 9.4 UI action

UI action:

```text
너무 얕음:
  DESCEND

HOLD range:
  HOLD

조금 깊음:
  ASCEND

너무 깊음:
  ASCEND
```

Timer policy:

```text
얕은 쪽 허용 범위 밖이면 timer pause
깊은 쪽 1.8m 이내면 timer run
깊은 쪽 1.8m 초과면 timer pause
```

---

# 10. DECO.STOP 실행 정책

## 10.1 기본 흐름

DECO.STOP은 현재 필요한 하나의 stop만 표시한다.

```text
DECO_REQUIRED
↓
CURRENT_DECO_STOP_SELECTED
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

---

## 10.2 현재 stop만 표시

전체 DecoPlan 표시는 하지 않는다.

표시하지 않는 예:

```text
18m 1'
15m 1'
12m 2'
9m  3'
6m  5'
3m  8'
```

표시하는 예:

```text
DECO.STOP
9m 1:00
ASCEND
```

---

## 10.3 Stop time

DECO.STOP 시간은 고정값이 아니다.

원칙:

```text
현재 tissue nitrogen loading을 기준으로 실시간 계산한다.
```

따라서 같은 9m stop이라도 상황에 따라 시간이 달라질 수 있다.

```text
가벼운 감압: 9m 1:00
큰 감압:     9m 5:00
```

---

## 10.4 현재 구현상 주의

v1.3.5-dev에서는 stop depth와 remain time이 실시간 계산 결과를 기반으로 갱신된다.

다만 다음 항목은 아직 추가 고도화가 필요하다.

```text
명시적 DecoRuntime 상태 머신
stopStarted flag
stopCompleted flag
다음 stop으로 넘어가는 시점의 시각적 안내
STOP DONE / NEXT 6m 표시
shallower stop jump 방지 로직 강화
```

---

# 11. NDL 0 / DECO.STOP 0m 방지 정책

## 11.1 문제

NDL이 0이 되었지만 실제 ceiling이나 stop이 아직 형성되지 않은 경우, 잘못하면 다음처럼 표시될 수 있다.

```text
DECO.STOP 0m
```

이는 잘못된 표시이다.

---

## 11.2 v1.3.5-dev 처리

현재 코드는 NDL이 0이더라도 바로 Deco phase로 들어가지 않고, `calculateDeco()` 결과를 확인한다.

DECO 진입 조건:

```text
ceiling_depth_m > 0.5m
또는 stop_depth_m > 0
또는 ceiling_gt_max_stop == true
```

즉, 실제 감압 의무가 있을 때만 Deco phase로 들어간다.

UI에서도 stop depth가 0이면 `DECO.STOP 0m` 대신 다음과 같은 fallback을 표시한다.

```text
DECO CHECK
WAIT
```

---

# 12. Final surfacing detection

## 12.1 다이빙 종료 기준

다이빙 종료는 수면 도달 즉시 확정하지 않는다.

코드 기준:

```cpp
#define DIVE_END_DEPTH_M      0.5f
#define DIVE_END_SURFACE_SEC  60
```

즉:

```text
수심 < 0.5m 상태가 60초 유지되면 최종 출수로 판단한다.
```

---

## 12.2 일반 출수 흐름

```text
수심 < 0.5m
↓
60초 유지
↓
endDive()
↓
PostDive
↓
일정 시간 후 Surface
```

일반 PostDive 표시 시간:

```cpp
#define POST_DIVE_DISPLAY_MS 180000UL
```

즉 일반적으로 3분이다.

---

# 13. DECO violation / MISSED DECO 정책

## 13.1 기본 철학

BackupDiveComputer는 감압 위반 후에도 hard lockout하지 않는다.

이유:

```text
백업 컴퓨터의 목적은 비상 상황에서도 정보를 계속 제공하는 것이다.
컴퓨터가 잠기면 수심, 시간, 감압 정보가 사라질 수 있다.
```

따라서 감압 위반 후에도 다음 정보는 계속 제공한다.

```text
수심
다이빙 시간
상승 속도
tissue-state 기반 DECO.STOP
Surface 정보
No-Fly
DECO.VIOL advisory
```

---

## 13.2 DECO violation 발생 조건

현재 코드에서는 다음 조건에서 DECO violation으로 처리한다.

```text
DECO phase에 들어간 적이 있음
그리고
출수 시점에 ceilingDepthM > 0.5m
```

코드 개념:

```cpp
if (dive_.decoEntered && dive_.ceilingDepthM > 0.5f) {
    dive_.decoViolation = true;
    activeDecoViolation_ = true;
}
```

---

## 13.3 Missed DECO surfacing flow

DECO.STOP 미완료 상태에서 출수한 경우:

```text
수심 < 0.5m
↓
60초 유지
↓
최종 출수 확정
↓
endDive()
↓
active ceiling 확인
↓
DECO violation 설정
↓
48시간 advisory 시작
↓
MISSED DECO alert 표시
↓ 30초
SURFACE 화면 전환
↓
SURFACE 화면에서 DECO.VIOL 남은 시간 표시
```

---

## 13.4 MISSED DECO alert 표시 시간

코드 기준:

```cpp
#define DECO_VIOLATION_ALERT_DISPLAY_MS 30000UL
```

즉:

```text
MISSED DECO alert는 30초 표시한다.
```

표시 예:

```text
MISSED DECO
VIOL ACTIVE
47:59 LEFT
```

또는 active violation이 clear된 후:

```text
MISSED DECO
NO DIVE 48H
46:12 LEFT
```

---

## 13.5 Surface 화면 정책

감압 위반 advisory 중에도 Surface 정보는 숨기지 않는다.

Surface 화면은 계속 다음 정보를 제공한다.

```text
LAST
MAX
TMP
SURFACE
N-FLY
```

단, 마지막 줄에서 `N-FLY`와 `DECO.VIOL`을 교대로 표시한다.

예:

```text
N-FLY     47:58
```

2초 후:

```text
DECO.VIOL 47:58
```

이 정책의 이유:

```text
감압 위반이 있더라도 사용자가 이전 다이빙 정보와 Surface 정보를 볼 수 있어야 한다.
백업 컴퓨터는 정보를 차단하지 않는다.
```

---

# 14. 48시간 post-violation advisory

## 14.1 상태 변수

v1.3.5-dev에서 구현된 상태 변수:

```cpp
bool activeDecoViolation_;
bool postViolationAdvisory_;
uint32_t postViolationAdvisoryEndEpochSec_;
uint8_t reentryCount_;
bool clearedAfterReentry_;
```

---

## 14.2 advisory 시작

DECO violation surfacing 시:

```text
postViolationAdvisory_ = true
postViolationAdvisoryEndEpochSec_ = now + 48h
```

코드 기준 시간:

```cpp
#define POST_VIOLATION_ADVISORY_HOURS 48UL
```

---

## 14.3 advisory 종료

매 update마다 advisory 종료 여부를 확인한다.

```text
현재 epoch >= postViolationAdvisoryEndEpochSec_
↓
postViolationAdvisory_ = false
postViolationAdvisoryEndEpochSec_ = 0
EVENT_POST_VIOLATION_ADVISORY_ENDED
```

---

## 14.4 주의

현재 advisory 상태는 runtime 변수이다.

아직 남은 작업:

```text
postViolationAdvisory_를 재부팅 후 복원
postViolationAdvisoryEndEpochSec_를 NVS 또는 log header에 저장
activeDecoViolation_ 영구 저장
```

---

# 15. Re-entry after missed DECO

## 15.1 기본 정책

감압 위반 후 재입수해도 컴퓨터는 잠기지 않는다.

재입수 시:

```text
tissue state 유지
DECO.STOP 재계산
필요 시 DECO.STOP 표시
NO DIVE advisory보다 실제 물속 행동 지시 우선
```

---

## 15.2 re-entry event

감압 위반 상태에서 새 다이빙이 시작되면:

```text
reentryCount_ 증가
clearedAfterReentry_ = false
EVENT_DECO_REENTRY
```

---

## 15.3 clear after re-entry

재입수 후 계산상 DECO가 clear되면:

```text
activeDecoViolation_ = false
clearedAfterReentry_ = true
EVENT_DECO_CLEARED_AFTER_REENTRY
```

단:

```text
postViolationAdvisory_는 48시간 동안 유지한다.
```

이유:

```text
계산상 ceiling이 clear되었다고 해서 의학적으로 완전히 안전하다는 뜻은 아니다.
감압 위반 이력은 계속 advisory로 알려야 한다.
```

---

## 15.4 재입수 권장 아님

이 기능은 재입수를 권장하기 위한 기능이 아니다.

목적:

```text
이미 재입수한 상황에서 정보를 계속 제공한다.
메인 컴퓨터 lockout 상황에서도 백업 정보를 제공한다.
```

감압 위반 후 행동은 다이버의 훈련, 현장 상황, 응급 절차, DAN 또는 의료기관 지침을 따라야 한다.

---

# 16. Event log policy

## 16.1 구현된 event type

`include/log_format.h` 기준:

```cpp
enum class DiveEventType : uint8_t {
    EVENT_DECO_REQUIRED = 1,
    EVENT_DECO_STOP_STARTED = 2,
    EVENT_DECO_STOP_COMPLETED = 3,
    EVENT_DECO_CLEARED = 4,
    EVENT_DECO_MISSED = 5,
    EVENT_DECO_VIOLATION_SURFACED = 6,
    EVENT_DECO_REENTRY = 7,
    EVENT_DECO_CLEARED_AFTER_REENTRY = 8,
    EVENT_POST_VIOLATION_ADVISORY_STARTED = 9,
    EVENT_POST_VIOLATION_ADVISORY_ENDED = 10,
    EVENT_CEILING_EXCEEDED = 11,
    EVENT_CEIL_GT_18M = 12
};
```

---

## 16.2 현재 event 기록 수준

현재 `logDiveEvent()`는 Serial 출력 중심이다.

예:

```text
[EVENT] EVENT_DECO_MISSED type=5 elapsed=...
```

아직 남은 작업:

```text
DiveEvent를 compact log에 영구 저장
eventCount 실제 반영
event buffer 구현
LittleFS event append 구조
Subsurface XML export에 event 반영
```

---

# 17. Safety Stop 정책

## 17.1 S-STOP과 DECO.STOP 구분

Safety Stop은 선택적 안전정지이다.

```text
S-STOP = Safety Stop
DECO.STOP = Mandatory decompression stop
```

혼동 금지:

```text
S-STOP missed ≠ DECO violation
DECO.STOP missed = DECO violation
```

---

## 17.2 Safety Stop 기본값

`include/config.h`:

```cpp
#define SAFETY_STOP_TRIGGER_DEPTH_M      10.0f
#define SAFETY_STOP_MIN_DEPTH_M           3.0f
#define SAFETY_STOP_MAX_DEPTH_M           6.0f
#define SAFETY_STOP_DURATION_S          180
#define SAFETY_STOP_REEVAL_SEC           30
#define SAFETY_STOP_SKIPPED_DISPLAY_SEC 30UL
```

---

## 17.3 Safety Stop skipped flow

Safety Stop이 필요한 상태에서 너무 얕게 올라가면:

```text
S.STOP PAUSED
↓
얕은 상태 30초 지속
↓
S.STOP SKIPPED
↓
30초 표시
↓
다이빙 종료 가능
```

최종 다이빙 종료는 여전히 다음 조건을 따른다.

```text
depth < 0.5m 상태 60초 유지
```

---

## 17.4 DECO.STOP 우선

DECO.STOP이 필요한 경우 Safety Stop은 표시하지 않는다.

우선순위:

```text
DECO.STOP > S-STOP
```

---

# 18. UI 우선순위

버튼 없는 자동 컴퓨터이므로 UI 우선순위가 중요하다.

권장 우선순위:

```text
1. 즉시 안전 경고
   - ceiling violation
   - CEIL >18m
   - 빠른 상승
2. DECO.STOP 행동 지시
3. MOD / ppO2 경고
4. S-STOP
5. NDL / 일반 dive 정보
6. 배터리 경고
7. Surface 정보
```

감압 위반 후 재입수 중:

```text
NO DIVE advisory보다 DECO.STOP 행동 지시가 우선이다.
```

Surface 중:

```text
DECO.VIOL advisory가 있어도 LAST/MAX/TMP/SURFACE/N-FLY를 막지 않는다.
```

---

# 19. Log format and storage

## 19.1 구현된 파일

현재 존재:

```text
include/log_format.h
include/log_storage.h
src/log_storage.cpp
```

---

## 19.2 DiveLogHeader

현재 `DiveLogHeader`에는 다음 정보가 있다.

```text
magic
version
headerSize
diveNumber
timeStatus
timeSessionId
startEpochSec
endEpochSec
durationSec
noFlyEndEpochSec
maxDepthCm
avgDepthCm
minTempDeciC
sampleCount
eventCount
gpsLatE7
gpsLonE7
gpsValid
```

---

## 19.3 DiveSample

현재 구조:

```text
timeSec
depthCm
tempDeciC
ndlOrTtsMin
```

아직 샘플 전체 저장은 완성되지 않았다.

---

## 19.4 DiveEvent

현재 구조:

```text
timeSec
type
value
```

아직 이벤트 영구 저장은 완성되지 않았다.

---

## 19.5 남은 로그 작업

남은 작업:

```text
gas FO2 저장 필드 추가
ppO2 max 저장 필드 추가
deco violation flag 저장
post violation advisory 저장
missed stop depth/remain 저장
sample append 저장
event append 저장
eventCount 실제 반영
Subsurface XML export 연결
```

---

# 20. Time sync policy

## 20.1 GPS 시간이 없을 수 있음

GPS 시간이 없으면 실제 날짜/시간 대신 상대 시간을 사용해야 한다.

가능한 상태:

```text
TimeSynced
RelativeOnly
TimeCorrected
SyncFailed
```

현재 enum:

```cpp
enum class LogTimeStatus : uint8_t {
    TimeSynced = 0,
    RelativeOnly = 1,
    TimeCorrected = 2,
    SyncFailed = 3
};
```

---

## 20.2 timeSessionId

시간 보정 가능 여부를 판단하기 위해 `timeSessionId`를 사용한다.

정책:

```text
같은 timeSessionId 안에서는 나중에 GPS/BLE 시간으로 보정 가능
timeSessionId가 다르면 임의 보정 금지
```

---

## 20.3 48시간 advisory와 시간

가능하면 epoch 기반으로 advisory를 계산한다.

```text
advisoryEndEpochSec = violationEpochSec + 48h
```

epoch가 없을 경우 향후 상대시간 fallback이 필요하다.

남은 작업:

```text
epoch 없는 상태의 advisory 처리
재부팅 후 advisory 복원
GPS/BLE 시간 확보 후 advisoryEndEpochSec 보정
```

---

# 21. Power management policy

## 21.1 기본 방향

물리 전원 버튼이 없으므로 완전 종료보다 저전력 대기 상태가 적합하다.

대기 상태:

```text
OLED OFF 또는 power save
GPS OFF
BLE OFF
Wi-Fi OFF
ESP32 Light Sleep
```

---

## 21.2 Deep Sleep 주의

Deep Sleep은 신중하게 사용한다.

이유:

```text
millis() 초기화 가능
time continuity 문제
tissue/advisory continuity 문제
GPS 없는 로그 보정 어려움
```

기본 정책:

```text
Light Sleep 우선
Deep Sleep은 배터리 보호 등 예외 상황에서만 검토
```

---

# 22. BLE / App policy

BLE는 항상 켜두지 않는다.

권장 정책:

```text
충전 중 BLE 활성화
로그 다운로드 중 BLE 활성화
설정 변경 중 BLE 활성화
다이빙 중 BLE OFF
대기 중 BLE OFF
```

향후 앱 설정 후보:

```text
FO2
ppO2 max
time sync
log download
device info
firmware version
```

FO2 설정은 향후 NVS / Preferences에 저장한다.

---

# 23. Subsurface XML policy

내부 저장은 compact binary를 사용한다.

Subsurface XML은 나중에 export tool에서 생성한다.

예정 도구:

```text
tools/bdc_to_subsurface_xml.py
```

XML에 포함할 정보:

```text
dive date/time
duration
sample time/depth/temp
GPS site
gas FO2
DECO.STOP events
MOD warning events
DECO violation events
```

---

# 24. Scenario system

현재 시뮬레이션은 JSON scenario를 사용한다.

대표 파일:

```text
scenarios/previous_dive_surface.json
```

자동 생성 파일:

```text
include/generated_scenario.h
```

생성 도구:

```text
tools/generate_scenario.py
```

주의:

```text
generated_scenario.h는 직접 수정하지 않는다.
JSON scenario를 수정한 뒤 빌드하여 자동 생성한다.
```

---

# 25. Serial mock commands

Wokwi / mock 환경에서 사용하는 명령 예:

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

gps ok
gps fail

beep test

log info
log clear
```

주의:

```text
depth <value> 명령을 사용하면 자동 profile이 중지되고 manual depth mode로 전환된다.
자동 profile을 유지한 채 일시정지/재개하려면 profile pause / profile resume을 사용한다.
```

---

# 26. Build

PlatformIO build:

```bash
pio run -e wokwi
```

PowerShell에서 `pio`를 찾지 못할 경우:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e wokwi
```

성공 시:

```text
SUCCESS
```

---

# 27. Git / Tag policy

현재 개발 브랜치:

```text
dev/v1.3
```

현재 개발 snapshot tag:

```text
v1.3.5-dev
```

문서 수정 후 commit 예:

```bash
git checkout dev/v1.3
git status
git add docs/DEVELOPMENT_v1.3.md
git commit -m "Update DEVELOPMENT docs for v1.3.5-dev"
git push origin dev/v1.3
```

태그를 최신 문서 포함 위치로 이동하려면, 개인 개발 단계에서만 다음을 사용할 수 있다.

```bash
git tag -f v1.3.5-dev
git push origin -f v1.3.5-dev
```

주의:

```text
이미 외부에서 태그를 사용 중이면 tag force push는 피한다.
```

---

# 28. v1.3.5-dev 완료된 주요 항목

현재 완료 또는 부분 구현 완료:

```text
FW_VERSION v1.3.5-dev
Air / EAN21 기본 gas
FO2 21~40% compile-time range
ppO2 max 1.4 bar
Bühlmann FN2 하드코딩 제거
MOD / ppO2 helper
DECO.STOP 18/15/12/9/6/3 ladder
CEIL >18m 경고
DECO.STOP 0m 방지
비대칭 DECO.STOP margin
Surface LAST/MAX/TMP/SURFACE/N-FLY
Surface DECO.VIOL 교대 표시
DECO violation runtime state
48시간 post-violation advisory runtime state
MISSED DECO alert 30초 표시
re-entry event
DECO cleared after re-entry event
log_format.h
log_storage.h / log_storage.cpp
DiveLogHeader 저장/로드
LittleFS / Wokwi RAM fallback
Battery low 10분 팝업
Fast ascent triple beep
Wokwi beep test
Safety Stop skipped flow
```

---

# 29. v1.3.5-dev 이후 남은 주요 작업

남은 작업:

```text
pio run -e wokwi 최종 빌드 검증
Wokwi 시뮬레이션 시나리오별 검증
FO2 >21% MOD 상시 표시 UI
MOD 초과 경고 UI
MOD warning event
DECO.STOP started/completed event
DiveEvent compact log 저장
DiveSample compact log 저장
eventCount 실제 반영
postViolationAdvisory 재부팅 후 복원
activeDecoViolation 재부팅 후 복원
advisoryEndEpochSec 저장
missedStopDepthM / missedStopRemainSec 저장
re-entry 시나리오 반복 검증
DECO.STOP stop sequencing 고도화
STOP DONE / NEXT stop UI
Subsurface XML export tool
BLE log download
BLE/app FO2 setting
NVS / Preferences gas setting
실제 하드웨어 전류 측정
GPS/BLE/OLED power control 검증
```

---

# 30. v1.3 완료 기준

v1.3 완료 기준:

```text
빌드 성공
Wokwi 시뮬레이션 성공
Surface 화면 정상
Dive 화면 정상
Safety Stop 정상
DECO.STOP ladder 정상
CEIL >18m 정상
DECO.STOP 0m 방지
감압 위반 출수 감지
MISSED DECO alert 정상
SURFACE DECO.VIOL 표시 정상
48시간 advisory 유지
재입수 시 hard lockout 없음
re-entry DECO.STOP 계산 지속
로그 header 저장/로드 정상
이벤트 저장 구조 완성
README / CHANGELOG / TODO / CHAT_CONTEXT 최신화
```

---

# 31. 최종 방향 요약

BackupDiveComputer v1.3의 핵심 방향은 다음과 같다.

```text
버튼 없는 스탠드얼론 백업 다이브 컴퓨터이다.

기본 gas는 Air / EAN21이다.
Nitrox-ready 구조를 준비하지만 현재는 single gas만 사용한다.

Bühlmann 계산은 config.h의 FO2에서 FN2를 계산해 사용한다.
ppO2 max는 1.4 bar이다.

DECO.STOP은 18m, 15m, 12m, 9m, 6m, 3m ladder를 사용한다.
전체 DecoPlan은 표시하지 않고 현재 필요한 DECO.STOP 하나만 표시한다.

DECO.STOP 위반 후에도 hard lockout하지 않는다.
출수 후 MISSED DECO alert를 표시하고 48시간 advisory를 유지한다.
SURFACE 화면에서는 기존 다이빙 정보를 계속 보여주며 DECO.VIOL 남은 시간을 함께 표시한다.

재입수 시에는 tissue state 기반으로 DECO.STOP 계산을 계속 제공한다.
단, 이 기능은 재입수를 권장하는 기능이 아니라 이미 재입수한 경우 정보를 제공하기 위한 기능이다.
```
