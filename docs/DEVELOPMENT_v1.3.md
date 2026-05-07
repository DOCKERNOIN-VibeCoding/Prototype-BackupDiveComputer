# BackupDiveComputer_v1.2 개발 정의서

---
이 문서는 BackupDiveComputer_v1.2의 개발 방향, 현재 구현 상태, 앞으로 구현할 내용을 정리한 문서이다.

코딩 경험이 없는 초심자도 이해할 수 있도록 가능한 한 쉬운 표현을 사용한다.
---


# 1. 프로젝트 개요

## 1.1 프로젝트명

```text
BackupDiveComputer
```

## 1.2 현재 개발 브랜치

```text
dev/v1.2
```

## 1.3 프로젝트 목표

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

---

# 2. 제품 컨셉

이 기기는 일반 전자기기처럼 버튼을 누르고 켜고 끄는 제품이 아니다.

제품 조건:

```text
완전 밀폐형
스탠드얼론 기기
물리 파워 버튼 없음
배터리가 방전되면 꺼짐
충전하면 다시 켜짐
Qi 충전 또는 밀폐형 충전 방식 고려
```

따라서 전원 관리가 매우 중요하다.

---

# 3. 현재 개발 환경

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

---

# 5. v1.2의 가장 중요한 변경점

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

v1.2에서는 실제 제품 방식에 맞게 epoch 기반으로 계산한다.

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

# 10. 로그 저장 방향

## 10.1 내부 저장은 compact format 사용

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

## 10.2 향후 추가할 파일

예정 파일:

```text
include/log_format.h
include/log_storage.h
src/log_storage.cpp
```

---

## 10.3 로그에 저장할 정보

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
```

---

# 11. 시간 동기화 정책

## 11.1 GPS 시간이 없을 수 있다

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

## 11.2 나중에 GPS 시간이 잡히면 보정

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

## 11.3 timeStatus

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

# 12. bootCount와 timeSessionId

## 12.1 bootCount

bootCount는 몇 번 부팅했는지를 기록하는 값이다.

용도:

```text
진단용
디버깅용
리셋 횟수 확인용
```

하지만 로그 시간 보정 기준으로는 부족하다.

---

## 12.2 timeSessionId

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

# 13. 전원 관리 정책

## 13.1 기본 방향

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

## 13.2 GPS 사용 시점

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

## 13.3 BLE 사용 시점

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

## 13.4 화면 사용 시점

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

## 13.5 Deep Sleep 사용 여부

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
```

단, 배터리가 매우 부족한 경우에는 예외적으로 Deep Sleep 또는 보호 셧다운을 사용할 수 있다.

---

# 14. 배터리 시간 예상

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

# 15. Subsurface XML 호환 방향

## 15.1 내부 저장과 외부 호환 분리

내부 저장:

```text
compact binary log
```

외부 호환:

```text
Subsurface XML
```

---

## 15.2 Export 흐름

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

## 15.3 XML scenario parser

향후에는 Subsurface XML 파일을 시뮬레이션 입력으로 사용할 수 있게 한다.

예정 기능:

```text
XML에서 dive date/time 읽기
XML에서 duration 읽기
XML에서 sample time/depth/temp 읽기
XML에서 maxDepth/minTemp 계산
generated_scenario.h 생성
```

---

# 16. PlatformIO 빌드 설정

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

# 17. 빌드 방법

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

PowerShell에서 pio 명령을 못 찾으면:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e wokwi
```

성공하면 다음과 비슷한 문구가 나온다.

```text
SUCCESS
```

---

# 18. GitHub 업로드 방법

문서를 수정한 뒤 다음 순서로 업로드한다.

```bash
git status
```

수정된 파일 확인 후:

```bash
git add docs/CHANGELOG.md docs/CHAT_CONTEXT_v1.2.md docs/DEVELOPMENT_v1.2.md docs/TODO.md
```

커밋:

```bash
git commit -m "Update v1.2 development documents"
```

푸시:

```bash
git push origin dev/v1.2
```

---

# 19. 현재 완료된 항목

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
불필요한 임시 코드 일부 정리
```

