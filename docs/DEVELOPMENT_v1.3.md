# BackupDiveComputer v1.3 Development Definition

이 문서는 `BackupDiveComputer` 프로젝트의 `dev/v1.3` 개발 기준 문서이다.

현재 문서 기준 스냅샷:

```text
v1.3.6-dev
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
v1.3.6-dev
```

`v1.3.6-dev`는 정식 안정판이 아니라 `v1.3` 개발 중간 스냅샷이다.

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

BackupDiveComputer는 버튼 없는 밀폐형 장치이므로 GPS, BLE, 충전, 로그 시간 보정은 모두 자동으로 동작해야 한다.

### 2.3.1 Qi 충전 중 동작

다이빙컴퓨터가 Qi 충전 중일 때는 다음 정책을 따른다.

```text
1. GPS를 켠다.
2. 위치 정보와 실제 시간 정보를 얻을 때까지 재시도한다.
3. BLE를 켠다.
4. 스마트폰 앱과 연결할 수 있도록 BLE advertising 또는 connection-ready 상태를 유지한다.
5. Qi 충전기에서 분리되면 GPS와 BLE를 모두 끈다.
```

단, 다음 경우에는 예외적으로 GPS/BLE 종료를 지연할 수 있다.

```text
BLE 로그 다운로드가 진행 중인 경우
BLE 설정 변경이 진행 중인 경우
GPS fix 직후 로그 시간 보정이 아직 완료되지 않은 경우
```

충전 중 UI 표시 정책:

```text
GPS 수신 시도 중: G 깜빡임
GPS fix / RTS 완료: G 고정 표시
GPS off 또는 실패 후 중지: - 표시

BLE advertising / 연결 대기 중: B 깜빡임
BLE connected: B 고정 표시
BLE off: - 표시
```

### 2.3.2 다이빙 중 동작

다이빙 중에는 전력 소모와 무선 통신 불필요성을 고려하여 다음 정책을 따른다.

```text
GPS OFF
BLE OFF
Wi-Fi OFF
수심/시간/감압 계산 우선
로그 샘플 기록 우선
```

다이빙 중에는 GPS 위치나 BLE 연결을 시도하지 않는다.

### 2.3.3 출수 후 GPS 동작

다이빙 후 출수하면 GPS를 켜서 위치 정보와 실제 시간 정보를 얻기 위해 시도한다.

정책:

```text
1회 시도 시간: 30초
최대 시도 횟수: 6회
총 최대 시도 시간: 180초
```

GPS fix 또는 RTS 획득에 성공하면 다음을 수행한다.

```text
출수 위치 저장
실제 시간 동기화 상태 갱신
RTS가 없던 직전 다이브 로그의 시간 보정 시도
보정 성공 시 로그의 timeStatus를 TimeCorrected로 변경
```

6회 모두 실패하면 다음을 수행한다.

```text
GPS OFF
해당 로그는 RelativeOnly 또는 SyncFailed 상태 유지
충전 중이 아니라면 추가 GPS 재시도 중지
```

### 2.3.4 잠깐 Qi 충전기에 갖다 댔을 때의 wake 동작

Sleep 또는 low-power 상태에서 장치를 잠깐 Qi 충전기에 갖다 대면 다음 순서로 동작한다.

```text
1. 장치가 wake 된다.
2. Splash 화면을 잠시 표시한다.
3. Surface 모드로 진입한다.
4. GPS 수신을 시도한다.
5. GPS 수신 정책은 출수 후 GPS 동작과 동일하게 30초씩 최대 6회 시도한다.
6. 충전이 계속 유지되면 BLE도 켜고 앱 연결 대기 상태를 유지한다.
7. Qi 충전기에서 분리되면 GPS와 BLE를 끈다.
```

### 2.3.5 RTS, GPS time, BLE time 용어

본 문서에서 RTS는 Real Time Sync를 의미한다.

