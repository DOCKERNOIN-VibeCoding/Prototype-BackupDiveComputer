#include "log_storage.h"

#include <LittleFS.h>

LogStorage logStorage;

// 마지막 다이빙 로그를 저장할 파일 이름입니다.
// 지금은 가장 단순하게 마지막 로그 하나만 저장합니다.
static const char* LAST_DIVE_LOG_PATH = "/last_dive.bdc";

bool LogStorage::begin() {
    // LittleFS는 ESP32 내부 flash에 파일처럼 저장할 수 있게 해주는 기능입니다.
    //
    // true:
    //   파일시스템이 없으면 포맷을 시도합니다.
    //
    // 시뮬레이션이나 초기 개발에서는 실패할 수도 있으므로,
    // 실패해도 프로그램이 멈추지는 않게 합니다.
    if (!LittleFS.begin(true)) {
        Serial.println("[LOG] LittleFS mount failed");
        ready_ = false;
        return false;
    }

    ready_ = true;
    Serial.println("[LOG] LittleFS ready");
    return true;
}

bool LogStorage::saveLastDive(const DiveLogHeader& header) {
    if (!ready_) {
        Serial.println("[LOG] save failed: storage not ready");
        return false;
    }

    File file = LittleFS.open(LAST_DIVE_LOG_PATH, "w");

    if (!file) {
        Serial.println("[LOG] save failed: cannot open file");
        return false;
    }

    size_t written = file.write((const uint8_t*)&header, sizeof(DiveLogHeader));
    file.close();

    if (written != sizeof(DiveLogHeader)) {
        Serial.println("[LOG] save failed: write size mismatch");
        return false;
    }

    Serial.println("[LOG] last dive saved");
    printHeader(header);

    return true;
}

bool LogStorage::loadLastDive(DiveLogHeader& header) {
    if (!ready_) {
        Serial.println("[LOG] load failed: storage not ready");
        return false;
    }

    if (!LittleFS.exists(LAST_DIVE_LOG_PATH)) {
        Serial.println("[LOG] no last dive log");
        return false;
    }

    File file = LittleFS.open(LAST_DIVE_LOG_PATH, "r");

    if (!file) {
        Serial.println("[LOG] load failed: cannot open file");
        return false;
    }

    size_t readSize = file.read((uint8_t*)&header, sizeof(DiveLogHeader));
    file.close();

    if (readSize != sizeof(DiveLogHeader)) {
        Serial.println("[LOG] load failed: read size mismatch");
        return false;
    }

    if (header.magic != BDC_LOG_MAGIC) {
        Serial.println("[LOG] load failed: bad magic");
        return false;
    }

    if (header.version != BDC_LOG_VERSION) {
        Serial.println("[LOG] load failed: unsupported version");
        return false;
    }

    Serial.println("[LOG] last dive loaded");
    printHeader(header);

    return true;
}

void LogStorage::printHeader(const DiveLogHeader& header) {
    Serial.println("[LOG] ---- DiveLogHeader ----");
    Serial.printf("[LOG] diveNumber=%u\n", header.diveNumber);
    Serial.printf("[LOG] startEpoch=%lu\n", header.startEpochSec);
    Serial.printf("[LOG] endEpoch=%lu\n", header.endEpochSec);
    Serial.printf("[LOG] duration=%lus\n", header.durationSec);
    Serial.printf("[LOG] maxDepth=%.1fm\n", header.maxDepthCm / 100.0f);
    Serial.printf("[LOG] minTemp=%.1fC\n", header.minTempDeciC / 10.0f);
    Serial.printf("[LOG] sampleCount=%u\n", header.sampleCount);
    Serial.printf("[LOG] noFlyEndEpoch=%lu\n", header.noFlyEndEpochSec);
    Serial.println("[LOG] -----------------------");
}
