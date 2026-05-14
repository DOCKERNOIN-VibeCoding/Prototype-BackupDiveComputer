# TODO - BackupDiveComputer v1.3

현재 개발 브랜치:

```text
dev/v1.3.6
```

현재 개발 스냅샷:

```text
dev/v1.3.6
```

이 문서는 `v1.3.5-dev` 기준으로 완료된 항목과 이후 개발해야 할 항목을 정리한다.

---

# 0. v1.3 핵심 방향

BackupDiveComputer v1.3의 핵심 방향은 다음과 같다.

```text
1. 버튼 없는 스탠드얼론 백업 다이브 컴퓨터
2. 레크리에이션 다이빙용 단일 gas 컴퓨터
3. 기본 gas는 Air / EAN21
4. 향후 Nitrox 지원을 위한 FO2 구조 준비
5. Bühlmann 계산은 FO2에서 FN2를 계산해 사용
6. ppO2 max는 1.4 bar
7. DECO.STOP은 18m / 15m / 12m / 9m / 6m / 3m ladder 사용
8. 전체 DecoPlan은 표시하지 않고 현재 필요한 DECO.STOP 하나만 표시
9. DECO.STOP 위반 후에도 hard lockout하지 않음
10. 위반 후 48시간 advisory 유지
11. 재입수 시 tissue state 기반으로 감압 계산 계속 제공
12. Safety Stop과 DECO.STOP은 명확히 분리
13. 출수 후 약 4분 이내 재입수는 연속다이빙으로 처리
14. PostDive → Surface 전환 시점을 실제 다이브 로그 종료 확정 시점으로 사용
15. GPS/BLE RTS 확보 후 RelativeOnly 로그를 bootCount 기반으로 시간 보정
16. Qi 충전 중 GPS/BLE 자동 활성화 및 분리 시 자동 OFF
```

---

# 1. 문서 정리

## 1.1 완료

- [x] `docs/DEVELOPMENT_v1.3.md`를 v1.3.5-dev 기준으로 재정리
- [x] `docs/TODO.md`를 v1.3.5-dev 기준으로 재정리
- [x] v7.x 중심 표현을 v1.x 개발 흐름으로 정리
- [x] Air / EAN21 기본 gas 정책 문서화
- [x] EAN32는 Air가 아님을 문서화
- [x] DECO.STOP ladder 정책 문서화
- [x] DECO.STOP 위반 후 hard lockout 금지 정책 문서화
- [x] 48시간 post-violation advisory 정책 문서화
- [x] 재입수 시 정보 제공 지속 정책 문서화
- [x] Safety Stop과 DECO.STOP 구분 문서화
- [x] DECO.STOP margin 정책 문서화
- [x] 최종 출수 60초 + MISSED DECO alert 30초 흐름 문서화

## 1.2 남은 작업

- [ ] `README.md`를 v1.3.5-dev 기준으로 정리
- [ ] `docs/CHAT_CONTEXT_v1.3.md`를 v1.3.5-dev 기준으로 정리
- [ ] `docs/CHANGELOG.md`에서 `Current` 중복 표현 정리
- [ ] `README.md`의 `v7.1`, `v7.2` 표현 제거
- [ ] 문서 전체에서 Markdown code block 깨짐 여부 최종 확인
- [ ] 문서 전체에서 `dev/v7.x` 잔여 표현 확인
- [ ] 문서 전체에서 `DECO_STOP_WINDOW_M` 구표현 제거 또는 historical note로 정리
- [ ] 문서 변경 후 GitHub push
- [ ] 필요 시 `v1.3.5-dev` tag를 문서 수정 commit으로 재지정

---

# 2. 빌드 / 기본 안정화

## 2.1 완료

- [x] `dev/v1.3` 브랜치 사용
- [x] `FW_VERSION`을 `v1.3.5-dev`로 설정
- [x] PlatformIO Wokwi 환경 유지
- [x] `generated_scenario.h` 자동 생성 구조 유지
- [x] `src/ui.cpp` 문법 오류 수정
- [x] `include/ui.h` Surface 함수 인자 정리
- [x] `uiDrawDecoViolationAlert()` 선언/구현 정리
- [x] `uiDrawSurface()` 인자 확장 반영

## 2.2 남은 작업

- [ ] 로컬에서 `pio run -e wokwi` 최종 빌드 확인
- [ ] Wokwi 시뮬레이터 실행 확인
- [ ] Serial command 입력 확인
- [ ] `git status` 기준 working tree clean 확인
- [ ] 빌드 로그 또는 임시 파일이 Git에 포함되지 않았는지 확인

---

# 3. Surface 화면

## 3.1 완료

