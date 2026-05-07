# CHAT_CONTEXT_v1.3.md

BackupDiveComputer v1.3 개발을 새로운 채팅창에서 이어가기 위한 전체 컨텍스트 문서입니다.

이 문서는 단순 요약이 아니라, 현재까지 확정된 설계 결정, GitHub 상태, 문서 정리 방향, 알고리즘 정책, UI 정책, 향후 구현 순서를 한 번에 복원하기 위한 기준 문서입니다.

---

## 1. Repository 정보

Repository:

- https://github.com/DOCKERNOIN-VibeCoding/Prototype-BackupDiveComputer

현재 기준 개발 브랜치:

- `dev/v1.3`

중요:  
`main`이 아니라 `dev/v1.3` 브랜치를 기준으로 작업을 이어가야 합니다.

최근 확인된 `dev/v1.3` 주요 커밋:

- `d177369b396aed3e5528a04a3e702aa0991a7b68`
  - Message: `Document Air Nitrox ready gas config and MOD policy`
  - URL: https://github.com/DOCKERNOIN-VibeCoding/Prototype-BackupDiveComputer/commit/d177369b396aed3e5528a04a3e702aa0991a7b68

이전 주요 커밋:

- `3599365835acabf5fb662204c7b4c14a4bde0ffd`
  - Message: `Update v1.3 deco violation and reentry policy docs`
- `9caf1b47d3d3f8dcda313c12bfcb45d5cc1172df`
  - Message: `Fix safety stop skipped flow before dive end`
- `450c5213837b5cc39b6f295e0ffb7b402d2d7c23`
  - Message: `Rename development version scheme to v1.3`

---

## 2. 현재 docs 구성

`dev/v1.3` 브랜치의 `/docs` 폴더에는 다음 문서가 있다.

- `docs/CHANGELOG.md`
- `docs/CHAT_CONTEXT_v1.3.md`
- `docs/DEVELOPMENT_v1.3.md`
- `docs/TODO.md`

새 채팅창에서 작업을 이어갈 때는 다음 순서로 읽으면 된다.

1. `docs/CHAT_CONTEXT_v1.3.md`
2. `docs/DEVELOPMENT_v1.3.md`
3. `docs/TODO.md`
4. `docs/CHANGELOG.md`

이 문서의 목적은 새 채팅에서 GitHub URL만 제시했을 때도, 현재 v1.3 개발 방향을 빠르게 복원할 수 있게 하는 것이다.

---

## 3. 프로젝트의 기본 성격

BackupDiveComputer는 다음 성격의 장비이다.

- ESP32-S3 기반 백업 다이브 컴퓨터
- 레크리에이션 다이빙용
- 스탠드얼론 자동 작동 방식
- 버튼 없는 장치
- 주 컴퓨터가 아니라 보조 컴퓨터
- 화면 크기는 128×64 OLED 기준
- 복잡한 조작 메뉴보다 자동 판단과 단순 표시를 우선한다

중요한 제약:

- 버튼 입력이 없다.
- 다이버가 화면을 넘기거나 메뉴를 선택할 수 없다.
- 따라서 Deco Plan 전체 목록을 사용자가 수동으로 확인하는 방식은 적합하지 않다.
- 화면은 현재 필요한 정보만 자동으로 순환 또는 우선순위에 따라 표시해야 한다.

---

## 4. 지원 범위

### 4.1 현재 목표

현재 v1.3 목표는 다음 기능을 안정적으로 구조화하는 것이다.

- 수심 표시
- 수온 표시
- 다이브 시간 표시
- 상승 속도 표시
- Safety Stop
- DECO.STOP
- NDL 계산
- Surface Interval
- No-Fly
- 배터리 UI
- 로그 저장 구조
- Subsurface XML 내보내기 준비
- Air/EAN21 기준 알고리즘
- 향후 Nitrox 지원을 위한 구조 준비

### 4.2 지원하지 않는 범위

현재 프로젝트는 테크니컬 다이빙 컴퓨터가 아니다.

따라서 다음은 v1.3 범위에서 제외한다.

- 더블 탱크
- 멀티 가스 전환
- Trimix
- Heliox
- CCR
- pSCR
- bailout gas planning
- 사용자 버튼 기반 메뉴
- 수동 Deco Planner 화면
- Shearwater/OSTC4 수준의 full technical deco table UI