RTS가 성립하는 경우:

```text
GPS에서 유효한 UTC time을 얻은 경우
BLE 앱에서 신뢰 가능한 현재 시간을 받은 경우
```

RTS가 없는 경우:

```text
실제 날짜/시간을 알 수 없음
다이빙 시작/종료 시각은 epoch로 확정할 수 없음
부팅 후 경과시간 기준으로만 상대 기록 가능
```

RTS 우선순위는 다음과 같다.

```text
1. GPS time
2. BLE app time
3. 시뮬레이션 epoch time
4. boot elapsed relative time
```

Wokwi simulation에서는 시뮬레이션 epoch를 RTS 대체값으로 사용할 수 있다.
실제 하드웨어에서는 GPS 또는 BLE 시간이 없는 경우 RTS가 없는 것으로 본다.

### 2.3.6 로그 시간 기록 및 사후 보정 정책

다이브 로그의 시간 기록은 다음 정책을 따른다.

#### RTS가 있는 상태에서 다이빙한 경우

다이빙 시작 시점에 RTS가 이미 있으면 로그에는 실제 epoch 시간을 기록한다.

```text
timeStatus = TimeSynced
startEpochSec = 실제 시작 epoch
endEpochSec = 실제 종료 epoch
durationSec = 실제 다이빙 시간
```

#### RTS가 없는 상태에서 다이빙한 경우

RTS가 없는 상태에서 다이빙을 시작하거나 종료하면 실제 날짜/시간을 확정하지 않는다.

이 경우 로그에는 다음 정보를 저장한다.

```text
timeStatus = RelativeOnly
startEpochSec = 0 또는 임시값
endEpochSec = 0 또는 임시값
durationSec = 다이빙 지속 시간
timeSessionId = 현재 시간 세션 ID
bootCount = 현재 부팅 카운터
bootElapsedStartSec = 부팅 후 다이빙 시작까지의 경과 시간
bootElapsedEndSec = 부팅 후 다이빙 종료까지의 경과 시간
```

단, bootElapsed는 Arduino `millis()` raw 값을 직접 저장하지 않는다.

ESP32에서는 `esp_timer_get_time()` 또는 이에 준하는 64-bit monotonic timer를 사용하여
부팅 후 경과 시간을 계산한다.

로그에는 millisecond 단위가 아니라 second 단위의 값을 저장한다.

```text
bootElapsedStartSec
bootElapsedEndSec
currentBootElapsedSec
```

이유:

```text
millis()는 약 49.7일 후 overflow된다.
millis()는 reset/deep sleep 후 0으로 초기화된다.
로그 시간 보정에는 절대 millis 값보다 bootCount + monotonic elapsed seconds가 안전하다.
```

단, UI blink, retry timeout, 짧은 상태 전환 타이머에는 기존처럼 `millis()`를 사용할 수 있다.
```


#### 나중에 GPS 또는 BLE로 RTS가 확보된 경우

Surface 모드 또는 충전 중 GPS/BLE로 RTS가 확보되면, 시스템은 보정 가능한 이전 로그를 검사한다.

보정 가능 조건:

```text
로그의 timeStatus가 RelativeOnly일 것
로그의 bootCount가 현재 bootCount와 같을 것
로그의 timeSessionId가 현재 timeSessionId와 같을 것
현재 boot elapsed time을 알고 있을 것
현재 RTS epoch를 알고 있을 것
```

보정 계산:

```text
bootEpochSec = currentRtsEpochSec - currentBootElapsedSec