- [x] `LAST` 라벨 적용
- [x] `LAST` 값을 마지막 다이빙 시작 날짜 기준으로 표시
- [x] `MAX` 표시
- [x] `TMP` 표시
- [x] `SURFACE` 표시
- [x] `N-FLY` 표시
- [x] Surface interval을 epoch 기반으로 계산
- [x] No-Fly remain을 epoch 기반으로 계산
- [x] 감압 위반 advisory 중에도 Surface 정보를 차단하지 않음
- [x] Surface 마지막 줄에서 `N-FLY` / `DECO.VIOL` 교대 표시
- [x] `DECO.VIOL` 남은 시간 표시
- [x] 충전 중 Surface 화면 유지 구조 적용

## 3.2 남은 작업

- [ ] `SURFACE` 시간이 시뮬레이션 시간에 따라 증가하는지 확인
- [ ] `N-FLY` 시간이 시뮬레이션 시간에 따라 감소하는지 확인
- [ ] `N-FLY` 종료 시 `SAFE TO FLY` 표시 확인
- [ ] `DECO.VIOL` / `N-FLY` 2초 교대 표시 확인
- [ ] GPS 표시 확인
- [ ] 충전 아이콘 점멸 확인
- [ ] `FULL` 표시 확인
- [ ] 작은 OLED에서 `DECO.VIOL` 문구 잘림 여부 확인

---

# 4. 시나리오 시스템

## 4.1 완료

- [x] `previous_dive_surface.json` preload 구조 정리
- [x] `lastDive.startEpoch` 사용
- [x] `lastDive.durationSec` 사용
- [x] `maxDepthM` 사용
- [x] `minTempC` 사용
- [x] `noFlyMinutesAtEnd` 사용
- [x] GPS preload 사용
- [x] `generate_scenario.py`에서 preload constants 생성
- [x] `generated_scenario.h` 자동 생성 유지
- [x] `generated_scenario.h` 직접 수정 금지 정책 정리

## 4.2 남은 작업

- [ ] 다른 scenario JSON 파일들도 같은 구조로 정리
- [ ] JSON validator로 모든 scenario 확인
- [ ] DECO.STOP 진입용 scenario 추가
- [ ] CEIL >18m scenario 추가
- [ ] MISSED DECO scenario 추가
- [ ] re-entry scenario 추가
- [ ] S.STOP skipped scenario 추가
- [ ] EAN32 scenario 추가
- [ ] MOD exceeded scenario 추가
- [ ] scenario JSON에서 gas 설정 override 가능하도록 설계

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

# 5. Air / Nitrox-ready 단일 gas

## 5.1 완료

- [x] 기본 gas를 Air / EAN21로 설정
- [x] `DIVE_GAS_FO2_PERCENT` 추가
- [x] `DIVE_GAS_PPO2_MAX_BAR` 추가
- [x] `DIVE_GAS_FO2_MIN_PERCENT` 추가
- [x] `DIVE_GAS_FO2_MAX_PERCENT` 추가
- [x] FO2 허용 범위 21~40% compile-time check 추가
- [x] `DIVE_GAS_FO2_PERCENT`가 산소분압이 아니라 산소비율임을 주석화
- [x] Air = EAN21 문서화
- [x] EAN32는 Air가 아님을 문서화
- [x] 현재 단계에서는 compile-time gas 설정 사용

## 5.2 남은 작업

- [ ] gas label helper 추가
- [ ] FO2 21%일 때 `AIR` 또는 `EAN21` 표시 정책 결정
- [ ] FO2 32%일 때 `EAN32` 표시 정책 결정
- [ ] 향후 앱 설정을 위한 NVS / Preferences 구조 예약
- [ ] app-configured FO2와 compile-time default 우선순위 결정
- [ ] gas 설정을 다이빙 로그에 저장
- [ ] gas 설정을 Subsurface XML export에 반영

---

# 6. Bühlmann / NDL / FN2

## 6.1 완료

- [x] Bühlmann 계산에서 FN2 하드코딩 제거
- [x] FO2 기반 FN2 계산 helper 추가
- [x] tissue nitrogen loading 계산에 FN2 적용
- [x] NDL 계산에 FN2 적용
- [x] decompression ceiling 계산에 FN2 적용
- [x] DECO.STOP duration 계산에 FN2 적용
- [x] GF99 계산에 FN2 적용
- [x] No-Fly / desaturation 계산에 gas 설정 영향 반영

## 6.2 남은 작업

- [ ] Air / EAN21 기준 테스트 작성
- [ ] EAN32 기준 테스트 작성
- [ ] 같은 profile에서 EAN21과 EAN32의 NDL 차이 검증
- [ ] 같은 profile에서 EAN21과 EAN32의 DECO.STOP 차이 검증
- [ ] 과도한 depth 입력 시 계산 안정성 검증
- [ ] 150m 등 비현실 depth에서 UI/알고리즘 fallback 검증
- [ ] Bühlmann 계산 결과를 단위 테스트로 검증할 수 있는 구조 검토

---

# 7. MOD / ppO2

## 7.1 완료