---

## 5. Air / Nitrox 정책

### 5.1 중요한 정정

일반 공기는 EAN32가 아니다.

- Air = EAN21
- 산소 비율 FO2 약 21%
- 질소 비율 FN2 약 79%

EAN32는 산소 32%의 Nitrox 혼합기체이다.

따라서 기본값은 반드시 `EAN21` 또는 `Air`로 두어야 한다.

### 5.2 현재 개발 단계 정책

지금 당장 Nitrox를 완전 지원하지 않더라도, 코드 구조는 Nitrox-ready로 만들어야 한다.

즉, Bühlmann 알고리즘에서 질소 비율을 하드코딩하지 않고, 설정값에서 산소 비율을 읽고, 그에 따라 질소 비율을 계산해야 한다.

기본 설정:

- `DIVE_GAS_FO2_PERCENT = 21`
- `DIVE_GAS_PPO2_MAX_BAR = 1.4`
- 허용 준비 범위: `21%` ~ `40%`
- 현재 UI에서는 변경 불가
- 향후 앱/BLE 설정을 통해 변경 가능하도록 구조화

예상 config 구조:

    // include/config.h

    #pragma once

    #define DIVE_GAS_FO2_PERCENT       21
    #define DIVE_GAS_FO2_MIN_PERCENT   21
    #define DIVE_GAS_FO2_MAX_PERCENT   40

    #define DIVE_GAS_PPO2_MAX_BAR      1.4f

알고리즘 내부에서는 다음처럼 사용한다.

    fo2 = DIVE_GAS_FO2_PERCENT / 100.0f
    fn2 = 1.0f - fo2

기존처럼 `FN2 = 0.79`를 코드 곳곳에 직접 박아두면 안 된다.

### 5.3 MOD 정책

FO2가 21%보다 높을 경우, 즉 Nitrox 설정일 경우에는 다이빙 화면에 MOD를 항상 표시하는 방향으로 준비한다.

기준 최대 산소분압:

- ppO2 max = 1.4 bar

MOD 공식:

    MOD(m) = ((ppO2_max / FO2) - 1) * 10

예:

- Air / EAN21
  - FO2 = 0.21
  - MOD ≈ 56.6 m
  - 레크리에이션 한계와 별개로, Nitrox 경고 목적의 상시 MOD 표시는 필요하지 않음

- EAN32
  - FO2 = 0.32
  - MOD = ((1.4 / 0.32) - 1) × 10
  - MOD ≈ 33.75 m
  - 표시 예: `MOD 33m`

MOD 초과 시:

- 경고를 표시한다.
- 단, 컴퓨터를 hard lock하지 않는다.
- 다른 핵심 정보가 사라지면 안 된다.
- ppO2 경고는 DECO.STOP, 상승속도 경고 등과 함께 우선순위에 따라 표시한다.

---

## 6. DECO.STOP 핵심 정책

### 6.1 용어

사용자에게 표시하는 핵심 용어는 `DECO.STOP`으로 유지한다.

`DecoPlan`, `Deco Schedule`, `Decompression Table` 같은 용어는 내부 설계 설명에는 사용할 수 있지만, 실제 화면에서는 단순하게 `DECO.STOP` 중심으로 표시한다.

Safety Stop과 Deco Stop은 반드시 구분한다.

- Safety Stop: `S-STOP`
- Mandatory decompression stop: `DECO.STOP`

### 6.2 버튼 없는 장치에 맞춘 원칙

본 장치는 버튼이 없으므로 Shearwater나 OSTC4처럼 다이버가 전체 Deco Plan 화면을 열어보는 방식은 적합하지 않다.

대신 다음 원칙을 따른다.

- 전체 감압 계획표를 표시하지 않는다.
- 현재 수행해야 할 하나의 DECO.STOP만 표시한다.
- 해당 정지가 완료되면 자동으로 다음 정지 수심과 시간을 표시한다.
- 다이버가 조작하지 않아도 화면이 자동으로 진행된다.
- 체내 질소량은 실시간으로 계속 추적한다.
- 각 정지 시간은 고정값이 아니라 실시간 계산값이다.

---

## 7. DECO.STOP 6단계 수심 사다리

v1.3에서 확정한 DECO.STOP 수심 사다리는 다음 6단계이다.