correctedStartEpochSec = bootEpochSec + bootElapsedStartSec
correctedEndEpochSec   = bootEpochSec + bootElapsedEndSec
```

보정 성공 시:

```text
startEpochSec = correctedStartEpochSec
endEpochSec = correctedEndEpochSec
timeStatus = TimeCorrected
```

보정 실패 시:

```text
timeStatus = RelativeOnly 유지
또는 명확히 실패가 확정된 경우 SyncFailed로 변경
```

#### bootCount가 다른 경우

로그의 bootCount가 현재 bootCount와 다르면 자동 시간 보정을 하지 않는다.

이유:

```text
장치가 꺼져 있던 시간을 알 수 없기 때문이다.
예를 들어 장치가 한 달 동안 꺼져 있었다면,
부팅 후 경과시간만으로 과거 로그의 실제 시간을 역산할 수 없다.
```

따라서 bootCount가 다른 로그는 사용자가 앱에서 수동 보정하거나,
별도의 외부 로그와 대조하지 않는 한 자동 수정하지 않는다.

### 2.3.7 GPS/BLE 상태 표시 원칙

상단바에는 GPS와 BLE 상태를 간결하게 표시한다.

표시 정책:

```text
GPS searching / trying RTS: G 깜빡임
GPS fix / RTS valid:        G 고정
GPS off / failed:           -

BLE advertising:            B 깜빡임
BLE connected:              B 고정
BLE off:                    -
```

이 표시는 Surface, PostDive, Charging 화면에서 우선 적용한다.

Dive 화면에서는 수심/시간/감압 정보가 우선이므로 GPS/BLE 표시를 생략할 수 있다.


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

# 4. 현재 v1.3.6-dev 구현 상태 요약

`v1.3.6-dev` 기준으로 다음 항목이 코드에 반영되어 있다.

## 4.1 Firmware version

`include/config.h`:

```cpp
#define FW_NAME     "BackupDiveComputer"
#define FW_VERSION  "v1.3.6-dev"
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

## 4.5 v1.3.6-dev 추가 구현

`v1.3.6-dev`에서는 다음 항목이 코드에 반영되었다.

