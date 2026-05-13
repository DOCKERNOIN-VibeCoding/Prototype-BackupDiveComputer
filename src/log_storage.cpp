#include "log_storage.h"

#include <LittleFS.h>

LogStorage logStorage;

// 마지막 다이빙 로그를 저장할 파일 이름입니다.
// 지금은 가장 단순하게 마지막 로그 하나만 저장합니다.
static const char* LAST_DIVE_LOG_PATH = "/last_dive.bdc";

bool LogStorage::begin() {
#ifdef WOKWI_SIMULATION
    // Wokwi에서는 ESP32 partition table / LittleFS가 안정적으로 적용되지 않을 수 있습니다.
    // 그래서 시뮬레이션에서는 LittleFS를 시도하지 않고 RAM 임시 저장소를 사용합니다.
    ready_ = true;
    ramFallback_ = true;
    ramHasLastDive_ = false;

    Serial.println("[LOG] Wokwi RAM fallback enabled");
    return true;
#else
    // 실제 ESP32 하드웨어에서는 LittleFS를 사용합니다.
    if (!LittleFS.begin(true)) {
        Serial.println("[LOG] LittleFS mount failed");
        ready_ = false;
        return false;
    }

    ready_ = true;
    Serial.println("[LOG] LittleFS ready");
    return true;
#endif
}


bool LogStorage::saveLastDive(const DiveLogHeader& header) {
    if (!ready_) {
        Serial.println("[LOG] save failed: storage not ready");
        return false;
    }

#ifdef WOKWI_SIMULATION
    if (ramFallback_) {
        ramLastDive_ = header;
        ramHasLastDive_ = true;

        Serial.println("[LOG] last dive saved to RAM fallback");
        printHeader(header);
        return true;
    }
#endif


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

#ifdef WOKWI_SIMULATION
    if (ramFallback_) {
        if (!ramHasLastDive_) {
            Serial.println("[LOG] no last dive log in RAM fallback");
            return false;
        }

        header = ramLastDive_;

        Serial.println("[LOG] last dive loaded from RAM fallback");
        printHeader(header);
        return true;
    }
#endif

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

bool LogStorage::hasLastDive() {
    if (!ready_) {
        return false;
    }

#ifdef WOKWI_SIMULATION
    if (ramFallback_) {
        return ramHasLastDive_;
    }
#endif

    return LittleFS.exists(LAST_DIVE_LOG_PATH);
}


bool LogStorage::clearLastDive() {
    if (!ready_) {
        Serial.println("[LOG] clear failed: storage not ready");
        return false;
    }

#ifdef WOKWI_SIMULATION
    if (ramFallback_) {
        ramHasLastDive_ = false;
        ramLastDive_ = {};
        Serial.println("[LOG] RAM fallback last dive cleared");
        return true;
    }
#endif

    if (!LittleFS.exists(LAST_DIVE_LOG_PATH)) {
        Serial.println("[LOG] no last dive log to clear");
        return true;
    }

    if (!LittleFS.remove(LAST_DIVE_LOG_PATH)) {
        Serial.println("[LOG] clear failed: remove failed");
        return false;
    }

    Serial.println("[LOG] last dive log cleared");
    return true;
}


void LogStorage::printHeader(const DiveLogHeader& header) {
    Serial.println("[LOG] ---- DiveLogHeader ----");
    Serial.printf("[LOG] diveNumber=%u\n", header.diveNumber);
    Serial.printf("[LOG] version=%u\n", header.version);
    Serial.printf("[LOG] timeStatus=%u\n", header.timeStatus);
    Serial.printf("[LOG] timeSessionId=%lu\n", (unsigned long)header.timeSessionId);
    Serial.printf("[LOG] bootCount=%lu\n", (unsigned long)header.bootCount);
    Serial.printf("[LOG] bootElapsedStart=%lus\n", (unsigned long)header.bootElapsedStartSec);
    Serial.printf("[LOG] bootElapsedEnd=%lus\n", (unsigned long)header.bootElapsedEndSec);
    Serial.printf("[LOG] startEpoch=%lu\n", header.startEpochSec);
    Serial.printf("[LOG] endEpoch=%lu\n", header.endEpochSec);
    Serial.printf("[LOG] duration=%lus\n", header.durationSec);
    Serial.printf("[LOG] maxDepth=%.1fm\n", header.maxDepthCm / 100.0f);
    Serial.printf("[LOG] minTemp=%.1fC\n", header.minTempDeciC / 10.0f);
    Serial.printf("[LOG] sampleCount=%u\n", header.sampleCount);
    Serial.printf("[LOG] noFlyEndEpoch=%lu\n", header.noFlyEndEpochSec);
    Serial.println("[LOG] -----------------------");
}