1. 18 m
2. 15 m
3. 12 m
4. 9 m
5. 6 m
6. 3 m

이 구조는 Shearwater, OSTC4 등 상업용 컴퓨터가 감압정지를 여러 수심으로 나누어 제시하는 방식에서 참고한 것이다.

다만 우리 컴퓨터는 full plan을 보여주지 않고, 현재 수행해야 할 stop만 보여준다.

예:

- `DECO.STOP 12m 02:00`
- 완료 후 자동 전환:
- `DECO.STOP 9m 03:00`
- 완료 후 자동 전환:
- `DECO.STOP 6m 05:00`
- 완료 후:
- `SURFACE OK`

### 7.1 첫 DECO.STOP 선택

첫 stop은 고정적으로 18m부터 시작하는 것이 아니라, 현재 조직 질소량과 raw ceiling을 기준으로 선택한다.

raw ceiling이 어느 수심까지 상승을 제한하는지 계산한 뒤, 다음 사다리 수심으로 매핑한다.

예시 매핑:

- ceiling ≤ 3 m → 3 m stop
- ceiling ≤ 6 m → 6 m stop
- ceiling ≤ 9 m → 9 m stop
- ceiling ≤ 12 m → 12 m stop
- ceiling ≤ 15 m → 15 m stop
- ceiling ≤ 18 m → 18 m stop

감압 부담이 작으면 3m 또는 6m만 생성될 수 있다.

감압 부담이 크면 다음처럼 더 많은 단계가 생성될 수 있다.

- 18m → 15m → 12m → 9m → 6m → 3m

### 7.2 최대 DECO.STOP 수심

OSTC4 화면 사례에서는 18m가 가장 깊은 데코스탑으로 표시되는 예시가 있었다.

따라서 본 프로젝트에서도 사용자에게 제시하는 DECO.STOP의 최대 수심은 18m로 한다.

단, raw ceiling이 18m보다 깊은 경우에는 18m stop으로 거짓 안내하지 않는다.

이 경우에는 다음과 같은 경고성 안내를 표시한다.

- `CEIL >18m`
- `DO NOT ASCEND`
- `HOLD DEPTH`

의미:

- 컴퓨터가 18m보다 깊은 ceiling을 감지했다.
- 아직 18m stop으로 올라가라고 안내할 단계가 아니다.
- 다이버는 상승하지 말아야 한다.
- ceiling이 18m 이하로 내려가면 그때부터 18m DECO.STOP을 표시한다.

---

## 8. DECO.STOP 시간 계산 정책

DECO.STOP 시간은 고정값이 아니다.

다음처럼 계산한다.

1. 현재 수심, 시간, gas FO2/FN2를 바탕으로 조직 질소량을 업데이트한다.
2. Bühlmann 기반으로 현재 raw ceiling을 계산한다.
3. raw ceiling을 18/15/12/9/6/3m 사다리에 매핑한다.
4. 현재 stop 수심에서 off-gassing을 시뮬레이션한다.
5. 다음 shallower stop으로 이동 가능한 시점까지 남은 시간을 추정한다.
6. 추정 시간은 주기적으로 갱신한다.

즉, 화면에 표시되는 `02:00`, `03:00` 같은 값은 실시간으로 변할 수 있다.

표시 시간 갱신 주기 예:

- 5초 또는 10초마다 재계산
- 또는 조직 업데이트 tick에 맞춰 재계산

### 8.1 stop 완료 조건

어떤 stop이 완료되었다는 것은 단순히 카운트다운이 0이 되었다는 뜻이 아니다.

원칙적으로는 다음 조건을 만족해야 한다.

- 현재 조직 상태에서 다음 shallower ladder depth까지 상승해도 ceiling 위반이 아님

예:

- 현재 stop: 12m
- 다음 stop: 9m
- 12m stop 완료 조건:
  - raw ceiling ≤ 9m

현재 stop이 6m이면 다음 stop은 3m이다.

- 6m stop 완료 조건:
  - raw ceiling ≤ 3m

3m stop 완료 조건:

- raw ceiling ≤ 0m 또는 surface 허용 기준 만족
- 이후 `DECO CLEAR` 또는 `SURFACE OK`

---

## 9. DECO.STOP 화면 정책