```text
1. Surface / PostDive 진입 시 GPS 자동 search 시작

2. Surface / PostDive GPS retry 정책 적용
   - 30초 x 6회
   - 실패 시 GPS OFF

3. Charging 진입 시 GPS search + BLE advertising 자동 시작

4. Dive 진입 시 GPS/BLE 자동 OFF

5. PostDive 중 재입수는 continuous dive로 처리
   - diveCount 증가 없음
   - 새 log start 없음

6. PostDive → Surface 전환 시 최종 로그 close/save

7. Surface 이후 재입수는 repetitive dive로 Serial log 구분
   - tissue loading 유지
   - 새 로그 시작

8. Buzzer 정책 정리
   - 이벤트 알림: 1회
   - 긍정 신호: 2회
   - 부정 신호: 3회
   - POST DIVE 진입음 제거
   - Safety Stop 일반 상태음 제거
   - MISSED DECO 3회 경고음 추가

9. Mock GPS 자동 fix 시뮬레이션 추가
   - GPS search 시작 후 일정 시간 뒤 GPS valid 처리
   - Surface / Charging UI에서 G blinking → G steady 확인 가능

10. BLE access window 정책 추가
   - Qi/Charging 감지 시 BLE advertising 시작
   - Charging 중에는 BLE access window를 계속 연장
   - Charging 해제 후에도 15분 동안 BLE 유지
   - BLE connected 상태에서는 window와 관계없이 유지
   - Dive 진입 시에는 BLE 즉시 OFF

11. GPS fix 기반 RTS 획득 처리 추가
   - GPS valid 상승 edge를 RTS 획득으로 처리
   - rtsEpochSec 저장
   - rtsAcquiredBootElapsedSec 저장

12. Log format v2 적용
   - bootCount 저장
   - bootElapsedStartSec 저장
   - bootElapsedEndSec 저장
   - timeSessionId 저장
   - RelativeOnly / TimeSynced / TimeCorrected 상태 사용

13. RelativeOnly 로그 시간 보정 구현
   - GPS RTS 획득 후 bootEpochSec 계산
   - bootCount가 같을 때만 자동 보정
   - timeSessionId가 같을 때만 자동 보정
   - 보정 성공 시 TimeCorrected로 변경

14. DiveLogHeader 확장
   - gasFo2Percent
   - ppO2MaxCentiBar
   - decoViolation
   - postViolationAdvisory
   - advisoryEndEpochSec
   - reentryCount
   - missedStopDepthCm
   - missedStopRemainSec

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

## 9.1 v1.3.6-dev margin

v1.3.6-dev부터 DECO.STOP 허용 범위는 대칭 ±0.6m가 아니다.

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

v1.3.6-dev에서는 stop depth와 remain time이 실시간 계산 결과를 기반으로 갱신된다.

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

## 11.2 v1.3.6-dev 처리

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

# 12. Final surfacing detection / Continuous dive policy

## 12.1 다이빙 종료 후보 기준

다이빙 종료는 수면 도달 즉시 확정하지 않는다.

코드 기준:

```cpp
#define DIVE_END_DEPTH_M      0.5f
#define DIVE_END_SURFACE_SEC  60
```

즉:

```text
수심 < 0.5m 상태가 60초 유지되면
Dive 상태에서 PostDive 상태로 전환한다.
```

이 시점은 사용자가 실제로 다이빙을 완전히 끝냈다고 확정하는 시점이 아니라,
"출수 후보" 또는 "tentative surfacing" 상태로 본다.

## 12.2 PostDive holding window

PostDive는 짧은 수면 체류 후 재입수하는 상황을 하나의 연속 다이빙으로 처리하기 위한 holding window 역할을 한다.

일반 PostDive 표시 시간:

```cpp
#define POST_DIVE_DISPLAY_MS 180000UL
```

즉:

```text
PostDive는 3분 동안 유지된다.
```

따라서 실제 수면 도달 후 연속다이빙으로 간주할 수 있는 총 시간은 다음과 같다.

```text
Dive 상태에서 수심 <0.5m 유지 대기: 60초
PostDive holding window: 180초
총 연속다이빙 판정 여유: 약 240초 = 약 4분
```

## 12.3 연속다이빙 판단

다음 조건에서 재잠수하면 연속다이빙으로 처리한다.

```text
Dive 상태에서 아직 60초 출수 확정 전 재잠수
또는
PostDive holding window 안에서 재잠수
```

연속다이빙 처리 정책:

```text
새 다이브 로그를 만들지 않는다.
diveNumber를 증가시키지 않는다.
기존 다이빙 로그에 샘플을 이어서 기록한다.
수면에 있었던 구간은 depth 0m 또는 surface segment로 기록할 수 있다.
tissue loading은 계속 이어서 계산한다.
dive time 정책은 별도로 정하되, 로그에는 연속 이벤트를 남길 수 있다.
```

## 12.4 실제 다이빙 종료 확정 시점

실제 다이빙 로그의 종료는 PostDive에서 Surface로 전환되는 시점에 확정한다.

흐름:

```text
수심 <0.5m
↓
60초 유지
↓
Dive → PostDive
↓
3분 동안 재입수 없음
↓
PostDive → Surface
↓
이 시점을 실제 로그 종료 확정 시점으로 처리
```

이 정책의 목적:

```text
짧은 수면 체류 후 재입수를 별도 반복다이빙으로 잘못 나누지 않는다.
교육, 체크아웃, 구조훈련, 얕은 수면 체류 상황에서 로그가 불필요하게 쪼개지는 것을 방지한다.
```

## 12.5 반복다이빙 판단

Surface 모드에 진입한 이후 다시 잠수하면 반복다이빙으로 처리한다.

반복다이빙 처리 정책:

```text
새 다이브 로그를 시작한다.
diveNumber를 증가시킨다.
이전 tissue loading은 유지한다.
Surface interval은 이전 로그 종료 시점부터 계산한다.
No-Fly, desaturation, post-violation advisory는 계속 유지한다.
```

즉:

```text
PostDive 안에서 재입수 = 연속다이빙
Surface 진입 후 재입수 = 반복다이빙
```

## 12.6 용어 정리

본 프로젝트에서 사용하는 용어는 다음과 같다.

```text
Continuous dive / 연속다이빙:
  약 4분 이내의 짧은 수면 체류 후 재입수하여 하나의 로그로 이어지는 다이빙