- [x] ppO2 max 기본값 1.4 bar 설정
- [x] MOD 계산 helper 추가
- [x] ppO2 계산 helper 추가
- [x] FO2 range 21~40% 구조 추가

## 7.2 남은 작업

- [ ] FO2 > 21%일 경우 다이빙 중 MOD 상시 표시
- [ ] FO2 = 21%일 경우 MOD 표시 생략 또는 낮은 우선순위 처리
- [ ] 현재 수심이 MOD를 초과하면 warning 발생
- [ ] warning 문구 결정
  - [ ] `PPO2 HIGH`
  - [ ] `MOD EXCEEDED`
  - [ ] `ASCEND`
- [ ] MOD 초과 중에도 depth/time/ascent/NDL/DECO.STOP 표시 유지
- [ ] MOD 초과 event 추가
- [ ] EAN32에서 MOD 약 33.7m 계산 확인
- [ ] EAN36에서 MOD 약 28.8m 계산 확인
- [ ] MOD 초과 warning scenario 추가

---

# 8. DECO.STOP ladder

## 8.1 완료

- [x] DECO.STOP 용어 유지
- [x] Safety Stop과 DECO.STOP 용어 분리
- [x] 고정 DECO ladder 추가
  - [x] 18m
  - [x] 15m
  - [x] 12m
  - [x] 9m
  - [x] 6m
  - [x] 3m
- [x] raw decompression ceiling 계산
- [x] raw ceiling을 ladder stop으로 매핑
- [x] raw ceiling >18m일 경우 `CEIL >18m` 경고 표시
- [x] 전체 DecoPlan 표시는 하지 않음
- [x] 현재 필요한 DECO.STOP 하나만 표시
- [x] `DECO.STOP 0m` 표시 방지
- [x] `DECO CHECK / WAIT` fallback 추가

## 8.2 남은 작업

- [ ] 현재 stop 완료 후 다음 shallower stop 자동 전환 검증
- [ ] `STOP DONE` / `NEXT 6m` UI 추가 검토
- [ ] 현재 stop 완료 전 shallower stop으로 jump하지 않도록 로직 강화
- [ ] stopStarted / stopCompleted 상태 추가 검토
- [ ] `DECO.STOP started` event 추가
- [ ] `DECO.STOP completed` event 추가
- [ ] DECO clear 후 `DECO CLEAR` / `SURFACE OK` 표시 검토

---

# 9. DECO.STOP margin / timer

## 9.1 완료

- [x] 기존 대칭 `DECO_STOP_WINDOW_M` 정책에서 비대칭 margin 정책으로 변경
- [x] shallow margin 0.6m 적용
- [x] hold margin 0.6m 적용
- [x] deep margin 1.8m 적용
- [x] 너무 얕으면 `DESCEND` 표시
- [x] HOLD 범위에서는 `HOLD` 표시
- [x] 조금 깊으면 `ASCEND` 표시하되 timer 진행 허용
- [x] 너무 깊으면 `ASCEND` 표시하고 timer pause
- [x] UI와 app timer 조건에 margin 반영

## 9.2 남은 작업

- [ ] 9m stop 기준 8.4m / 9.6m / 10.8m 동작 검증
- [ ] 6m stop 기준 5.4m / 6.6m / 7.8m 동작 검증
- [ ] 3m stop 기준 2.4m / 3.6m / 4.8m 동작 검증
- [ ] shallow violation 시 timer pause 검증
- [ ] deep margin 내 timer run 검증
- [ ] deep margin 밖 timer pause 검증
- [ ] 센서 노이즈가 timer에 미치는 영향 검토
- [ ] depth smoothing 또는 hysteresis 필요성 검토

---

# 10. DECO violation / MISSED DECO / 48h advisory

## 10.1 완료

- [x] `activeDecoViolation_` 상태 추가
- [x] `postViolationAdvisory_` 상태 추가
- [x] `postViolationAdvisoryEndEpochSec_` 추가
- [x] `reentryCount_` 추가
- [x] `clearedAfterReentry_` 추가
- [x] 감압정지 미완료 출수 감지
- [x] 미완료 출수 시 `activeDecoViolation_ = true`
- [x] 미완료 출수 시 48시간 advisory 시작
- [x] PostDive 단계에서 `MISSED DECO` alert 화면 표시
- [x] DECO violation PostDive도 일반 PostDive와 동일하게 3분 유지하도록 변경
- [x] `DECO_VIOLATION_ALERT_DISPLAY_MS`를 Surface 전환 기준에서 제거하거나 UI 강조 시간으로만 재정의
- [ ] MISSED DECO PostDive 3분 표시 Wokwi 검증
- [x] Surface 화면에 `DECO.VIOL` 남은 시간 표시
- [x] Surface 정보는 감압 위반 후에도 계속 표시
- [x] No hard lockout 정책 반영
- [x] advisory 종료 시 event 발생/Serial log 출력

## 10.2 남은 작업