화면은 작은 128×64 OLED 기준으로 설계한다.

DECO.STOP 발생 시 가장 중요한 정보만 표시한다.

예시:

    DECO.STOP
    12m  02:00
    HOLD

상승해야 하는 경우:

    DECO.STOP
    12m  02:00
    ASCEND

너무 얕게 올라간 경우:

    DECO.STOP
    12m  02:00
    DOWN!

ceiling 위반 또는 stop depth보다 얕은 경우에는 `DOWN!`을 명확히 보여준다.

### 9.1 전체 Deco Plan 미표시

본 장치는 보조 컴퓨터이므로 전체 Deco Plan을 보여줄 필요가 없다.

금지 또는 비권장:

- 18m 1분, 15m 1분, 12m 2분, 9m 3분, 6m 5분 전체 목록 표시
- 사용자가 버튼으로 plan page를 넘겨보는 구조
- 복잡한 runtime table UI

권장:

- 현재 필요한 stop 하나만 표시
- 완료 후 다음 stop 자동 표시
- 다이버가 조작 없이 따라갈 수 있는 흐름

---

## 10. DECO.STOP 위반 정책

v1.3에서 중요한 원칙:

- DECO.STOP 위반 후에도 기기를 hard lock하지 않는다.
- `LOCKED`, `ERROR`, `USE TABLE ONLY` 같은 완전 잠금형 동작은 하지 않는다.
- 다이버가 재입수하면 계속 조직 상태를 추적하고 다시 DECO.STOP 안내를 제공한다.

### 10.1 위반 상황

다음 상황은 violation으로 본다.

- mandatory DECO.STOP이 남아 있는데 수면 도달
- stop depth보다 얕은 곳에 장시간 머무름
- ceiling 위반 상태로 상승
- stop 수행 전 다이브 종료

### 10.2 surfacing after violation

DECO.STOP을 끝내지 않고 수면에 올라온 경우:

- `MISSED DECO`
- `NO DIVE 48H`
- 또는 이에 준하는 강한 advisory 표시

단, 이것은 hard lock이 아니다.

기기는 계속 켜지고, 상태를 보여주며, 재입수 시 조직 계산을 이어간다.

### 10.3 48시간 post-violation advisory

DECO.STOP 위반 후 수면에 도달하면 48시간 post-violation advisory를 유지한다.

정책:

- `postViolationAdvisory = true`
- 시작 시각 기록
- 48시간 후 종료
- 재입수해도 조직 계산은 계속 수행
- advisory는 안전 경고로 유지

표시 예:

    MISSED DECO
    NO DIVE 48H

또는

    DECO VIOL
    47:32 LEFT

### 10.4 re-entry 정책

DECO.STOP 위반 후 재입수한 경우:

- 컴퓨터는 동작을 멈추지 않는다.
- 현재 tissue state를 기준으로 다시 ceiling을 계산한다.
- 필요한 경우 DECO.STOP을 다시 표시한다.
- DECO.STOP 안내가 `NO DIVE` advisory보다 우선한다.

즉, 물속에서는 실제 생리학적 위험을 줄이는 안내가 우선이다.

---

## 11. DECO 관련 이벤트 로그

다음 이벤트 타입을 로그에 남길 수 있도록 준비한다.

- `EVENT_DECO_REQUIRED`
- `EVENT_DECO_STOP_STARTED`
- `EVENT_DECO_STOP_COMPLETED`
- `EVENT_DECO_CLEARED`
- `EVENT_DECO_MISSED`
- `EVENT_DECO_VIOLATION_SURFACED`
- `EVENT_DECO_REENTRY`
- `EVENT_DECO_CLEARED_AFTER_REENTRY`
- `EVENT_POST_VIOLATION_ADVISORY_STARTED`
- `EVENT_POST_VIOLATION_ADVISORY_ENDED`
- `EVENT_CEILING_EXCEEDED`
- `EVENT_CEIL_GT_18M`

이벤트에는 가능하면 다음 값을 함께 기록한다.

- timestamp
- depth
- current ceiling
- required stop depth
- remaining stop time
- gas FO2
- dive runtime

---

## 12. Safety Stop 정책

Safety Stop은 DECO.STOP과 별개이다.

Safety Stop 표시명:

- `S-STOP`

DECO.STOP 표시명:

- `DECO.STOP`

둘을 혼동하면 안 된다.

### 12.1 Safety Stop 기본 흐름

일반 레크리에이션 다이빙에서 mandatory decompression이 없는 경우에만 safety stop을 적용한다.

예상 조건:

- 수심 10m 이상 다녀온 후
- 6m 근처 safety stop zone 진입
- 3분 카운트다운
- 수심 범위를 벗어나면 pause 또는 skip 처리

### 12.2 DECO.STOP 우선

DECO.STOP이 필요한 상황에서는 Safety Stop을 표시하지 않는다.

우선순위:

1. 상승속도/치명적 경고
2. DECO.STOP
3. MOD/ppO2 경고
4. Safety Stop
5. 일반 dive info

---

## 13. Bühlmann / NDL 계산 정책

v1.3의 핵심은 Bühlmann 알고리즘 구조를 정리하는 것이다.

정책:

- 조직 질소량은 실시간으로 추적한다.
- NDL은 현재 tissue loading과 gas FN2를 기준으로 계산한다.
- FN2는 `Config.h`의 FO2에서 계산한다.
- Air/EAN21 기준으로도 구조는 Nitrox-ready여야 한다.
- 향후 FO2를 앱에서 변경해도 알고리즘 변경 없이 동작해야 한다.

주의:

- 지금은 레크리에이션 컴퓨터이므로 복잡한 technical multi-gas algorithm까지 구현하지 않는다.
- 단일 gas 기준으로 충분하다.
- 기본 gas는 Air/EAN21이다.

---

## 14. UI 우선순위

버튼 없는 장치이므로 UI는 자동 우선순위 기반이어야 한다.

권장 우선순위:

1. 치명적 안전 경고
   - 너무 빠른 상승
   - ceiling 위반
   - `DOWN!`
   - `CEIL >18m`
2. DECO.STOP
3. MOD / ppO2 경고
4. Safety Stop
5. 배터리 경고
6. 일반 다이브 정보
   - depth
   - dive time
   - temperature
   - NDL
7. surface 정보
   - LAST
   - MAX
   - TMP
   - SURFACE
   - N-FLY

중요:

- MOD 경고가 떠도 DECO.STOP 안내를 완전히 가리면 안 된다.
- 배터리 팝업도 DECO.STOP보다 우선하면 안 된다.
- 안전상 즉시 행동이 필요한 정보가 항상 우선한다.

---

## 15. Surface 화면 정책

Surface 화면은 기존 v1.2에서 이어진 중요한 기능이다.

표시 항목:

- `LAST`
- `MAX`
- `TMP`
- `SURFACE`
- `N-FLY`

정책:

- `LAST`는 마지막 다이브 시작 날짜를 표시한다.
- Surface interval은 마지막 다이브 종료 시각 기준으로 계산한다.
- No-Fly는 epoch 기반 종료 시각으로 계산한다.

관련 변수:

- `lastDiveStartEpochSec_`
- `lastDiveDurationSec_`
- `lastDiveEndEpochSec_`
- `lastDiveMaxDepthM_`
- `lastDiveMinTempC_`
- `noFlyEndEpochSec_`

계산:

- `lastDiveEndEpochSec_ = lastDiveStartEpochSec_ + lastDiveDurationSec_`
- `surfaceIntervalSec = nowEpochSec - lastDiveEndEpochSec_`
- `noFlyRemainSec = noFlyEndEpochSec_ - nowEpochSec`

---

## 16. No-Fly 정책

No-Fly는 고정 JSON 값만 표시하는 방식에서 벗어나 epoch 기반으로 계산해야 한다.

정책:

- 마지막 다이브 종료 시각을 저장한다.
- No-Fly 종료 시각을 저장한다.
- 현재 시각과 비교하여 남은 시간을 계산한다.
- 재부팅 후에도 유지되어야 한다.
- 향후 로그 저장소에서 마지막 dive 정보를 읽어 복원한다.

---

## 17. 시간 동기화 정책

GPS가 항상 가능한 것은 아니다.

따라서 시간 처리는 다음 구조가 필요하다.

- `bootCount`
- `timeSessionId`
- `timeStatus`
- relative epoch
- GPS sync state
- BLE/app sync state

정책:

- GPS가 없으면 임시 시간 또는 상대 시간을 사용한다.
- GPS/BLE로 실제 시간이 확보되면 같은 `timeSessionId` 안에서 로그 시간을 보정할 수 있다.
- `timeSessionId`가 달라진 로그를 임의로 재보정하면 안 된다.

---

## 18. 로그 저장 정책

내부 로그는 XML이 아니라 compact binary 형식으로 저장한다.

Subsurface XML은 나중에 export tool에서 생성한다.

예상 구조:

- `DiveLogHeader`
- `DiveSample`
- `DiveEvent`

### 18.1 DiveLogHeader 예시 필드

- magic
- version
- diveId
- startEpochSec
- durationSec
- maxDepthCm
- minTempDeciC
- gasFo2Percent
- ppO2MaxBar
- sampleIntervalSec
- timeSessionId
- bootCount
- noFlyEndEpochSec
- eventCount
- sampleCount

### 18.2 DiveSample 예시 필드

- elapsedSec
- depthCm
- tempDeciC
- ndlSec
- ceilingCm
- decoStopDepthCm
- decoStopRemainSec
- ascentRateCmPerMin
- batteryPercent

### 18.3 DiveEvent 예시 필드

- elapsedSec
- eventType
- depthCm
- value1
- value2

---

## 19. Subsurface XML export 정책

기기 내부에 XML을 직접 저장하지 않는다.

정책:

- 내부 로그는 compact binary
- PC tool 또는 앱에서 XML로 변환
- 변환 도구 후보:
  - `tools/bdc_to_subsurface_xml.py`

Subsurface XML에 포함할 정보:

- dive date/time
- duration
- max depth
- temperature
- samples
- events
- GPS dive site
- cylinder/gas 정보
- FO2
- deco events
- safety stop events
- violation events

---

## 20. BLE 정책

BLE는 항상 켜두지 않는다.

기본 정책:

- BLE log download는 충전 중에만 허용
- 일반 dive 중 BLE는 꺼둔다
- 배터리 절약 우선
- 향후 앱에서 FO2, ppO2 설정 변경 가능하도록 구조화

향후 BLE characteristic 후보:

- device info
- battery status
- log list
- log download
- gas FO2 setting
- ppO2 max setting
- time sync
- firmware/version info

---

## 21. 전원 및 배터리 정책

ESP32-S3 기반 장치의 배터리 사용 시간을 늘리기 위해 standby에서는 저전력 정책을 적용한다.

정책:

- 기본 standby는 Light Sleep 중심
- Deep Sleep은 시간 연속성 문제 때문에 신중히 사용
- GPS OFF
- BLE OFF
- Wi-Fi OFF
- OLED OFF 또는 dim
- 충전 시작 시 wake 가능
- Qi/touch wake 가능성 검토

배터리 UI:

- 10% 미만 경고
- 2분마다 2초 팝업 표시
- 충전 중 아이콘 blink
- 100%에서 `FULL`

중요:

- 배터리 경고가 DECO.STOP을 가리면 안 된다.
- 물속에서는 안전 정보가 우선이다.

---

## 22. 시뮬레이션 정책

현재 개발은 PlatformIO/Wokwi 기반 시뮬레이션을 사용한다.

Scenario JSON을 사용하여 depth/temperature/time 데이터를 입력하고, 이를 header로 변환한다.

관련 파일:

- `scenarios/previous_dive_surface.json`
- `tools/generate_scenario.py`
- `include/generated_scenario.h`

정책:

- JSON scenario를 통해 다양한 dive profile 테스트
- generated_scenario.h는 빌드 시 자동 생성
- 실제 센서 연동 전 알고리즘/UI를 먼저 검증

---

## 23. 테스트해야 할 주요 시나리오

v1.3에서 반드시 테스트할 시나리오:

### 23.1 기본 Surface 시나리오

- 이전 다이브 기록 존재
- LAST 표시
- MAX 표시
- TMP 표시
- SURFACE interval 계산
- N-FLY 남은 시간 계산

### 23.2 Air/EAN21 NDL 시나리오

- FO2 = 21%
- FN2 = 79%
- NDL 계산이 기존 Air 기준과 일치하는지 확인
- MOD 상시 표시 없음 또는 비강조

### 23.3 EAN32 준비 시나리오