Repetitive dive / 반복다이빙:
  Surface 모드 진입 이후 새 로그로 시작하지만,
  이전 다이빙의 tissue loading을 이어받는 다이빙

Final log close / 로그 종료 확정:
  PostDive에서 Surface로 전환되는 시점
```

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

DECO.STOP 미완료 상태에서 출수한 경우에도 최종 종료 확정은 일반 출수와 동일한 PostDive holding window를 따른다.

흐름:

```text
수심 <0.5m
↓
60초 유지
↓
Dive → PostDive
↓
active ceiling 확인
↓
DECO violation 설정
↓
48시간 advisory 시작
↓
MISSED DECO alert 표시
↓
PostDive 3분 유지
↓
재입수 없으면 Surface 전환
↓
Surface 화면에서 DECO.VIOL 남은 시간 표시
```

PostDive 중 재입수하면 다음과 같이 처리한다.

```text
새 로그를 만들지 않는다.
연속다이빙으로 처리한다.
tissue state를 유지한다.
DECO.STOP 계산을 계속 제공한다.
activeDecoViolation_ 상태는 유지하거나,
재입수 후 계산상 clear될 때까지 유지한다.
```

## 13.4 MISSED DECO PostDive 표시 시간

DECO violation이 있는 경우에도 PostDive 표시 시간은 일반 PostDive와 동일하게 3분으로 유지한다.

코드 목표:

```cpp
#define POST_DIVE_DISPLAY_MS 180000UL
```

즉:

```text
MISSED DECO alert도 PostDive 상태에서 최대 3분 동안 표시한다.
```

기존의 짧은 alert timeout 값은 Surface 전환 기준으로 사용하지 않는다.

```cpp
#define DECO_VIOLATION_ALERT_DISPLAY_MS 30000UL
```

이 값은 향후 다음 용도로만 사용할 수 있다.

```text
MISSED DECO 문구의 최소 강조 표시 시간
beep 또는 visual alert 반복 주기
UI 경고 순환 주기
```

하지만 상태 전환 기준은 다음으로 통일한다.

```text
PostDive → Surface = POST_DIVE_DISPLAY_MS
일반 출수 = 3분
DECO 위반 출수 = 3분
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

v1.3.6-dev에서 구현된 상태 변수:

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
## 19.6 Log format v2 후보 필드

RTS가 없는 상태에서 생성된 로그를 나중에 GPS/BLE 시간으로 보정하기 위해 log format v2에서는 다음 필드를 추가하는 것을 검토한다.

```text
bootCount
bootElapsedStartSec
bootElapsedEndSec
timeCorrectionStatus
timeCorrectionSource
```

후보 구조:

```cpp
uint32_t bootCount;
uint32_t bootElapsedStartSec;
uint32_t bootElapsedEndSec;
uint8_t  timeCorrectionSource; // 0 none, 1 GPS, 2 BLE, 3 simulation
uint8_t  reservedTime[3];
```

또는 기존 header를 크게 변경하지 않기 위해 별도 TimeCorrection record를 둘 수 있다.

```text
DiveLogHeader
DiveSample[]
DiveEvent[]
DiveTimeCorrectionRecord
```

정책:

```text
log format v1은 현재 구조 유지
bootCount 기반 시간 보정이 실제 구현될 때 log format v2 검토
기존 v1 로그를 읽을 수 있도록 backward compatibility 유지
```

---

# 20. Time sync policy

## 20.1 GPS 시간이 없을 수 있음