- [ ] `missedStopDepthM` 추가
- [ ] `missedStopRemainSec` 추가
- [ ] `activeDecoViolation_`를 재부팅 후 복원
- [ ] `postViolationAdvisory_`를 재부팅 후 복원
- [ ] `postViolationAdvisoryEndEpochSec_`를 NVS 또는 log에 저장
- [ ] advisory 종료 event를 compact log에 영구 저장
- [ ] MISSED DECO alert 30초 표시 Wokwi 검증
- [ ] 출수 60초 + alert 30초 + Surface 전환 검증
- [ ] Surface `DECO.VIOL` 교대 표시 검증
- [ ] active violation clear 후에도 48h advisory 유지 검증
- [ ] 감압 위반 후 재입수를 권장하지 않는다는 문구 문서/README에 명시

---

# 11. Re-entry after missed DECO

## 11.1 완료

- [x] 감압 위반 상태에서 재입수 시 hard lockout하지 않음
- [x] 재입수 시 tissue state 유지
- [x] 재입수 시 DECO.STOP 재계산 계속
- [x] 재입수 시 `reentryCount_` 증가
- [x] 재입수 시 `EVENT_DECO_REENTRY` Serial log 출력
- [x] 재입수 후 DECO clear 시 `activeDecoViolation_` clear
- [x] 재입수 후 DECO clear 시 `clearedAfterReentry_ = true`
- [x] `EVENT_DECO_CLEARED_AFTER_REENTRY` Serial log 출력
- [x] DECO clear 후에도 `postViolationAdvisory_`는 유지

## 11.2 남은 작업

- [ ] re-entry scenario 작성
- [ ] re-entry 후 DECO.STOP 표시 검증
- [ ] re-entry 후 DECO clear 검증
- [ ] re-entry 후 active violation clear 검증
- [ ] re-entry 후 advisory 유지 검증
- [ ] 여러 번 re-entry 시 `reentryCount_` 동작 검증
- [ ] re-entry event를 compact log에 영구 저장
- [ ] cleared-after-reentry event를 compact log에 영구 저장

---

# 12. Safety Stop

## 12.1 완료

- [x] S-STOP과 DECO.STOP 분리
- [x] Safety Stop skipped flow before dive end 수정
- [x] Safety Stop trigger depth 10m 설정
- [x] Safety Stop zone 3m~6m 설정
- [x] Safety Stop duration 180초 설정
- [x] 얕은 쪽 이탈 30초 후 skipped 처리
- [x] 깊은 쪽 이탈 30초 후 dive resumed 처리
- [x] S.STOP SKIPPED 표시 구조 추가

## 12.2 남은 작업

- [ ] S-STOP 정상 표시 검증
- [ ] S-STOP PAUSED 표시 검증
- [ ] S-STOP SKIPPED 표시 검증
- [ ] S-STOP SKIPPED 후 약 30초 표시 검증
- [ ] S-STOP skipped 후 최종 Surface 전환 검증
- [ ] S-STOP missed를 DECO violation으로 처리하지 않는지 검증
- [ ] S-STOP 관련 event 추가 검토
- [ ] DECO.STOP이 필요한 경우 S-STOP 표시하지 않는지 검증

---

# 13. UI 우선순위

## 13.1 완료

- [x] DECO.STOP과 S-STOP 표시 구분
- [x] CEIL >18m 경고 표시
- [x] ASCEND / HOLD / DESCEND 행동 지시 표시
- [x] Unicode 화살표 대신 `u8g2.drawTriangle()` 사용
- [x] Battery Low popup 구현
- [x] Surface `DECO.VIOL` 교대 표시 구현

## 13.2 남은 작업

- [ ] UI priority manager 설계
- [ ] `PPO2 HIGH` warning 우선순위 정의
- [ ] `MOD EXCEEDED` warning 우선순위 정의
- [ ] `DECO.STOP` 우선순위 검증
- [ ] `MISSED DECO` alert 우선순위 검증
- [ ] `ASCENT FAST` 우선순위 검증
- [ ] `LOW BAT` 우선순위 검증
- [ ] 경고가 겹칠 때 순환 표시 정책 결정
- [ ] DECO.STOP 중 MOD 정보 표시 방식 결정
- [ ] FO2 > 21%일 때 MOD 상시 표시 위치 결정
- [ ] 128x64 OLED에서 글자 잘림 확인

---

# 14. 로그 포맷

## 14.1 완료