- FO2 = 32%
- FN2 = 68%
- NDL이 Air보다 길어지는 방향인지 확인
- MOD ≈ 33m 계산 확인
- 수심 33m 초과 시 MOD 경고 확인

### 23.4 짧은 deco 시나리오

- 감압 부담 작음
- 6m 또는 3m stop만 생성
- 전체 ladder를 강제로 다 표시하지 않음

### 23.5 중간 deco 시나리오

- 12m → 9m → 6m → 3m 등 일부 ladder 생성
- 각 stop 완료 후 자동 전환

### 23.6 긴 deco 시나리오

- 18m → 15m → 12m → 9m → 6m → 3m 전체 ladder 가능
- 18m보다 깊은 ceiling 발생 시 `CEIL >18m` 표시

### 23.7 missed deco 시나리오

- DECO.STOP 남은 상태에서 수면 도달
- `MISSED DECO`
- 48h advisory 시작
- event log 기록

### 23.8 re-entry 시나리오

- missed deco 후 재입수
- hard lock 없이 tissue 계산 계속
- 필요한 DECO.STOP 재표시
- re-entry event 기록

### 23.9 safety stop skip 시나리오

- Safety Stop 도중 수심 이탈
- pause 또는 skipped 처리
- 다이브 종료 흐름 정상 확인

---

## 24. 참고한 상업용 컴퓨터 조사 내용

Shearwater, OSTC4 등 상업용 컴퓨터는 Bühlmann ZH-L16 계열 알고리즘과 Gradient Factor를 사용하며, deco ceiling을 계산한 뒤, 이를 일정 stop depth로 나누어 표시한다.

관찰한 핵심:

- Deco Stop은 하나의 수심에서만 진행되지 않는다.
- 12m, 9m, 6m 등 여러 수심으로 나누어 진행된다.
- OSTC4 사례에서는 18m가 가장 깊은 deco stop으로 보였다.
- Shearwater는 live deco plan을 통해 여러 stop depth/time을 보여줄 수 있다.
- 그러나 우리 장치는 버튼이 없고 보조 컴퓨터이므로 full plan 표시보다 현재 stop 자동 안내가 적합하다.

참고 URL:

- https://shearwater.com/en-ca/blogs/community/evolution-of-dive-planning
- https://wreckedinmyrevo.com/2020/02/15/shearwater-feature-requests/
- https://wreckedinmyrevo.com/2020/07/19/shearwater-live-deco-plan/
- https://wreckedinmyrevo.com/2022/08/27/shearwater-live-deco-plan-part-ii/
- https://hwdiving.com/downloads/hwOS_tech_sport_EN_WEB.pdf
- https://www.heinrichsweikamp.net/downloads/OSTC_GF_web_en.pdf
- https://dan.org/alert-diver/article/nitrox/

---

## 25. 문서 정리 이슈

업로드된 v1.3 문서에서 확인된 정리 필요 사항:

1. 일부 문서 제목이 아직 v1.2로 되어 있을 수 있다.
2. `dev/v1.2` push 명령이 남아 있으면 `dev/v1.3`으로 수정해야 한다.
3. TODO.md 내부 중복 Air/Nitrox 섹션은 정리해야 한다.
4. Air를 EAN32로 착각한 내용이 있으면 반드시 삭제해야 한다.
5. 기본값은 Air/EAN21이다.
6. Markdown code block 안에 문서 heading이 들어가 실제 문서가 코드처럼 렌더링되는 부분이 있으면 제거해야 한다.
7. v1.3 정책은 문서 끝에 붙인 임시 섹션처럼 두지 말고, 번호가 있는 정식 섹션으로 통합해야 한다.

권장 커밋 메시지:

- `Clean up v1.3 docs and clarify FO2 MOD policy`

---

## 26. Git 작업 기준

문서 수정 후 작업 순서:

    git checkout dev/v1.3
    git status
    git add docs/CHAT_CONTEXT_v1.3.md docs/DEVELOPMENT_v1.3.md docs/TODO.md
    git commit -m "Clean up v1.3 docs and clarify FO2 MOD policy"
    git push origin dev/v1.3

확인:

    git status
    git log --oneline -5

정상 상태:

- working tree clean
- 최신 커밋이 `dev/v1.3`에 존재
- GitHub에서 docs 파일이 최신 내용으로 보임

---

