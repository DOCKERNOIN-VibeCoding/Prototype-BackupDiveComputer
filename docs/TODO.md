
```md
# TODO - BackupDiveComputer_v1.3

---
이 문서는 BackupDiveComputer_v1.3 이후 개발해야 할 항목을 체크리스트 형태로 정리한다.

현재 개발 브랜치:

```text
dev/v1.3
```

v1.3의 핵심 방향:

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

## 1. 문서 정리

- [ ] `docs/DEVELOPMENT_v1.3.md` 제목을 v1.3 기준으로 수정
- [ ] `docs/DEVELOPMENT_v1.3.md` 내부 브랜치 표기를 `dev/v1.3`으로 수정
- [ ] `docs/DEVELOPMENT_v1.3.md`의 Git 명령 예시를 `dev/v1.3` 기준으로 수정
- [ ] `docs/TODO.md` 제목을 `BackupDiveComputer_v1.3`으로 수정
- [ ] `docs/TODO.md` 내부의 v1.2 문서명 표기를 v1.3 기준으로 수정
- [ ] `docs/CHAT_CONTEXT_v1.3.md`가 최신 정책을 반영하는지 확인
- [ ] `docs/CHANGELOG.md`에 v1.3 정책 변경 내역 추가
- [ ] Markdown 코드블록 안에 실제 문서 섹션이 들어간 부분 제거
- [ ] 중복된 Air/Nitrox TODO 항목 통합
- [ ] 중복된 DECO.STOP TODO 항목 통합
- [ ] 문서 변경사항 Git commit
- [ ] 문서 변경사항 GitHub push

---

## 2. 현재 코드 안정화

- [x] `dev/v1.3` 브랜치 생성 및 사용
- [x] v1.2 epoch 기반 Surface UI 구조 유지
- [x] `lastDiveStartEpochSec_` 구조 반영
- [x] `lastDiveDurationSec_` 구조 반영
- [x] `lastDiveEndEpochSec_` 구조 반영
- [x] `noFlyEndEpochSec_` 구조 반영
- [x] `getSurfaceIntervalSec()` 추가
- [x] `getNoFlyRemainSec()` 추가
- [x] PostDive No-Fly 계산을 epoch 기준으로 정리
- [x] `src/ui.cpp` 문법 오류 수정
- [x] `include/ui.h` Surface 함수 인자 정리
- [x] `scenarios/previous_dive_surface.json` JSON 문법 정리
- [x] 불필요한 임시 코드 일부 제거
- [ ] 로컬에서 `pio run -e wokwi` 최종 빌드 확인
- [ ] Wokwi 시뮬레이터에서 Surface 화면 확인
- [ ] `git status` 기준 working tree clean 확인

---

## 3. Surface 화면 확인

- [x] `LAST` 라벨 적용
- [x] `LAST` 값을 마지막 다이빙 시작 날짜 기준으로 표시
- [x] `MAX` 표시
- [x] `TMP` 표시
- [x] `SURFACE` 표시
- [x] `N-FLY` 표시
- [ ] `SURFACE` 시간이 시뮬레이션 시간에 따라 증가하는지 확인
- [ ] `N-FLY` 시간이 시뮬레이션 시간에 따라 감소하는지 확인
- [ ] `N-FLY` 종료 시 `SAFE TO FLY` 표시 확인
- [ ] GPS 표시 확인
- [ ] 충전 아이콘 점멸 확인
- [ ] `FULL` 표시 확인
- [ ] 감압 위반 advisory가 있을 경우 Surface 화면 우선순위 확인
- [ ] 향후 FO2 > 21% 설정 시 Surface 또는 Dive 화면에서 gas label 표시 가능성 검토

---

## 4. 시나리오 시스템

- [x] `previous_dive_surface.json` preload 구조 정리
- [x] `lastDive.startEpoch` 사용
- [x] `lastDive.durationSec` 사용
- [x] `maxDepthM` 사용
- [x] `minTempC` 사용
- [x] `noFlyMinutesAtEnd` 사용
- [x] GPS preload 사용
- [x] `generate_scenario.py`에서 preload constants 생성
- [x] `generated_scenario.h`에 epoch 기반 constants 생성
- [ ] 다른 scenario JSON 파일들도 같은 구조로 정리
- [ ] `current.json` 구조 확인
- [ ] JSON validator로 모든 scenario 확인
- [ ] scenario JSON에 gas 설정 구조 추가 검토
- [ ] scenario JSON에서 `fo2Percent` override 가능하도록 설계
- [ ] scenario JSON에서 `ppO2MaxBar` override 가능하도록 설계

예상 gas scenario 구조:

```json
{
  "gas": {
    "fo2Percent": 21,
    "ppO2MaxBar": 1.4
  }
}
```

---

## 5. `generated_scenario.h` 정책

- [x] 자동 생성 파일로 유지
- [x] 직접 수정하지 않는 정책 확정
- [x] `generate_scenario.py`에서 생성
- [ ] README 또는 DEVELOPMENT 문서에 자동 생성 파일 주의사항 추가 확인
- [ ] `.gitignore`에서 generated file 관리 정책 재확인
- [ ] gas 관련 scenario constants 생성 여부 검토

예상 generated constants:

```cpp
#define SCENARIO_GAS_FO2_PERCENT 21
#define SCENARIO_GAS_PPO2_MAX_BAR 1.4f
```

---

## 6. Air / Nitrox-Ready 단일 가스 설정

BackupDiveComputer는 테크니컬 다이빙 컴퓨터가 아니라 레크리에이션 다이빙용 백업 컴퓨터이다.

지원 범위:

```text
지원:
- Single gas
- Air / EAN21
- 향후 Nitrox / EANx 21% ~ 40%