- [x] `include/log_format.h` 추가
- [x] `BDC_LOG_MAGIC` 정의
- [x] `BDC_LOG_VERSION` 정의
- [x] `LogTimeStatus` enum 정의
- [x] `DiveLogHeader` 구조 정의
- [x] `DiveSample` 구조 정의
- [x] `DiveEvent` 구조 정의
- [x] depth cm 단위 저장 결정
- [x] temp deciC 단위 저장 결정
- [x] GPS lat/lon E7 저장 결정
- [x] timeStatus 저장 필드 추가
- [x] timeSessionId 저장 필드 추가
- [x] DECO event type enum 추가
- [x] gas FO2 저장 필드 추가
- [x] ppO2 max 저장 필드 추가
- [x] deco violation flag 저장 필드 추가
- [x] post violation advisory 저장 필드 추가
- [x] advisory end epoch 저장 필드 추가
- [x] missed stop depth/remain 저장 필드 추가
- [x] reentry count 저장 필드 추가
- [x] log format version 2 적용
- [x] bootCount 저장 필드 추가
- [x] bootElapsedStartSec 저장 필드 추가
- [x] bootElapsedEndSec 저장 필드 추가
- [x] RelativeOnly 로그를 TimeCorrected로 갱신하는 구조 구현


## 14.2 남은 작업

- [ ] timeCorrectionSource 저장 필드 검토
- [ ] log format v2 실제 하드웨어 LittleFS 저장/로드 검증
- [ ] 기존 v1 로그와의 호환 또는 마이그레이션 정책 결정
- [ ] Subsurface XML export에 log format v2 필드 반영


---

# 15. 로그 저장 기능

## 15.1 완료

- [x] `include/log_storage.h` 추가
- [x] `src/log_storage.cpp` 추가
- [x] LittleFS 사용 구조 추가
- [x] Wokwi simulation RAM fallback 구조 추가
- [x] 부팅 시 마지막 로그 읽기
- [x] 마지막 로그로 Surface 화면 preload
- [x] 다이빙 종료 시 `DiveLogHeader` 저장
- [x] `log info` mock command
- [x] `log clear` mock command
- [x] GPS/BLE RTS 획득 후 pending 또는 RelativeOnly 로그 시간 보정
- [x] 보정된 로그를 LittleFS/RAM fallback에 다시 저장

## 15.2 남은 작업

- [ ] 다이빙 중 sample 영구 저장
- [ ] event 영구 저장
- [ ] eventCount 실제 반영
- [ ] sampleCount 실제 저장 sample 수와 일치 검증
- [ ] logDiveEvent()를 Serial 출력에서 storage 기록으로 확장
- [ ] MOD warning event 저장
- [ ] DECO.STOP started event 저장
- [ ] DECO.STOP completed event 저장
- [ ] MISSED DECO event 저장
- [ ] re-entry event 저장
- [ ] cleared-after-reentry event 저장
- [ ] Add `docs/LOG_FORMAT.md` defining native BDC log policy, DiveSample/DiveEvent persistence, BLE raw log transfer, and future Subsurface XML/UDDF export strategy.

---

# 16. 시간 동기화 / timeSessionId

## 16.1 완료

- [x] `LogTimeStatus` enum 구조 추가
- [x] `timeSessionId` 필드 추가
- [x] epoch 기반 Surface / No-Fly 계산 구조 추가
- [x] 48시간 advisory는 epoch 기반으로 계산
- [x] GPS fix 시 RTS 획득 처리
- [x] bootCount 저장
- [x] timeSessionId 생성 로직 구현
- [x] bootElapsedStartSec 계산 구현
- [x] bootElapsedEndSec 계산 구현
- [x] currentBootElapsedSec helper 구현
- [x] RelativeOnly 상태 실제 적용
- [x] TimeSynced 상태 실제 적용
- [x] GPS 시간이 나중에 잡혔을 때 로그 시간 보정
- [x] bootCount mismatch 시 자동 보정 금지 구현
- [x] timeSessionId mismatch 시 자동 보정 금지 구현
- [x] TimeCorrected 상태로 로그 header 갱신


## 16.2 남은 작업

- [ ] BLE app time synced 상태 추가
- [ ] BLE time sync를 RTS source로 사용하는 정책 구현
- [ ] GPS time과 BLE time의 우선순위 구현
- [ ] SyncFailed 상태 전환 조건 정의
- [ ] bootCount / timeSessionId 보정 실패 시 Serial log 외 영구 이벤트 저장
- [ ] RelativeOnly / TimeCorrected 로그 Wokwi 시나리오 검증
- [ ] 재부팅 후 TimeCorrected 로그 preload 검증


---

# 17. 배터리 / 충전 / 알람

## 17.1 완료

- [x] Battery Low threshold 10% 설정
- [x] Battery Low popup 10분 간격 설정
- [x] Battery Low popup 2초 표시 설정
- [x] Battery Low beep 1회 호출
- [x] 충전 중 배터리 아이콘 점멸
- [x] 100% 충전 시 `FULL` 표시
- [x] 빠른 상승 위험 시 3회 연속 beep
- [x] Wokwi `beep test` command 추가
- [x] Wokwi buzzer 동작 확인

## 17.2 남은 작업

