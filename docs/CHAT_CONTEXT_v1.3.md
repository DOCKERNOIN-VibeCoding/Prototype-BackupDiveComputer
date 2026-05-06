# Chat Context - BackupDiveComputer_v1.2
---
이 문서는 새 대화에서 개발 내용을 빠르게 이어가기 위한 요약 문서이다.
---


## 프로젝트 정보

```text
Repository:
https://github.com/DOCKERNOIN-VibeCoding/Prototype-BackupDiveComputer

Branch:
dev/v1.2

Target:
BackupDiveComputer_v1.2
```

---

## 현재 개발 목표

BackupDiveComputer_v1.2의 핵심 목표는 다음과 같다.

```text
1. Surface 화면을 실제 다이브 로그 기반 구조로 변경
2. LAST / MAX / TMP / SURFACE / N-FLY 표시 구현
3. Surface interval과 No-Fly를 epoch 기반으로 계산
4. JSON 시나리오를 실제 로그와 유사한 preload 구조로 정리
5. 향후 실제 제품 로그 저장 구조와 시뮬레이션 구조를 통일
6. Subsurface XML 호환 export/import를 준비
7. 밀폐형 무버튼 기기의 저전력 운용 모델 검토
```

---

## 현재 구현된 핵심 구조

### Surface 화면 표시 항목

```text
LAST     마지막 다이빙 시작 날짜
MAX      마지막 다이빙 최대 수심
TMP      마지막 다이빙 최저 수온
SURFACE  마지막 다이빙 종료 후 경과 시간
N-FLY    비행 금지 남은 시간
```

LAST는 `LAST DIVE`가 아니라 `LAST`로 표시한다.

LAST 값은 날짜만 표시한다.

```text
YYYY-MM-DD
```

---

## epoch 기반 Surface 계산

v1.2에서는 surface interval을 시나리오에서 직접 넣지 않는다.

대신 다음 값으로 계산한다.

```text
lastDiveStartEpochSec
lastDiveDurationSec
lastDiveEndEpochSec
currentEpochSec
```

계산 방식:

```text
lastDiveEndEpochSec = lastDiveStartEpochSec + lastDiveDurationSec
surfaceIntervalSec = currentEpochSec - lastDiveEndEpochSec
```

No-Fly 계산:

```text
noFlyRemainSec = noFlyEndEpochSec - currentEpochSec
```

No-Fly가 끝난 경우:

```text
N-FLY = SAFE TO FLY
```

---

## 주요 코드 파일

```text
include/app.h
src/app.cpp
include/ui.h
src/ui.cpp
tools/generate_scenario.py
scenarios/previous_dive_surface.json
include/generated_scenario.h
platformio.ini
```

---

## generate_scenario.py 역할

`tools/generate_scenario.py`는 PC에서 실행되는 개발용 스크립트이다.

역할:

```text
JSON 시나리오 파일을 읽는다.
필요한 값을 검증한다.
C++에서 사용할 수 있는 include/generated_scenario.h를 만든다.
```

중요:

```text
generate_scenario.py는 ESP32 제품 안에서 실행되는 코드가 아니다.
실제 제품 기능 자체는 app.cpp의 로직이 담당한다.
```

현재는 시뮬레이션용이지만, 실제 제품의 로그 구조를 미리 흉내 내기 위한 중요한 개발 도구이다.

---

## generated_scenario.h 주의사항

```text
include/generated_scenario.h
```

이 파일은 자동 생성 파일이다.

직접 수정하지 않는다.

수정 순서:

```text
1. scenarios/*.json 수정
2. pio run -e wokwi 실행
3. generate_scenario.py 자동 실행
4. generated_scenario.h 자동 갱신
```

---

## 현재 시뮬레이션 시나리오

대표 시나리오:

```text
scenarios/previous_dive_surface.json
```

포함 정보:

```text
startEpoch
tzOffsetMin
preload enabled
diveCount
lastDive startEpoch
lastDive durationSec
lastDive maxDepthM
lastDive minTempC
noFlyMinutesAtEnd
GPS latitude
GPS longitude
GPS place
points[]
```

---

## 실제 제품과 시뮬레이션의 차이

현재 시뮬레이션:

```text
JSON 파일에서 이전 다이빙 정보를 읽음
JSON 파일에서 수심/수온 profile을 읽음
generated_scenario.h로 C++ 상수 생성
sim_sensor.cpp가 가짜 센서값을 제공
```

실제 제품:

```text
압력 센서에서 실제 수심을 읽음
온도 센서에서 실제 수온을 읽음
RTC/GPS/BLE에서 실제 시간을 동기화
다이빙 종료 후 로그를 내부 flash에 저장
부팅 시 이전 로그를 읽어 Surface 화면 표시
```

하지만 핵심 계산 방식은 같다.

```text
Surface interval = 현재 시각 - 마지막 다이빙 종료 시각
N-FLY remaining = No-Fly 종료 시각 - 현재 시각
```

---

## 제품 컨셉

이 기기는 다음 조건을 가진다.

```text
완전 밀폐형
스탠드얼론
물리 파워 버튼 없음
방전되면 꺼짐
충전되면 켜짐
Qi 충전 또는 접촉식 충전 고려
```

---

## 저전력 운용 방향

현재 의도한 동작:

```text
GPS는 출수 직후와 충전 중에만 사용
BLE는 충전 중에만 사용
화면은 다이빙 중, 충전 중, Qi 터치 Wake 중에만 켜짐
대기 중에는 화면 OFF, GPS OFF, BLE OFF, Light Sleep 유지
```

이 구조는 대기 전류를 낮추는 데 유리하다.

단, 실제 제품에서는 하드웨어적으로 GPS, BLE, 화면 전원이 정말 꺼지는지 확인해야 한다.

---

## Deep Sleep 정책

현재 방향:

```text
가능하면 Deep Sleep을 기본 대기 모드로 사용하지 않는다.
Light Sleep 중심으로 시간 연속성을 유지한다.
```

이유:

```text
Deep Sleep 후에는 millis()가 초기화될 수 있다.
시간 보정이 복잡해진다.
다이브 로그의 실제 시간 계산에 문제가 생길 수 있다.
```

---

## timeSessionId 개념

단순 bootCount보다 timeSessionId가 더 적합하다.

```text
bootCount:
  부팅 횟수 기록용

timeSessionId:
  시간이 이어지는 한 세션을 구분하는 값
```

GPS 시간이 없을 때 다이빙한 경우:

```text
startEpochSec = 0
timeStatus = RelativeOnly
timeSessionId 저장
startSessionMs 저장
endSessionMs 저장
```

나중에 GPS 시간이 잡혔을 때:

```text
현재 timeSessionId와 로그의 timeSessionId가 같으면 시간 보정 가능
다르면 정확한 시간 보정 불가
```

---

## 향후 로그 저장 구조

향후 실제 제품에서는 compact binary log를 저장한다.

예정 파일:

```text
include/log_format.h
include/log_storage.h
src/log_storage.cpp
```

로그에 저장할 정보:

```text
diveNumber
startEpochSec
durationSec
endEpochSec
maxDepth
minTemp
sampleCount
eventCount
noFlyEndEpochSec
GPS
timeStatus
timeSessionId
```

---

## Subsurface XML 방향

내부 저장은 XML로 하지 않는다.

```text
내부 저장:
  compact binary log

외부 호환:
  Subsurface XML export/import
```

이유:

```text
XML은 용량이 크다.
ESP32 내부 저장에 비효율적이다.
BLE 전송량이 커진다.
기기 내부 파싱 비용이 크다.
```

권장 방향:

```text
기기 내부에는 compact log 저장
BLE 또는 USB로 compact log 전송
PC/app에서 Subsurface XML로 변환
```

---

## Wokwi 빌드 명령

PlatformIO 터미널에서:

```bash
pio run -e wokwi
```

PowerShell에서 pio 명령을 못 찾는 경우:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e wokwi
```

---

## GitHub 업로드 기본 순서

```bash
git status
git add docs/CHANGELOG.md docs/CHAT_CONTEXT_v1.2.md docs/DEVELOPMENT_v1.2.md docs/TODO.md
git commit -m "Update v1.2 development documents"
git push origin dev/v1.2
```

---

## 현재까지 중요한 결론

```text
v1.2는 단순 UI 수정이 아니라,
시뮬레이션 데이터 구조와 실제 제품 로그 구조를 맞추는 단계이다.

Surface 화면은 이전 다이브 로그를 기반으로 표시해야 한다.

Surface interval과 N-FLY는 저장된 고정값이 아니라,
현재 시각과 이전 다이브 로그의 epoch 값을 이용해 계속 계산되어야 한다.

무버튼 밀폐형 제품에서는 Light Sleep 중심으로 운영하고,
GPS/BLE/화면은 필요한 순간에만 켜는 것이 적절하다.

GPS 시간이 없는 상태에서 기록된 로그는 timeSessionId를 이용해
나중에 보정 가능 여부를 판단한다.
```
```
