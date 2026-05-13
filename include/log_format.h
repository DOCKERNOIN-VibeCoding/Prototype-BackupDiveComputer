#ifndef LOG_FORMAT_H
#define LOG_FORMAT_H

#include <Arduino.h>

// ------------------------------------------------------------
// BackupDiveComputer Compact Log Format v1
// ------------------------------------------------------------
//
// 이 파일은 "로그를 어떤 모양으로 저장할지" 정하는 파일입니다.
//
// 쉽게 말하면:
//   종이 로그북의 항목 이름을 미리 정해두는 것과 비슷합니다.
//
// 예:
//   다이빙 번호
//   시작 시간
//   종료 시간
//   최대 수심
//   최저 수온
//   No-Fly 종료 시간
//
// 실제 저장은 log_storage.cpp가 담당합니다.
// ------------------------------------------------------------

// 로그 파일인지 확인하기 위한 표시값입니다.
// BDC_LOG_MAGIC은 "이 파일은 BackupDiveComputer 로그다"라는 표식입니다.
static const uint32_t BDC_LOG_MAGIC = 0x4C434442; // 'B','D','C','L' 느낌의 magic 값

// 로그 포맷 버전입니다.
// 나중에 저장 구조가 바뀌면 2, 3으로 올립니다.
static const uint16_t BDC_LOG_VERSION = 2;

// 시간 상태
enum class LogTimeStatus : uint8_t {
    TimeSynced = 0,     // 다이빙 당시 실제 시간이 있었음
    RelativeOnly = 1,   // 실제 시간 없이 경과 시간만 있음
    TimeCorrected = 2,  // 나중에 GPS/BLE 시간으로 보정됨
    SyncFailed = 3      // 보정 실패
};

// 로그 헤더
//
// 지금 v7.3 첫 단계에서는 이 Header만 저장해도 충분합니다.
// 나중에 샘플 데이터와 이벤트 데이터를 뒤에 이어 붙일 수 있습니다.
struct __attribute__((packed)) DiveLogHeader {
    uint32_t magic;              // 로그 파일 확인용
    uint16_t version;            // 로그 포맷 버전
    uint16_t headerSize;         // 이 헤더의 크기

    uint16_t diveNumber;         // 몇 번째 다이빙인지
    uint8_t timeStatus;          // LogTimeStatus 값
    uint8_t reserved0;           // 나중을 위한 빈칸

    uint32_t timeSessionId;      // 시간 세션 ID

    uint32_t bootCount;          // 부팅 카운터
    uint32_t bootElapsedStartSec;// 부팅 후 다이빙 시작까지 경과 시간
    uint32_t bootElapsedEndSec;  // 부팅 후 다이빙 종료까지 경과 시간

    uint32_t startEpochSec;      // 다이빙 시작 epoch
    uint32_t endEpochSec;        // 다이빙 종료 epoch
    uint32_t durationSec;        // 다이빙 시간

    uint32_t noFlyEndEpochSec;   // No-Fly 종료 epoch

    int16_t maxDepthCm;          // 최대 수심, cm 단위
    int16_t avgDepthCm;          // 평균 수심, cm 단위, 아직 미사용이면 0
    int16_t minTempDeciC;        // 최저 수온, 0.1도 단위
    int16_t reserved1;           // 나중을 위한 빈칸

    uint16_t sampleCount;        // 샘플 개수
    uint16_t eventCount;         // 이벤트 개수

    int32_t gpsLatE7;            // GPS 위도 x 10,000,000
    int32_t gpsLonE7;            // GPS 경도 x 10,000,000
    uint8_t gpsValid;            // GPS 유효 여부
    uint8_t reserved2[3];        // 정렬용 빈칸
};

// 다이빙 중 일정 간격으로 저장할 샘플 구조입니다.
// v7.3 첫 단계에서는 구조만 만들어 둡니다.
struct __attribute__((packed)) DiveSample {
    uint16_t timeSec;            // 다이빙 시작 후 몇 초
    int16_t depthCm;             // 수심 cm
    int16_t tempDeciC;           // 수온 0.1도 단위
    uint16_t ndlOrTtsMin;        // NDL 또는 TTS
};

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

// 이벤트 구조입니다.
// 예: 안전정지 시작, 감압 진입, 빠른 상승 경고 등
struct __attribute__((packed)) DiveEvent {
    uint16_t timeSec;            // 다이빙 시작 후 몇 초
    uint8_t type;                // 이벤트 종류
    uint8_t value;               // 이벤트 보조값
};

#endif