- [ ] 실제 화면에서 Battery Low 표시 확인
- [ ] 실제 화면에서 Charging 표시 확인
- [ ] 실제 화면에서 FULL 표시 확인
- [ ] DECO.STOP 중 Battery Low popup 우선순위 확인
- [ ] MOD warning 중 Battery Low popup 우선순위 확인
- [ ] 실제 하드웨어 buzzer 음량 확인
- [ ] 방수 하우징 내 buzzer 전달 성능 검증

---

# 18. 전원 관리

## 18.1 남은 작업

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
- [ ] Deep Sleep 사용 시 tissue/advisory/time continuity 영향 검토
- [ ] 실제 하드웨어 전류 측정
- [x] Qi 충전 중 GPS retry-until-fix 정책 구현
- [x] Qi 충전 중 BLE advertising 유지 구현
- [x] Qi 분리 시 GPS/BLE OFF 구현
- [ ] Sleep → Qi wake → Splash → Surface → GPS 6회 시도 흐름 구현
- [x] 출수 후 GPS 30초 x 6회 시도 구현
- [ ] `BDC_DISABLE_DEEP_SLEEP` config flag 추가
- [ ] v1.3에서는 Deep Sleep 사용 금지 정책 코드 주석화
- [ ] 향후 power management 함수가 `BDC_DISABLE_DEEP_SLEEP`을 반드시 따르도록 설계
- [ ] Light Sleep만 향후 검토 대상으로 유지

---

# 19. BLE / 앱 연동

## 19.1 남은 작업

- [ ] BLE service UUID 설계
- [ ] device info characteristic 설계
- [ ] battery status characteristic 설계
- [ ] log list characteristic 설계
- [ ] log download characteristic 설계
- [ ] BLE time sync characteristic 설계
- [ ] FO2 설정 characteristic 설계
- [ ] BLE transfer active 중 power-off 지연 정책 구현
- [ ] Firmware update / OTA 가능성 검토
- [x] BLE connected 상태와 advertising 상태를 UI top bar에 반영
- [x] BLE connected / advertising 상태 Wokwi UI 검증
- [x] BLE 연결 중에는 Qi 분리 직후 OFF 지연 여부 정책 결정
- [x] Qi/Charging 해제 후 BLE 15분 access window 정책 구현


---

# 20. Subsurface XML

## 20.1 남은 작업

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
- [ ] DECO violation event export 여부 검토
- [ ] Subsurface에서 import 테스트
- [ ] XML schema 호환성 확인

---

# 21. XML scenario parser

## 21.1 남은 작업

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

# 22. 테스트 시나리오

## 22.1 기본 Surface 테스트

- [ ] 이전 다이빙 preload 후 Surface 화면 확인
- [ ] LAST 표시 확인
- [ ] MAX 표시 확인
- [ ] TMP 표시 확인
- [ ] SURFACE 증가 확인
- [ ] N-FLY 감소 확인
- [ ] SAFE TO FLY 전환 확인

## 22.2 Air / EAN21 테스트

- [ ] FO2 = 21%
- [ ] FN2 = 0.79
- [ ] MOD 표시 생략 또는 낮은 우선순위 확인
- [ ] NDL 계산 확인
- [ ] Bühlmann 계산에서 FN2 = 0.79 사용 확인

## 22.3 EAN32 테스트

- [ ] FO2 = 32%
- [ ] FN2 = 0.68
- [ ] ppO2 max = 1.4 bar
- [ ] MOD 약 33.7m 계산 확인
- [ ] 다이빙 중 MOD 상시 표시 확인
- [ ] MOD 초과 시 warning 확인
- [ ] EAN21 대비 NDL 증가 확인

## 22.4 DECO.STOP ladder 테스트

- [ ] minimal deco: 3m only
- [ ] light deco: 6m -> 3m
- [ ] moderate deco: 9m -> 6m -> 3m
- [ ] heavier deco: 12m -> 9m -> 6m -> 3m
- [ ] heavy deco: 18m -> 15m -> 12m -> 9m -> 6m -> 3m
- [ ] raw ceiling >18m warning behavior
- [ ] DECO.STOP 0m이 표시되지 않는지 확인
- [ ] stop 완료 후 다음 stop 자동 표시 확인

## 22.5 DECO.STOP margin 테스트

- [ ] 9m stop에서 8.3m → DESCEND / timer pause
- [ ] 9m stop에서 9.2m → HOLD / timer run
- [ ] 9m stop에서 10.2m → ASCEND / timer run
- [ ] 9m stop에서 11.0m → ASCEND / timer pause

## 22.6 감압 위반 / MISSED DECO 테스트

- [ ] 감압정지 미완료 출수
- [ ] 수심 <0.5m 60초 후 최종 출수 처리 확인
- [ ] `EVENT_DECO_MISSED` Serial log 확인
- [ ] `EVENT_DECO_VIOLATION_SURFACED` Serial log 확인
- [ ] `EVENT_POST_VIOLATION_ADVISORY_STARTED` Serial log 확인
- [ ] MISSED DECO alert 30초 표시 확인
- [ ] Surface 전환 후 `DECO.VIOL` 표시 확인
- [ ] 48시간 advisory 남은 시간 감소 확인