GPS 또는 BLE time sync가 없는 경우 실제 날짜/시간 대신 상대 시간을 사용한다.

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

## 20.2 timeSessionId

시간 보정 가능 여부를 판단하기 위해 `timeSessionId`를 사용한다.

정책:

```text
같은 timeSessionId 안에서는 나중에 GPS/BLE 시간으로 보정 가능
timeSessionId가 다르면 임의 보정 금지
```

`timeSessionId`는 다음 조건에서 새로 생성될 수 있다.

```text
cold boot
deep sleep wake 후 시간 연속성을 보장할 수 없는 경우
RTC continuity가 깨진 경우
factory reset 또는 log reset
```

## 20.3 bootCount 기반 보정

`bootCount`는 장치가 새로 부팅될 때마다 증가하는 영구 카운터이다.

용도:

```text
RTS가 없을 때 기록된 로그가 현재 부팅 세션에서 생성된 것인지 판단
장기간 전원 OFF 후 잘못된 시간 보정을 방지
```

RTS가 없는 로그에는 다음 정보를 저장한다.

```text
bootCount
bootElapsedStartSec
bootElapsedEndSec
timeSessionId
durationSec
```

GPS 또는 BLE로 현재 실제 시간이 확보되면 다음을 계산한다.

```text
bootEpochSec = currentEpochSec - currentBootElapsedSec
```

그 후 로그 시간이 다음과 같이 보정된다.

```text
correctedStartEpochSec = bootEpochSec + bootElapsedStartSec
correctedEndEpochSec   = bootEpochSec + bootElapsedEndSec
```

보정 성공 조건:

```text
log.timeStatus == RelativeOnly
log.bootCount == currentBootCount
log.timeSessionId == currentTimeSessionId
currentEpochSec is valid
currentBootElapsedSec is valid
```

보정 성공 시:

```text
log.startEpochSec = correctedStartEpochSec
log.endEpochSec = correctedEndEpochSec
log.timeStatus = TimeCorrected
```

보정 실패 시:

```text
bootCount mismatch → 자동 보정 금지
timeSessionId mismatch → 자동 보정 금지
RTS invalid → 보정 보류
명확히 보정 불가능한 경우 → SyncFailed
```

## 20.4 GPS/BLE time source

시간 동기화 source는 다음과 같이 구분한다.

```text
GPS time
BLE app time
Simulation time
None
```

우선순위:

```text
1. GPS time
2. BLE app time
3. Simulation time
4. Relative boot elapsed time
```

실제 하드웨어에서는 GPS/BLE 시간이 없으면 실제 epoch를 신뢰하지 않는다.

## 20.5 48시간 advisory와 시간

가능하면 epoch 기반으로 advisory를 계산한다.

```text
advisoryEndEpochSec = violationEpochSec + 48h
```

epoch가 없을 경우:

```text
상대 시간으로 advisory를 임시 계산한다.
RTS 확보 후 advisoryEndEpochSec를 보정한다.
bootCount 또는 timeSessionId가 맞지 않으면 자동 보정하지 않는다.
```

남은 작업:

```text
epoch 없는 상태의 advisory 처리
재부팅 후 advisory 복원
GPS/BLE 시간 확보 후 advisoryEndEpochSec 보정
bootCount 기반 로그 시간 보정 구현
log format v2 검토
```

## 20.5 bootElapsed continuity policy

bootElapsed는 Deep Sleep을 거쳐 보존되는 시간을 의미하지 않는다.
bootElapsed는 awake session 기준

정책:

```text
bootElapsed는 현재 awake boot session 기준 경과 시간이다.
Deep Sleep continuity는 전제로 하지 않는다.
v1.3에서는 Deep Sleep을 사용하지 않는다.
bootCount와 timeSessionId가 일치하는 경우에만 RTS 기반 로그 시간 보정을 허용한다.
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

## 21.2 Deep Sleep policy

BackupDiveComputer v1.3에서는 Deep Sleep을 사용하지 않는다.

정책:

```text
Deep Sleep 금지
Light Sleep만 향후 검토
OLED power save/off 검토
GPS OFF
BLE OFF
Wi-Fi OFF
ESP32는 awake 또는 light sleep 기반으로 유지
```

Deep Sleep을 사용하지 않는 이유:

```text
Deep Sleep wake 후 millis()/esp_timer 기반 elapsed continuity가 깨질 수 있다.
timeSessionId, bootElapsed, tissue/advisory continuity 판단이 복잡해진다.
RTS가 없는 로그의 시간 보정에서 오류 가능성이 커진다.
```

따라서 향후 power management 코드는 반드시 다음 정책 플래그를 따라야 한다.

```cpp
#define BDC_DISABLE_DEEP_SLEEP 1
```

`BDC_DISABLE_DEEP_SLEEP`이 1인 경우 다음 호출은 사용하지 않는다.

```cpp
esp_deep_sleep_start()
ESP.deepSleep()
```

bootElapsed는 Deep Sleep continuity를 전제로 하지 않는다.
bootElapsed는 하나의 awake boot session 안에서만 유효한 elapsed time으로 정의한다.

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
v1.3.6-dev
```

문서 수정 후 commit 예:

```bash
git checkout dev/v1.3
git status
git add docs/DEVELOPMENT_v1.3.md
git commit -m "Update DEVELOPMENT docs for v1.3.6-dev"
git push origin dev/v1.3
```

태그를 최신 문서 포함 위치로 이동하려면, 개인 개발 단계에서만 다음을 사용할 수 있다.

```bash
git tag -f v1.3.6-dev
git push origin -f v1.3.6-dev
```

주의:

```text
이미 외부에서 태그를 사용 중이면 tag force push는 피한다.
```

---

# 28. v1.3.6-dev 완료된 주요 항목

현재 완료 또는 부분 구현 완료:

```text
FW_VERSION v1.3.6-dev
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

# 29. v1.3.6-dev 이후 남은 주요 작업

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

---

# v1.3.6 Remaining Work Toward Tag

v1.3.6 태그 생성 전 남은 핵심 작업은 다음과 같다.

## 1. 검증

- Wokwi에서 일반 다이빙 로그 저장 확인
- RTS 확보 전 다이빙 시작 시 RelativeOnly 로그 생성 확인
- 이후 GPS RTS 획득 시 TimeCorrected 보정 확인
- bootCount mismatch 시 자동 보정하지 않는지 확인
- timeSessionId mismatch 시 자동 보정하지 않는지 확인
- BLE access window 15분 정책 확인
- Dive 진입 시 GPS/BLE 즉시 OFF 확인

## 2. 이벤트 영구 저장

현재 대부분의 이벤트는 Serial log 중심이다.

남은 작업:

- eventCount 실제 증가
- DiveEvent 저장
- logDiveEvent()를 storage 기록으로 확장
- MISSED DECO event 저장
- DECO REENTRY event 저장
- DECO CLEARED AFTER REENTRY event 저장
- advisory started / ended event 저장

## 3. MOD / Nitrox warning

남은 작업:

- FO2 > 21%일 때 MOD 표시
- MOD 초과 시 PPO2 HIGH 또는 MOD EXCEEDED 표시
- MOD warning event 저장
- EAN32 / EAN36 시나리오 테스트

## 4. UI priority 정리

남은 작업:

- LOW BAT
- ASCENT FAST
- DECO.STOP
- MISSED DECO
- MOD EXCEEDED
- S.STOP SKIPPED

위 경고가 겹칠 때 표시 우선순위와 순환 표시 정책을 정리한다.

## 5. BLE 실제 서비스 설계

남은 작업:

- BLE service UUID
- battery characteristic
- device info characteristic
- log list characteristic
- log download characteristic
- BLE time sync characteristic
- FO2 setting characteristic