지원하지 않음:
- multi-gas switching
- double tank gas management
- trimix
- helium
- CCR
- bailout gas
- deco gas switching
```

작업 항목:

- [x] `include/config.h`에 gas 설정 상수 추가
  - [x] `DIVE_GAS_FO2_PERCENT`
  - [x] `DIVE_GAS_PPO2_MAX_BAR`
  - [x] `DIVE_GAS_FO2_MIN_PERCENT`
  - [x] `DIVE_GAS_FO2_MAX_PERCENT`

권장 기본값:

```cpp
#define DIVE_GAS_FO2_PERCENT 21
#define DIVE_GAS_PPO2_MAX_BAR 1.40f
#define DIVE_GAS_FO2_MIN_PERCENT 21
#define DIVE_GAS_FO2_MAX_PERCENT 40
```

- [x] 기본 gas를 Air / EAN21로 설정
  - [x] FO2 = 21%
  - [x] FN2 = 79%

- [x] `DIVE_GAS_FO2_PERCENT`가 산소분압이 아니라 산소분율임을 주석에 명시
- [x] EAN32는 Air가 아니라 산소 32% Nitrox임을 문서에 명시
- [x] FO2 값 유효 범위 검사 구조 추가
  - [x] 최소 21%
  - [x] 최대 40%
- [x] 현재 단계에서는 compile-time 설정 사용
- [ ] 향후 앱 설정을 위한 NVS / Preferences 저장 구조 예약
- [ ] 향후 앱에서 FO2 변경 가능하도록 구조 설계
- [ ] gas label helper 추가 검토

예상 gas label:

```text
FO2 21% -> AIR 또는 EAN21
FO2 32% -> EAN32
FO2 36% -> EAN36
```

---

## 7. Bühlmann 알고리즘과 FO2 / FN2 연결

핵심 원칙:

```text
Bühlmann 계산에서 질소 비율 0.79를 하드코딩하지 않는다.
config.h의 FO2를 읽고 FN2를 계산해 사용한다.
```

작업 항목:

- [x] Bühlmann 계산 모듈에서 FO2 값을 config에서 읽도록 구조 설계
- [x] FN2 계산 helper 추가

예상 코드:

```cpp
float fo2 = DIVE_GAS_FO2_PERCENT / 100.0f;
float fn2 = 1.0f - fo2;
```

- [x] tissue nitrogen loading 계산에 FN2 적용
- [x] NDL 계산에 FN2 적용
- [x] decompression ceiling 계산에 FN2 적용
- [x] DECO.STOP duration 계산에 FN2 적용
- [x] surface interval residual nitrogen 계산에 FN2 적용
- [x] No-Fly / desaturation 계산에 gas 설정 영향 검토
- [ ] Air / EAN21 기준 테스트 작성
- [ ] EAN32 기준 테스트 작성
- [ ] 같은 프로파일에서 EAN21과 EAN32의 NDL 차이가 발생하는지 검증
- [ ] 같은 프로파일에서 EAN21과 EAN32의 DECO.STOP 시간이 다르게 계산되는지 검증

---

## 8. MOD / ppO2 정책

기준:

```text
최대 ppO2 = 1.4 bar
```

MOD 계산식:

```text
MOD(m) = ((ppO2Max / FO2) - 1.0) * 10
```

예:

```text
FO2 = 0.32
ppO2Max = 1.4 bar