## 22.7 Re-entry 테스트

- [ ] 48시간 advisory 중 재입수
- [ ] hard lockout 없음 확인
- [ ] `EVENT_DECO_REENTRY` 확인
- [ ] DECO.STOP 계산 계속 확인
- [ ] DECO clear 후 `activeDecoViolation_` clear 확인
- [ ] `EVENT_DECO_CLEARED_AFTER_REENTRY` 확인
- [ ] `postViolationAdvisory_`는 유지되는지 확인

## 22.8 Safety Stop 테스트

- [ ] S-STOP 정상 표시
- [ ] S-STOP PAUSED 표시
- [ ] S-STOP SKIPPED 표시
- [ ] S-STOP SKIPPED 후 약 30초 표시
- [ ] S-STOP missed가 DECO.VIOL로 처리되지 않는지 확인
- [ ] DECO.STOP과 S-STOP 혼동 없음 확인

## 22.9 Continuous / repetitive dive 테스트

- [x] 수심 <0.5m 30초 후 재잠수 시 같은 Dive 유지 확인
- [x] 수심 <0.5m 60초 후 PostDive 진입 확인
- [x] PostDive 3분 이내 재잠수 시 연속다이빙 처리 확인
- [x] PostDive 중 재잠수 시 새 로그를 만들지 않는지 확인
- [ ] PostDive 중 재잠수 시 기존 로그에 샘플이 이어지는지 확인
- [x] PostDive 3분 경과 후 Surface 전환 확인
- [x] Surface 진입 이후 재잠수 시 반복다이빙으로 새 로그 생성 확인
- [x] 반복다이빙에서도 tissue loading이 이어지는지 확인

## 22.10 GPS/BLE RTS 및 로그 시간 보정 테스트

- [ ] RTS 없이 다이빙 시작 시 RelativeOnly 로그 생성 확인
- [ ] RelativeOnly 로그에 bootCount 저장 확인
- [ ] RelativeOnly 로그에 bootElapsedStartSec 저장 확인
- [ ] RelativeOnly 로그에 bootElapsedEndSec 저장 확인
- [ ] Surface에서 GPS RTS 획득 시 직전 로그 시간 보정 확인
- [ ] 보정 성공 시 timeStatus가 TimeCorrected로 변경되는지 확인
- [ ] bootCount가 다른 로그는 자동 보정하지 않는지 확인
- [ ] timeSessionId가 다른 로그는 자동 보정하지 않는지 확인
- [ ] GPS 실패 30초 x 6회 후 GPS OFF 확인
- [ ] Qi 충전 중 BLE advertising 표시 확인
- [ ] BLE connected 시 B 고정 표시 확인


# 23. Hardware Prototype

## 23.1 완료

- [x] `docs/HARDWARE_PROTOTYPE.md` 추가
- [x] `docs/HARDWARE_PROTOTYPE.json` 추가
- [x] ESP32-S3 기반 prototype 방향 정리
- [x] ST7567A LCD 후보 정리
- [x] MS5837 depth sensor 후보 정리
- [x] GPS module 후보 정리
- [x] Qi charging / protected 16340 battery 방향 정리

## 23.2 남은 작업

- [ ] ST7567A LCD pinout 실물 확인
- [ ] MS5837 I2C address 확인
- [ ] MS5837 pull-up voltage 확인
- [ ] GPS baud rate 확인
- [ ] GPS cold-start behavior 확인
- [ ] wireless charging board charge termination 확인
- [ ] power-path behavior 확인
- [ ] protected 16340 battery fit 확인
- [ ] 실제 battery capacity 확인
- [ ] GPS OFF / LCD OFF 상태 sleep current 측정
- [ ] 방수 하우징 내 버튼 없는 wake 방식 검토

---

# 24. Git 관리

## 24.1 완료

- [x] `.gitignore`에서 `.pio*` 제외
- [x] `.gitignore`에서 `logs/` 제외
- [x] `.gitignore`에서 `exports/` 제외
- [x] `dev/v1.3` 브랜치 사용
- [x] `v1.3.5-dev` tag 생성

## 24.2 남은 작업

- [ ] 임시 빌드 로그 파일이 Git에 들어가지 않는지 확인
- [ ] `git status`로 modified 파일 확인
- [ ] 문서 수정 commit
- [ ] commit 후 working tree clean 확인
- [ ] push 후 GitHub `dev/v1.3` 확인
- [ ] 필요 시 `v1.3.5-dev` tag를 최신 commit으로 이동
- [ ] 오래된 `dev/v7.2`, `dev/v7.3` 원격 브랜치 삭제 여부 결정
- [ ] 오래된 로컬 `dev/v7.2`, `dev/v7.3` 브랜치 삭제 여부 결정

