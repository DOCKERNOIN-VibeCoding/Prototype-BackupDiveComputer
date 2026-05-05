
# TODO - BackupDiveComputer_v7.2

---
이 문서는 앞으로 해야 할 일을 체크리스트 형태로 정리한다.
---

## 1. 현재 문서 정리

- [x] CHANGELOG.md 업데이트
- [x] CHAT_CONTEXT_v7.2.md 업데이트
- [x] DEVELOPMENT_v7.2.md 업데이트
- [x] TODO.md 업데이트
- [ ] 문서 변경사항 Git commit
- [ ] 문서 변경사항 GitHub push

---

## 2. 현재 v7.2 코드 안정화

- [x] dev/v7.2 브랜치 생성 및 사용
- [x] epoch 기반 previous dive preload 반영
- [x] `lastDiveStartEpochSec_` 구조 반영
- [x] `lastDiveDurationSec_` 구조 반영
- [x] `lastDiveEndEpochSec_` 구조 반영
- [x] `noFlyEndEpochSec_` 구조 반영
- [x] `getSurfaceIntervalSec()` 추가
- [x] `getNoFlyRemainSec()` 추가
- [x] PostDive No-Fly 계산을 `getNoFlyRemainSec()` 기준으로 정리
- [x] `src/ui.cpp` 문법 오류 수정
- [x] `include/ui.h` Surface 함수 인자 정리
- [x] `scenarios/previous_dive_surface.json` JSON 문법 정리
- [x] 불필요한 임시 코드 일부 제거
- [ ] 로컬에서 `pio run -e wokwi` 최종 빌드 확인
- [ ] Wokwi 시뮬레이터에서 Surface 화면 확인

---

## 3. Surface 화면 확인

- [x] LAST 라벨 적용
- [x] LAST 값을 마지막 다이빙 시작 날짜 기준으로 표시
- [x] MAX 표시
- [x] TMP 표시
- [x] SURFACE 표시
- [x] N-FLY 표시
- [ ] SURFACE 시간이 시뮬레이션 시간에 따라 증가하는지 확인
- [ ] N-FLY 시간이 시뮬레이션 시간에 따라 감소하는지 확인
- [ ] N-FLY 종료 시 SAFE TO FLY 표시 확인
- [ ] GPS 표시 확인
- [ ] 충전 아이콘 점멸 확인
- [ ] FULL 표시 확인

---

## 4. 시나리오 시스템

- [x] `previous_dive_surface.json` preload 구조 정리
- [x] lastDive startEpoch 사용
- [x] lastDive durationSec 사용
- [x] maxDepthM 사용
- [x] minTempC 사용
- [x] noFlyMinutesAtEnd 사용
- [x] GPS preload 사용
- [x] `generate_scenario.py`에서 preload constants 생성
- [x] `generated_scenario.h`에 epoch 기반 constants 생성
- [ ] 다른 scenario JSON 파일들도 같은 구조로 정리
- [ ] `current.json` 구조 확인
- [ ] JSON validator로 모든 scenario 확인

---

## 5. generated_scenario.h 정책

- [x] 자동 생성 파일로 유지
- [x] 직접 수정하지 않는 정책 확정
- [x] `generate_scenario.py`에서 생성
- [ ] README 또는 DEVELOPMENT 문서에 주의사항 추가 확인
- [ ] `.gitignore`에서 generated file 관리 정책 재확인

---

## 6. 실제 제품 로그 저장 준비

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

---

## 7. 로그 저장 기능

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

---

## 8. 시간 동기화와 timeSessionId

- [ ] bootCount 저장
- [ ] timeSessionId 저장
- [ ] reset reason 기록
- [ ] GPS time synced 상태 추가
- [ ] BLE time synced 상태 추가
- [ ] RTC 사용 여부 검토
- [ ] LogTimeStatus enum 추가
- [ ] TimeSynced 상태 구현
- [ ] RelativeOnly 상태 구현
- [ ] TimeCorrected 상태 구현
- [ ] SyncFailed 상태 구현
- [ ] GPS 시간이 나중에 잡혔을 때 로그 시간 보정
- [ ] timeSessionId가 다르면 보정하지 않도록 구현

---

## 9. 전원 관리

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

---

## 10. 배터리 및 충전 UI

- [x] Battery Low 팝업 정책 정의
- [x] 10% 이하 경고 정책 정의
- [x] 2분 간격 팝업 정책 정의
- [x] 2초 표시 정책 정의
- [x] 충전 중 배터리 아이콘 점멸 정책 정의
- [x] 100% 충전 시 FULL 표시 정책 정의
- [ ] 실제 화면에서 Battery Low 확인
- [ ] 실제 화면에서 Charging 확인
- [ ] 실제 화면에서 FULL 확인

---

## 11. BLE 로그 다운로드

- [ ] BLE는 충전 중에만 활성화하는 정책 유지
- [ ] BLE service 설계
- [ ] 로그 목록 characteristic 설계
- [ ] 로그 다운로드 characteristic 설계
- [ ] compact log 전송 방식 설계
- [ ] 전송 중 오류 처리 설계
- [ ] 전송 완료 확인 방식 설계
- [ ] PC/app 변환 도구와 연동 계획 수립

---

## 12. Subsurface XML

- [ ] Subsurface XML export 구조 조사
- [ ] `tools/bdc_to_subsurface_xml.py` 추가
- [ ] compact log에서 XML 생성
- [ ] dive date/time 출력
- [ ] duration 출력
- [ ] sample time/depth/temp 출력
- [ ] GPS site 출력
- [ ] cylinder/gas 기본값 출력
- [ ] Subsurface에서 import 테스트
- [ ] XML schema 호환성 확인

---

## 13. XML scenario parser

- [ ] `generate_scenario.py`에서 `.xml` 입력 감지
- [ ] Subsurface XML 읽기
- [ ] dive date/time parse
- [ ] duration parse
- [ ] sample time parse
- [ ] sample depth parse
- [ ] sample temp parse
- [ ] maxDepth 계산
- [ ] minTemp 계산
- [ ] generated_scenario.h 생성
- [ ] XML 기반 Wokwi 시뮬레이션 테스트

---

## 14. Git 관리

- [x] `.gitignore`에서 `.pio*` 제외
- [x] `.gitignore`에서 `logs/` 제외
- [x] `.gitignore`에서 `exports/` 제외
- [ ] 임시 빌드 로그 파일이 Git에 들어가지 않는지 확인
- [ ] `git status`로 M 표시 확인
- [ ] commit 후 M 표시 사라지는지 확인
- [ ] push 후 GitHub dev/v7.2 확인

---

## 15. 최종 v7.2 완료 조건

- [ ] `pio run -e wokwi` 성공
- [ ] Wokwi 시뮬레이션 실행 성공
- [ ] Surface 화면 정상 표시
- [ ] LAST/MAX/TMP/SURFACE/N-FLY 정상 표시
- [ ] JSON preload 정상 동작
- [ ] No-Fly epoch 계산 정상
- [ ] Surface interval epoch 계산 정상
- [ ] Battery Low 팝업 정상
- [ ] Charging UI 정상
- [ ] 문서 4개 최신화
- [ ] GitHub dev/v7.2에 최종 push