MOD = ((1.4 / 0.32) - 1.0) * 10
MOD ≈ 33.7m
```

작업 항목:

- [x] MOD 계산 helper 추가
- [x] ppO2 계산 helper 추가

예상 코드:

```cpp
float calculateMODMeters(float fo2, float ppO2MaxBar);
float calculatePpO2Bar(float fo2, float depthM);
```

- [x] 기본 ppO2 max를 1.4 bar로 설정
- [ ] FO2 > 21%일 경우 다이빙 중 MOD 상시 표시
- [ ] FO2 = 21%일 경우 MOD 표시 생략 또는 낮은 우선순위로 처리
- [ ] 현재 수심이 MOD를 초과하면 warning 발생
- [ ] warning 문구 결정
  - [ ] `PPO2 HIGH`
  - [ ] `MOD EXCEEDED`
  - [ ] `ASCEND`
- [ ] MOD 초과 경고가 발생해도 dive computer 기능은 lock하지 않음
- [ ] MOD 초과 중에도 depth/time/ascent/NDL/DECO.STOP 표시 유지
- [ ] EAN32에서 MOD 약 33.7m 계산 확인
- [ ] EAN36에서 MOD 약 28.8m 계산 확인
- [ ] MOD 초과 warning 테스트 scenario 추가

---

## 9. DECO.STOP ladder 정책

BackupDiveComputer는 전체 DecoPlan 표를 보여주지 않는다.

이유:

```text
- 버튼이 없다.
- 메뉴 조작이 없다.
- 백업용 컴퓨터이다.
- 화면이 작다.
- 현재 필요한 행동 지시가 더 중요하다.
```

지원 DECO.STOP ladder:

```text
18m
15m
12m
9m
6m
3m
```

작업 항목:

- [x] 고정 DECO ladder 추가

예상 코드:

```cpp
static constexpr float DECO_LADDER_M[] = {
    18.0f,
    15.0f,
    12.0f,
    9.0f,
    6.0f,
    3.0f
};
```

- [ ] single-stop-only DECO.STOP 동작 제거
- [x] raw decompression ceiling 계산
- [x] raw ceiling을 ladder의 첫 stop으로 매핑
- [x] ceiling <= 3m이면 first stop 3m
- [x] ceiling <= 6m이면 first stop 6m
- [x] ceiling <= 9m이면 first stop 9m
- [x] ceiling <= 12m이면 first stop 12m
- [x] ceiling <= 15m이면 first stop 15m
- [x] ceiling <= 18m이면 first stop 18m
- [x] raw ceiling > 18m인 경우 18m stop으로 속이지 않고 별도 경고 표시
- [x] 전체 DecoPlan 표시는 하지 않음
- [x] 현재 필요한 DECO.STOP 하나만 표시
- [ ] 현재 stop 완료 후 다음 shallower stop 자동 표시
- [ ] stop depth는 ladder에서 선택
- [ ] stop duration은 실시간 tissue nitrogen loading으로 계산
- [x] DECO.STOP과 S-STOP을 명확히 분리

---

## 10. DECO.STOP 실행 로직

상태 흐름:

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

작업 항목:

- [ ] `DecoRuntime` 구조체 추가 또는 확장
- [ ] `currentStopDepthM` 추가
- [ ] `nextStopDepthM` 추가
- [ ] `rawCeilingM` 추가
- [ ] `currentStopRemainSec` 추가
- [ ] `stopStarted` flag 추가
- [ ] `stopCompleted` flag 추가
- [ ] stop window 기준 정의
- [ ] stop window 안에서만 countdown 감소
- [ ] stop보다 너무 깊으면 `ASCEND` 표시
- [ ] stop window 안이면 `HOLD` 표시
- [ ] stop보다 너무 얕으면 `DOWN!` 표시
- [ ] 너무 얕은 상태에서는 감압 타이머 pause
- [ ] 현재 stop 완료 전 shallower stop으로 jump하지 않음
- [ ] 아직 stop을 시작하지 않았다면 더 깊은 stop으로 upgrade 가능
- [ ] stop 완료 시 `STOP DONE` 표시
- [ ] 다음 stop이 있으면 `NEXT 9m` 같은 안내 표시
- [ ] 마지막 3m stop 완료 시 `DECO CLEAR` 표시
- [ ] DECO clear 후 `SURFACE OK` 표시

---

## 11. DECO.STOP 시간 예측

원칙:

```text
DECO.STOP 시간은 고정값이 아니다.
현재 tissue state를 기준으로 실시간 계산한다.
```

작업 항목:

- [ ] stop remaining time estimation helper 추가
- [ ] 현재 tissue state를 복사해 예측 계산에 사용
- [ ] 현재 stop depth에 머문다고 가정하고 off-gassing 시뮬레이션
- [ ] ceiling이 다음 shallower stop depth 이하가 되는 시간을 계산
- [ ] 마지막 3m stop은 surface allowed 조건까지 계산
- [ ] 계산 간격 결정
  - [ ] 10초 단위
  - [ ] 30초 단위 검토
- [ ] 표시 단위 결정
  - [ ] 1분 단위 표시
  - [ ] 30초 단위 표시 검토
- [ ] 너무 짧은 stop 시간이 표시되지 않도록 최소 표시 시간 검토
- [ ] stop 중 주기적으로 remaining time 재계산
- [ ] 예상 시간이 증가할 수 있는 경우 UI 정책 결정
- [ ] 예상 시간이 감소하지 않고 튀는 경우 smoothing 정책 검토

---

## 12. 감압 위반 / 재입수 / 48시간 advisory 정책

기본 원칙:

```text
Hard lockout 없음
계산 계속
정보 제공 계속
위반 상태 추적
재입수 시 DECO.STOP 계산 계속
48시간 advisory 유지
```

작업 항목:

- [ ] `activeDecoViolation` 상태 추가
- [ ] `postViolationAdvisory` 상태 추가
- [ ] `advisoryEndEpochSec` 추가
- [ ] `missedStopDepthM` 추가
- [ ] `missedStopRemainSec` 추가
- [ ] `reentryCount` 추가
- [ ] `clearedAfterReentry` 추가
- [ ] 감압정지 미완료 출수 감지
- [ ] 미완료 출수 시 `activeDecoViolation = true`
- [ ] 미완료 출수 시 48시간 advisory 시작
- [ ] Surface 화면에 `MISSED DECO` 표시
- [ ] Surface 화면에 `NO DIVE ADVISED` 표시
- [ ] Surface 화면에 advisory 남은 시간 표시
- [ ] 48시간 advisory 중에도 hard lockout 하지 않음
- [ ] 48시간 advisory 중 재입수 시 `NO DIVE`보다 DECO.STOP 행동 지시 우선
- [ ] 재입수 시 tissue state 유지
- [ ] 재입수 시 DECO.STOP 재계산 계속
- [ ] 재입수 후 필요한 감압정지 완료 시 `activeDecoViolation` clear
- [ ] `postViolationAdvisory`는 48시간 동안 유지
- [ ] advisory 종료 시 event log 기록
- [ ] DAN 등은 재입수를 권장하지 않음을 문서에 명시
- [ ] 이 기능은 재입수를 권장하는 기능이 아니라 이미 재입수한 경우 정보를 제공하는 기능임을 명시

---

## 13. Safety Stop 정책

DECO.STOP과 S-STOP은 분리한다.

작업 항목:

- [x] Safety Stop skipped flow before dive end 수정
- [ ] S-STOP 표시와 DECO.STOP 표시 우선순위 정리
- [ ] DECO.STOP이 필요한 경우 S-STOP 표시하지 않음
- [ ] S-STOP은 선택적 안전정지로 취급
- [ ] DECO.STOP은 필수 감압정지로 취급
- [ ] 출수 후 S-STOP PAUSED 표시 정책 확인
- [ ] 일정 시간 내 재진입하지 않으면 S-STOP SKIPPED 표시
- [ ] S-STOP SKIPPED 후 약 30초 뒤 SURFACE mode 전환 확인
- [ ] S-STOP 관련 log event 추가 검토

---

## 14. UI 우선순위

버튼 없는 자동 컴퓨터이므로 화면 우선순위가 중요하다.

우선순위:

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

작업 항목:

- [ ] UI priority manager 설계
- [ ] `PPO2 HIGH` warning 우선순위 정의
- [ ] `MOD EXCEEDED` warning 우선순위 정의
- [ ] `DECO.STOP` 우선순위 정의
- [ ] `MISSED DECO` 우선순위 정의
- [ ] `ASCENT FAST` 우선순위 정의
- [ ] `LOW BAT` 우선순위 정의
- [ ] 경고가 겹칠 때 순환 표시 정책 결정
- [ ] DECO.STOP 중 MOD 정보 표시 방식 결정
- [ ] FO2 > 21%일 때 MOD 상시 표시 위치 결정
- [ ] OLED 128x64에서 표시 가능한 레이아웃 검토

---

## 15. 로그 저장 구조

- [ ] `include/log_format.h` 추가
- [ ] `DiveLogHeader` 구조 정의
- [ ] `DiveSample` 구조 정의
- [ ] `DiveEvent` 구조 정의
- [ ] 로그 magic/version 정의
- [ ] sample 저장 단위 결정
- [ ] depth cm 단위 저장 결정
- [ ] temp deciC 단위 저장 결정
- [ ] GPS lat/lon E7 저장 결정
- [ ] timeStatus 저장 필드 추가
- [ ] timeSessionId 저장 필드 추가
- [ ] gas FO2 저장 필드 추가
- [ ] ppO2 max 저장 필드 추가
- [ ] deco violation flag 저장 필드 추가
- [ ] post violation advisory 저장 필드 추가

---

## 16. 로그 저장 기능

- [ ] `include/log_storage.h` 추가
- [ ] `src/log_storage.cpp` 추가
- [ ] 다이빙 시작 시 log start 정보 생성
- [ ] 다이빙 중 sample 저장
- [ ] 이벤트 저장
- [ ] 다이빙 종료 시 log header 확정
- [ ] 내부 flash 저장 방식 결정
- [ ] LittleFS 사용 여부 결정
- [ ] NVS 사용 범위 결정
- [ ] 부팅 시 마지막 로그 읽기
- [ ] 마지막 로그로 Surface 화면 preload
- [ ] gas FO2를 로그에 저장
- [ ] MOD warning event 저장
- [ ] DECO.STOP started event 저장
- [ ] DECO.STOP completed event 저장
- [ ] MISSED DECO event 저장

필수 event 후보:

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

## 17. 시간 동기화와 timeSessionId

- [ ] bootCount 저장
- [ ] timeSessionId 저장
- [ ] reset reason 기록
- [ ] GPS time synced 상태 추가
- [ ] BLE time synced 상태 추가
- [ ] RTC 사용 여부 검토
- [ ] `LogTimeStatus` enum 추가
- [ ] `TimeSynced` 상태 구현
- [ ] `RelativeOnly` 상태 구현
- [ ] `TimeCorrected` 상태 구현
- [ ] `SyncFailed` 상태 구현
- [ ] GPS 시간이 나중에 잡혔을 때 로그 시간 보정
- [ ] timeSessionId가 다르면 보정하지 않도록 구현
- [ ] 48시간 advisory 계산에 epoch time 사용
- [ ] epoch time이 없을 때 advisory 상대시간 처리 정책 결정

---

## 18. 전원 관리

- [ ] Light Sleep 기본 대기 정책 설계
- [ ] Deep Sleep 사용 조건 정의
- [ ] 배터리 부족 시 보호 동작 정의
- [ ] GPS power off 제어 확인
- [ ] BLE 완전 off 제어 확인
- [ ] OLED power save/off 확인
- [ ] Wi-Fi 항상 off 확인
- [ ] Qi touch wake 동작 정의
- [ ] 충전 시작 wake 동작 정의
- [ ] 출수 직후 GPS 동작 정의
- [ ] 충전 중 BLE 동작 정의
- [ ] 실제 하드웨어 전류 측정 계획 수립
- [ ] Deep Sleep 사용 시 tissue/advisory/time continuity 영향 검토

---

## 19. 배터리 및 충전 UI

- [x] Battery Low 팝업 정책 정의
- [x] 10% 이하 경고 정책 정의
- [x] 10분 간격 팝업 정책 정의
- [x] 2초 표시 정책 정의
- [x] 충전 중 배터리 아이콘 점멸 정책 정의
- [x] 100% 충전 시 FULL 표시 정책 정의
- [ ] 실제 화면에서 Battery Low 확인
- [ ] 실제 화면에서 Charging 확인
- [ ] 실제 화면에서 FULL 확인
- [ ] DECO.STOP 중 Battery Low 경고 우선순위 확인
- [ ] MOD warning 중 Battery Low 경고 우선순위 확인
- [x] LOW BATTERY 표시 시 짧은 비프 1회 호출
- [x] 빠른 상승 위험 경고 시 3회 연속 비프 호출 (18m/min 초과시)
- [x] 실제 Wokwi/VS Code 환경에서 buzzer 소리 출력 확인

---

## 20. BLE / 앱 연동 준비

현재 기기는 버튼이 없으므로 향후 설정 변경은 앱을 통해 수행한다.

작업 항목:

- [ ] BLE는 충전 중에만 활성화하는 정책 유지
- [ ] BLE service 설계
- [ ] 로그 목록 characteristic 설계
- [ ] 로그 다운로드 characteristic 설계
- [ ] compact log 전송 방식 설계
- [ ] 전송 중 오류 처리 설계
- [ ] 전송 완료 확인 방식 설계
- [ ] 앱 설정 characteristic 설계
- [ ] FO2 설정 characteristic 예약
- [ ] ppO2 max 설정 characteristic 예약 여부 검토
- [ ] FO2 설정값 NVS 저장 구조 설계
- [ ] app-configured FO2와 compile-time default 우선순위 결정

---

## 21. Subsurface XML

- [ ] Subsurface XML export 구조 조사
- [ ] `tools/bdc_to_subsurface_xml.py` 추가
- [ ] compact log에서 XML 생성
- [ ] dive date/time 출력
- [ ] duration 출력
- [ ] sample time/depth/temp 출력
- [ ] GPS site 출력
- [ ] cylinder/gas 기본값 출력
- [ ] FO2 gas 정보 출력
- [ ] Air / EAN21 출력 방식 결정
- [ ] EAN32 출력 방식 결정
- [ ] MOD / ppO2 정보 export 여부 검토
- [ ] DECO.STOP event export 여부 검토
- [ ] Subsurface에서 import 테스트
- [ ] XML schema 호환성 확인

---

## 22. XML scenario parser

- [ ] `generate_scenario.py`에서 `.xml` 입력 감지
- [ ] Subsurface XML 읽기
- [ ] dive date/time parse
- [ ] duration parse
- [ ] sample time parse
- [ ] sample depth parse
- [ ] sample temp parse
- [ ] gas FO2 parse 가능성 검토
- [ ] maxDepth 계산
- [ ] minTemp 계산
- [ ] generated_scenario.h 생성
- [ ] XML 기반 Wokwi 시뮬레이션 테스트

---

## 23. 테스트 시나리오

### 23.1 기본 Surface 테스트

- [ ] 이전 다이빙 preload 후 Surface 화면 확인
- [ ] LAST/MAX/TMP/SURFACE/N-FLY 확인
- [ ] No-Fly 감소 확인
- [ ] SAFE TO FLY 전환 확인

### 23.2 Air / EAN21 테스트

- [ ] FO2 = 21%
- [ ] FN2 = 0.79
- [ ] MOD 표시 생략 또는 낮은 우선순위 확인
- [ ] Bühlmann 계산이 FN2 = 0.79를 사용하는지 확인
- [ ] NDL 계산 확인

### 23.3 EAN32 테스트

- [ ] FO2 = 32%
- [ ] FN2 = 0.68
- [ ] ppO2 max = 1.4 bar
- [ ] MOD 약 33.7m 계산 확인
- [ ] 다이빙 중 MOD 상시 표시 확인
- [ ] MOD 초과 시 `PPO2 HIGH` 또는 `MOD EXCEEDED` 표시 확인
- [ ] EAN21 대비 NDL 증가 확인

### 23.4 DECO.STOP ladder 테스트

- [ ] minimal deco: 3m only
- [ ] light deco: 6m -> 3m
- [ ] moderate deco: 9m -> 6m -> 3m
- [ ] heavier deco: 12m -> 9m -> 6m -> 3m
- [ ] heavy deco: 18m -> 15m -> 12m -> 9m -> 6m -> 3m
- [ ] raw ceiling >18m warning behavior
- [ ] stop window 안에서만 timer 감소
- [ ] too shallow 시 `DOWN!` 표시
- [ ] too deep 시 `ASCEND` 표시
- [ ] stop 완료 후 다음 stop 자동 표시

### 23.5 감압 위반 / 재입수 테스트

- [ ] 감압정지 미완료 출수
- [ ] `activeDecoViolation` 설정 확인
- [ ] 48시간 advisory 시작 확인
- [ ] Surface 화면에 `MISSED DECO` 표시 확인
- [ ] Surface 화면에 `NO DIVE ADVISED` 표시 확인
- [ ] 48시간 advisory 중 재입수
- [ ] 재입수 시 DECO.STOP 계산 계속 확인
- [ ] 재입수 후 DECO.STOP 완료
- [ ] `activeDecoViolation` clear 확인
- [ ] `postViolationAdvisory` 유지 확인
- [ ] log event 기록 확인

### 23.6 Safety Stop 테스트

- [ ] S-STOP 정상 표시
- [ ] S-STOP PAUSED 표시
- [ ] S-STOP SKIPPED 표시
- [ ] 약 30초 후 SURFACE mode 전환
- [ ] DECO.STOP과 S-STOP 혼동 없음 확인

---

## 24. Git 관리

- [x] `.gitignore`에서 `.pio*` 제외
- [x] `.gitignore`에서 `logs/` 제외
- [x] `.gitignore`에서 `exports/` 제외
- [ ] 임시 빌드 로그 파일이 Git에 들어가지 않는지 확인
- [ ] `git status`로 modified 파일 확인
- [ ] commit 후 working tree clean 확인
- [ ] push 후 GitHub `dev/v1.3` 확인
- [ ] 오래된 `dev/v7.2`, `dev/v7.3` 원격 브랜치 삭제 확인
- [ ] 오래된 로컬 `dev/v7.2`, `dev/v7.3` 브랜치 삭제 확인

권장 문서 정리 커밋 메시지:

```bash
git commit -m "Clean up v1.3 docs and clarify FO2 MOD policy"
```

push:

```bash
git push origin dev/v1.3
```

---

## 25. v1.3 완료 조건

v1.3 문서 완료 조건:

- [ ] `DEVELOPMENT_v1.3.md`가 v1.3 기준으로 정리됨
- [ ] `TODO.md`가 v1.3 기준으로 정리됨
- [ ] `CHAT_CONTEXT_v1.3.md`가 최신 정책을 반영함
- [ ] `CHANGELOG.md`에 v1.3 변경점 기록됨
- [ ] Markdown 코드블록 오류 없음
- [ ] 중복 TODO 항목 없음
- [ ] GitHub `dev/v1.3`에 push 완료

v1.3 코드 완료 조건:

- [ ] `pio run -e wokwi` 성공
- [ ] Wokwi 시뮬레이션 실행 성공
- [ ] Surface 화면 정상 표시
- [ ] LAST/MAX/TMP/SURFACE/N-FLY 정상 표시
- [ ] JSON preload 정상 동작
- [ ] No-Fly epoch 계산 정상
- [ ] Surface interval epoch 계산 정상
- [ ] Battery Low 팝업 정상
- [ ] Charging UI 정상
- [ ] `config.h`에 FO2 / ppO2 설정 구조 추가
- [ ] Bühlmann 계산에서 FN2 하드코딩 제거 준비
- [ ] MOD 계산 helper 추가
- [ ] FO2 > 21%일 때 MOD 표시 구조 준비
- [ ] DECO.STOP ladder 정책 코드 반영 준비
- [ ] 감압 위반 / 재입수 상태 구조 반영 준비

---

## 26. 최종 방향 요약

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
```
```