## 27. 새 채팅창에서 이어갈 때 사용할 프롬프트

새로운 채팅창에서 다음처럼 요청하면 된다.

    아래 GitHub 저장소의 dev/v1.3 브랜치를 기준으로 BackupDiveComputer 개발을 이어가 주세요.

    Repository:
    https://github.com/DOCKERNOIN-VibeCoding/Prototype-BackupDiveComputer

    먼저 docs/CHAT_CONTEXT_v1.3.md, docs/DEVELOPMENT_v1.3.md, docs/TODO.md를 읽고,
    현재 확정된 정책을 유지한 상태에서 다음 구현 작업을 제안해 주세요.

    중요한 전제:
    - 버튼 없는 스탠드얼론 보조 다이브 컴퓨터입니다.
    - 레크리에이션 단일 gas 컴퓨터입니다.
    - 기본 gas는 Air/EAN21입니다.
    - Nitrox는 향후 지원을 위해 FO2 설정 구조만 먼저 준비합니다.
    - ppO2 max는 1.4 bar입니다.
    - DECO.STOP은 18m, 15m, 12m, 9m, 6m, 3m ladder를 사용합니다.
    - 전체 Deco Plan은 표시하지 않고 현재 필요한 DECO.STOP만 표시합니다.
    - DECO.STOP 시간은 tissue nitrogen loading을 기반으로 실시간 계산합니다.
    - missed deco 후에도 hard lock하지 않고 48시간 advisory를 표시합니다.

---

## 28. v1.3에서 다음으로 구현할 우선순위

현재 문서 정리 후 코드 구현 우선순위는 다음과 같다.

1. `include/config.h`에 FO2/ppO2 설정 추가
2. Bühlmann 계산에서 FN2 하드코딩 제거
3. FO2 기반 NDL 계산 구조 적용
4. MOD 계산 함수 추가
5. FO2 > 21%일 때 MOD 표시 준비
6. ppO2 초과 경고 UI 준비
7. DECO.STOP ladder 자료구조 추가
8. raw ceiling을 ladder depth로 매핑
9. 현재 stop 하나만 표시하는 UI 구현
10. stop 완료 후 다음 stop 자동 전환
11. `CEIL >18m` 경고 구현
12. missed deco / re-entry / 48h advisory 상태 구현
13. event log 타입 추가
14. scenario JSON 테스트 추가
15. Subsurface export에 gas FO2 반영

---

## 29. 핵심 설계 문장

v1.3의 핵심 설계는 다음 한 문장으로 요약할 수 있다.

BackupDiveComputer v1.3은 버튼 없는 레크리에이션용 보조 다이브 컴퓨터로서, 기본 Air/EAN21 단일 gas를 사용하되 향후 Nitrox 설정을 위해 FO2/ppO2 구조를 미리 갖추고, Bühlmann 기반 tissue loading으로 NDL과 DECO.STOP을 실시간 계산하며, mandatory decompression이 필요한 경우 18m-15m-12m-9m-6m-3m ladder 중 현재 필요한 stop 하나만 자동으로 표시하고, missed deco 후에도 hard lock 없이 48시간 advisory와 re-entry 계산을 유지한다.

---

## 30. 절대 유지해야 할 결정 사항

다음 결정은 이후 개발에서 임의로 뒤집으면 안 된다.

- 기본 gas는 Air/EAN21이다.
- Air를 EAN32로 처리하면 안 된다.
- ppO2 max 기준은 1.4 bar이다.
- Nitrox는 지금 UI에서 직접 변경하지 않더라도 구조는 미리 준비한다.
- 장치는 버튼이 없다.
- full Deco Plan 화면은 제공하지 않는다.
- DECO.STOP은 현재 필요한 하나만 표시한다.
- DECO.STOP ladder는 18m, 15m, 12m, 9m, 6m, 3m이다.
- stop 시간은 고정값이 아니라 실시간 계산값이다.
- Safety Stop과 DECO.STOP은 구분한다.
- DECO.STOP 위반 후 hard lock하지 않는다.
- 48시간 post-violation advisory를 유지한다.
- 재입수 시 조직 계산을 이어간다.
- 내부 로그는 compact binary로 저장하고 Subsurface XML은 export tool에서 생성한다.
- 작업 브랜치는 `dev/v1.3`이다.