문서 수정 commit 예:

```bash
git add docs/TODO.md
git commit -m "Update TODO for v1.3.5-dev"
git push origin dev/v1.3
```

필요 시 tag 재지정:

```bash
git tag -f v1.3.5-dev
git push origin -f v1.3.5-dev
```

---

# 25. v1.3.5-dev 이후 우선순위

## P0 - Continuous dive / PostDive / 로그 종료 기준 정리

- [ ] Dive 종료 후보: 수심 <0.5m 60초 유지 확인
- [ ] Dive → PostDive 전환 시 로그를 확정 종료하지 않도록 수정
- [ ] 일반 PostDive 3분 유지 확인
- [ ] DECO violation PostDive도 3분 유지하도록 수정
- [ ] PostDive → Surface 전환 시 실제 로그 종료 확정
- [ ] PostDive 중 재입수 시 연속다이빙으로 처리
- [ ] PostDive 중 재입수 시 같은 로그에 이어 기록
- [ ] Surface 진입 후 재입수 시 반복다이빙으로 새 로그 생성
- [ ] 반복다이빙에서도 tissue loading 유지

## P1 - GPS/BLE 전원 제어 및 상태 표시

- [ ] Surface/PostDive/Charging 상단바 GPS/BLE 표시 확인
- [ ] GPS searching 중 G 깜빡임 구현
- [ ] GPS valid / RTS 완료 시 G 고정 표시 구현
- [ ] GPS 실패 또는 OFF 시 - 표시 구현
- [ ] BLE advertising 중 B 깜빡임 구현
- [ ] BLE connected 시 B 고정 표시 구현
- [ ] BLE OFF 시 - 표시 구현
- [ ] Qi 충전 중 GPS retry-until-fix 구현
- [ ] Qi 충전 중 BLE advertising 유지 구현
- [ ] Qi 분리 시 GPS/BLE OFF 구현
- [ ] 출수 후 GPS 30초 x 6회 시도 구현
- [ ] Sleep/Qi wake 후 Surface 진입 및 GPS 6회 시도 구현

## P2 - RTS / 로그 시간 보정

- [ ] bootCount 영구 저장 구현
- [ ] bootElapsedStartSec / bootElapsedEndSec 로그 저장 구현
- [ ] timeSessionId 생성 로직 구현
- [ ] RTS 획득 시 bootEpochSec 계산 구현
- [ ] RelativeOnly 로그 시간 보정 구현
- [ ] 보정 성공 시 TimeCorrected로 갱신
- [ ] bootCount mismatch 시 자동 보정 금지
- [ ] timeSessionId mismatch 시 자동 보정 금지
- [ ] GPS/BLE time source 우선순위 구현
- [ ] 보정된 로그를 LittleFS에 재저장

## P3 - 로그 포맷 v2 / 샘플 / 이벤트 저장

- [ ] log format v2 필요성 결정
- [ ] DiveSample 영구 저장
- [ ] DiveEvent 영구 저장
- [ ] eventCount 반영
- [ ] sampleCount 검증
- [ ] violation/advisory 정보를 log header 또는 별도 record에 저장

## P4 - 감압 위반 정책 안정화

- [ ] DECO violation scenario 작성
- [ ] re-entry scenario 작성
- [ ] active violation clear 검증
- [ ] postViolationAdvisory 48h 유지 검증
- [ ] violation 상태 재부팅 복원 설계
- [ ] advisoryEndEpochSec 저장 구조 설계

## P5 - Nitrox / MOD UI

- [ ] FO2 >21% 표시
- [ ] MOD 표시
- [ ] MOD 초과 warning
- [ ] EAN32 scenario
- [ ] MOD exceeded scenario

## P6 - BLE / 앱 / 설정

- [ ] BLE log download 설계
- [ ] BLE time sync 설계
- [ ] FO2 앱 설정 설계
- [ ] NVS 저장 구조 설계

## P7 - 실제 하드웨어

- [ ] LCD 실물 테스트
- [ ] sensor 실물 테스트
- [ ] GPS 실물 테스트
- [ ] Qi charging 실물 테스트
- [ ] sleep current 측정
- [ ] 방수 하우징 내 표시/소리/충전 검증

---

# 26. 최종 요약

현재 `v1.3.5-dev` 기준으로 가장 중요한 남은 작업은 다음이다.

```text
1. 빌드/시뮬레이션 검증
2. DECO.STOP margin 동작 검증
3. MISSED DECO alert 30초 표시 검증
4. Surface DECO.VIOL 표시 검증
5. re-entry 후 DECO clear 및 advisory 유지 검증
6. violation/advisory 상태의 재부팅 후 복원 설계
7. DiveEvent / DiveSample 영구 저장 구현
8. MOD / ppO2 UI 구현
9. README / CHAT_CONTEXT / CHANGELOG 최신화
```