---

# 20. 앞으로 개발할 항목

```text
실제 로그 저장 구조 추가
log_format.h 추가
log_storage.h / log_storage.cpp 추가
다이빙 종료 시 내부 로그 저장
부팅 시 마지막 로그 읽기
timeSessionId 기반 시간 보정
GPS 시간 동기화 상태 처리
BLE 로그 다운로드
Subsurface XML export tool
Subsurface XML scenario parser
실제 하드웨어 전류 측정
GPS/BLE/display 전원 제어 검증
Light Sleep 전력 측정
배터리 부족 시 보호 동작 구현
```

---

# 21. v1.2 완료 기준

v1.2 1차 완료 기준:

```text
Wokwi 빌드 성공
Surface 화면 정상 표시
LAST 날짜 표시 정상
MAX 표시 정상
TMP 표시 정상
SURFACE 시간이 계속 증가
N-FLY 시간이 계속 감소
N-FLY 종료 후 SAFE TO FLY 표시
충전 표시 정상
Battery Low 팝업 정상
Serial command 정상
JSON preload 정상
generated_scenario.h 자동 생성 정상
```

v1.2 확장 완료 기준:

```text
Compact log format 초안 추가
다이빙 종료 로그 저장 초안 구현
Subsurface XML export 초안 구현
XML scenario parser 초안 구현
timeSessionId 설계 반영
저전력 정책 코드 구조 반영
```

---

# 22. 최종 방향 요약

BackupDiveComputer_v1.2는 단순히 화면을 바꾸는 버전이 아니다.

핵심은 다음이다.

```text
시뮬레이션 데이터 구조,
실제 제품 로그 구조,
Surface 화면 표시 로직,
No-Fly 계산,
Subsurface XML 호환 방향,
저전력 운용 정책

이 모든 것을 실제 제품 구조에 맞게 정리하는 단계이다.
```


## 추가 섹션: 감압 위반 후 재입수 / 48시간 정책

```md
# 감압 위반 후 재입수 및 48시간 경고 정책

## 1. 기본 철학

BackupDiveComputer는 백업용 다이브 컴퓨터이다.

따라서 메인 다이브 컴퓨터가 감압 위반으로 잠기거나 제한되더라도,
BackupDiveComputer는 수심, 시간, 상승률, 감압 정보를 계속 제공해야 한다.

이 프로젝트에서는 감압정지를 완료하지 못하고 출수한 경우에도
컴퓨터 기능을 완전히 잠그는 hard lockout은 사용하지 않는다.

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

## 2. 참고한 상업용 컴퓨터 정책

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

Suunto Nautic은 algorithm deviation 발생 후에도 lock하지 않고,
원래 decompression plan을 계속 표시한다.

또한 required decompression stops가 clear되거나 48시간이 지나면
경고 상태가 해제되는 구조를 가진다.

이 프로젝트는 이 부분을 참고한다.

---

## 3. BackupDiveComputer 최종 정책

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

## 4. 상태 구분

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

## 5. 표면 화면 동작

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

## 6. 재입수 시 동작

48시간 advisory 중에 사용자가 다시 입수한 경우,
이미 물속에 있으므로 `NO DIVE` 문구보다 실제 행동 지시가 우선이다.

재입수 시에는 다음 정보를 계속 제공한다.

```text
현재 수심
다이빙 시간
상승 속도
DECO.STOP
TTS
ceiling warning
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

## 7. 감압 완료 시 동작

재입수 후 필요한 감압정지를 모두 완료하면
현재 감압 미해결 상태는 clear한다.

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

## 8. 로그 저장 정책

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

## 9. 구현 우선순위

v1.3에서는 다음 순서로 구현한다.

```text
1. DECO.STOP 표시 상태 정리
2. stop window / ceiling violation 기준 정리
3. 감압 미완료 출수 감지
4. activeDecoViolation 상태 추가
5. postViolationAdvisory 48시간 타이머 추가
6. 재입수 시 DECO.STOP 재계산 유지
7. 감압 완료 시 activeDecoViolation clear
8. compact log event 저장
9. Surface UI에 advisory 표시
```

---

## 10. 주의

이 기능은 재입수를 권장하기 위한 기능이 아니다.

목적은 다음이다.

```text
이미 재입수한 다이버에게 필요한 정보를 계속 제공한다.
메인 컴퓨터가 잠겼을 때 백업 컴퓨터 역할을 유지한다.
감압 미해결 상태를 추적하고, 가능한 경우 계산상 clear 상태를 판단한다.
```

의학적 판단이나 재입수 여부는 다이버의 훈련, 현장 상황, 응급 절차,
DAN 또는 의료기관 지침을 따라야 한다.
```

## Automatic Sequential DECO.STOP Policy

BackupDiveComputer is a standalone automatic dive computer with no buttons.
It shall not require any manual menu navigation to view or execute decompression.

Mandatory decompression shall be guided through a fixed 3m-step ladder:

- 18m
- 15m
- 12m
- 9m
- 6m
- 3m

The device shall not show a full decompression plan table. Instead, it shall show
only the current required DECO.STOP. After the current stop is completed, the next
required stop is shown automatically.

Example:

- DECO.STOP 12m 1:00
- after completion: DECO.STOP 9m 1:00
- after completion: DECO.STOP 6m 2:00
- after completion: DECO.STOP 3m 3:00
- after completion: DECO CLEAR

The stop depths are selected from the fixed ladder, but stop durations are not fixed.
Stop durations must be calculated in real time from the current tissue nitrogen
loading and decompression model.

The first stop is determined from the current raw decompression ceiling:

- ceiling <= 3m  -> first stop 3m
- ceiling <= 6m  -> first stop 6m
- ceiling <= 9m  -> first stop 9m
- ceiling <= 12m -> first stop 12m
- ceiling <= 15m -> first stop 15m
- ceiling <= 18m -> first stop 18m

If the raw ceiling is deeper than 18m, the computer shall not incorrectly display
18m as a safe stop. It shall display a CEIL >18m warning and continue calculating
until the ceiling becomes compatible with the supported ladder.

The current stop is completed when the calculated ceiling allows ascent to the next
shallower ladder stop. For the final 3m stop, completion requires that surfacing is
allowed by the decompression model.

DECO.STOP remains the safety-critical instruction label.
Safety stop remains separate as S-STOP and shall never be mixed with DECO.STOP.

(Commercial Dive Computer Deco.Plan works reference : https://www.youtube.com/watch?v=O3rDf7O7snM , https://www.youtube.com/watch?v=3KEwlSqEQvo )


## Recreational Single-Gas Air/Nitrox Policy

BackupDiveComputer is designed as a recreational backup dive computer, not a
technical diving computer.

Supported gas model:

- Single active gas only
- Air / EAN21
- Nitrox / EANx, planned range 21% to 40% oxygen

Unsupported:

- Multi-gas switching
- Double tank gas management
- Trimix
- Helium
- CCR
- Bailout gas
- Deco gas switching

The default gas shall be Air / EAN21 for safety. EAN32 is not air; EAN32 means
32% oxygen and 68% nitrogen. If the diver is actually breathing air but the computer
calculates using EAN32, the computer may underestimate nitrogen loading.

The firmware shall be structured to support configurable FO2 even before the mobile
app is implemented. Initially, FO2 may be defined as a compile-time configuration or
scenario value. In the future, FO2 shall be configurable from the mobile app and
stored in non-volatile settings.

Gas configuration shall affect:

- tissue nitrogen loading
- NDL calculation
- DECO.STOP timing
- MOD / ppO2 warning
- dive log metadata
- Subsurface XML export

DECO.STOP ladder remains:

- 18m
- 15m
- 12m
- 9m
- 6m
- 3m
