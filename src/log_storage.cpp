#include "log_storage.h"

#include <LittleFS.h>

LogStorage logStorage;

static const char* LAST_DIVE_LOG_PATH = "/last_dive.bdc";

static bool validateHeader(const DiveLogHeader& header) {
    if (header.magic != BDC_LOG_MAGIC) {
        Serial.println("[LOG] bad magic");
        return false;
    }

    if (header.version != BDC_LOG_VERSION) {
        Serial.printf("[LOG] unsupported version=%u expected=%u\n",
                      header.version,
                      BDC_LOG_VERSION);
        return false;
    }

    if (header.headerSize != sizeof(DiveLogHeader)) {
        Serial.printf("[LOG] header size mismatch log=%u fw=%u\n",
                      header.headerSize,
                      (unsigned)sizeof(DiveLogHeader));
        return false;
    }

    if (header.sampleCount > BDC_MAX_DIVE_SAMPLES) {
        Serial.println("[LOG] invalid sampleCount");
        return false;
    }

    if (header.eventCount > BDC_MAX_DIVE_EVENTS) {
        Serial.println("[LOG] invalid eventCount");
        return false;
    }

    return true;
}

bool LogStorage::begin() {
#ifdef WOKWI_SIMULATION
    ready_ = true;
    ramFallback_ = true;
    ramHasLastDive_ = false;
    ramLastDiveHeader_ = {};
    ramLastDiveSampleCount_ = 0;
    ramLastDiveEventCount_ = 0;

    Serial.println("[LOG] Wokwi RAM fallback enabled");
    return true;
#else
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
    return saveLastDive(header, nullptr, 0, nullptr, 0);
}

bool LogStorage::saveLastDive(const DiveLogHeader& header,
                              const DiveSample* samples,
                              uint16_t sampleCount,
                              const DiveEvent* events,
                              uint16_t eventCount) {
    if (!ready_) {
        Serial.println("[LOG] save failed: storage not ready");
        return false;
    }

    if (sampleCount > BDC_MAX_DIVE_SAMPLES) {
        sampleCount = BDC_MAX_DIVE_SAMPLES;
    }

    if (eventCount > BDC_MAX_DIVE_EVENTS) {
        eventCount = BDC_MAX_DIVE_EVENTS;
    }

    DiveLogHeader h = header;
    h.magic = BDC_LOG_MAGIC;
    h.version = BDC_LOG_VERSION;
    h.headerSize = sizeof(DiveLogHeader);
    h.sampleCount = sampleCount;
    h.eventCount = eventCount;

#ifdef WOKWI_SIMULATION
    if (ramFallback_) {
        ramLastDiveHeader_ = h;
        ramLastDiveSampleCount_ = sampleCount;
        ramLastDiveEventCount_ = eventCount;

        for (uint16_t i = 0; i < sampleCount; i++) {
            ramLastDiveSamples_[i] = samples ? samples[i] : DiveSample{};
        }

        for (uint16_t i = 0; i < eventCount; i++) {
            ramLastDiveEvents_[i] = events ? events[i] : DiveEvent{};
        }

        ramHasLastDive_ = true;

        Serial.println("[LOG] full last dive saved to RAM fallback");
        printDiveLog(ramLastDiveHeader_,
                     ramLastDiveSamples_,
                     ramLastDiveSampleCount_,
                     ramLastDiveEvents_,
                     ramLastDiveEventCount_);
        return true;
    }
#endif

    File file = LittleFS.open(LAST_DIVE_LOG_PATH, "w");

    if (!file) {
        Serial.println("[LOG] save failed: cannot open file");
        return false;
    }

    size_t written = 0;

    written += file.write((const uint8_t*)&h, sizeof(DiveLogHeader));

    if (sampleCount > 0 && samples != nullptr) {
        written += file.write((const uint8_t*)samples,
                              sizeof(DiveSample) * sampleCount);
    }

    if (eventCount > 0 && events != nullptr) {
        written += file.write((const uint8_t*)events,
                              sizeof(DiveEvent) * eventCount);
    }

    file.close();

    size_t expected =
        sizeof(DiveLogHeader) +
        sizeof(DiveSample) * sampleCount +
        sizeof(DiveEvent) * eventCount;

    if (written != expected) {
        Serial.printf("[LOG] save failed: write size mismatch written=%u expected=%u\n",
                      (unsigned)written,
                      (unsigned)expected);
        return false;
    }

    Serial.println("[LOG] full last dive saved");
    printDiveLog(h, samples, sampleCount, events, eventCount);

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

        header = ramLastDiveHeader_;

        if (!validateHeader(header)) {
            return false;
        }

        Serial.println("[LOG] last dive header loaded from RAM fallback");
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
        Serial.println("[LOG] load failed: header read size mismatch");
        return false;
    }

    if (!validateHeader(header)) {
        Serial.println("[LOG] load failed: invalid header");
        return false;
    }

    Serial.println("[LOG] last dive header loaded");
    printHeader(header);

    return true;
}

bool LogStorage::loadLastDive(DiveLogHeader& header,
                              DiveSample* samples,
                              uint16_t maxSamples,
                              uint16_t& sampleCountOut,
                              DiveEvent* events,
                              uint16_t maxEvents,
                              uint16_t& eventCountOut) {
    sampleCountOut = 0;
    eventCountOut = 0;

    if (!ready_) {
        Serial.println("[LOG] full load failed: storage not ready");
        return false;
    }

#ifdef WOKWI_SIMULATION
    if (ramFallback_) {
        if (!ramHasLastDive_) {
            Serial.println("[LOG] no last dive log in RAM fallback");
            return false;
        }

        header = ramLastDiveHeader_;

        if (!validateHeader(header)) {
            return false;
        }

        uint16_t sc = header.sampleCount;
        uint16_t ec = header.eventCount;

        if (sc > maxSamples) {
            sc = maxSamples;
        }

        if (ec > maxEvents) {
            ec = maxEvents;
        }

        for (uint16_t i = 0; i < sc; i++) {
            samples[i] = ramLastDiveSamples_[i];
        }

        for (uint16_t i = 0; i < ec; i++) {
            events[i] = ramLastDiveEvents_[i];
        }

        sampleCountOut = sc;
        eventCountOut = ec;

        Serial.println("[LOG] full last dive loaded from RAM fallback");
        printDiveLog(header, samples, sampleCountOut, events, eventCountOut);
        return true;
    }
#endif

    if (!LittleFS.exists(LAST_DIVE_LOG_PATH)) {
        Serial.println("[LOG] no last dive log");
        return false;
    }

    File file = LittleFS.open(LAST_DIVE_LOG_PATH, "r");

    if (!file) {
        Serial.println("[LOG] full load failed: cannot open file");
        return false;
    }

    size_t readSize = file.read((uint8_t*)&header, sizeof(DiveLogHeader));

    if (readSize != sizeof(DiveLogHeader)) {
        file.close();
        Serial.println("[LOG] full load failed: header read size mismatch");
        return false;
    }

    if (!validateHeader(header)) {
        file.close();
        Serial.println("[LOG] full load failed: invalid header");
        return false;
    }

    uint16_t sc = header.sampleCount;
    uint16_t ec = header.eventCount;

    if (sc > maxSamples) {
        Serial.println("[LOG] full load warning: sample buffer too small, truncating");
        sc = maxSamples;
    }

    if (ec > maxEvents) {
        Serial.println("[LOG] full load warning: event buffer too small, truncating");
        ec = maxEvents;
    }

    if (sc > 0) {
        size_t expected = sizeof(DiveSample) * sc;
        size_t got = file.read((uint8_t*)samples, expected);

        if (got != expected) {
            file.close();
            Serial.println("[LOG] full load failed: sample read size mismatch");
            return false;
        }

        // If file has more samples than caller requested, skip remaining.
        uint16_t skippedSamples = header.sampleCount - sc;
        if (skippedSamples > 0) {
            file.seek(file.position() + sizeof(DiveSample) * skippedSamples);
        }
    }

    if (ec > 0) {
        size_t expected = sizeof(DiveEvent) * ec;
        size_t got = file.read((uint8_t*)events, expected);

        if (got != expected) {
            file.close();
            Serial.println("[LOG] full load failed: event read size mismatch");
            return false;
        }
    }

    file.close();

    sampleCountOut = sc;
    eventCountOut = ec;

    Serial.println("[LOG] full last dive loaded");
    printDiveLog(header, samples, sampleCountOut, events, eventCountOut);

    return true;
}

bool LogStorage::updateLastDiveHeader(const DiveLogHeader& header) {
    if (!ready_) {
        Serial.println("[LOG] header update failed: storage not ready");
        return false;
    }

    DiveLogHeader h = header;
    h.magic = BDC_LOG_MAGIC;
    h.version = BDC_LOG_VERSION;
    h.headerSize = sizeof(DiveLogHeader);

#ifdef WOKWI_SIMULATION
    if (ramFallback_) {
        if (!ramHasLastDive_) {
            Serial.println("[LOG] header update failed: no RAM log");
            return false;
        }

        // Preserve sample/event payload counts from the new header.
        ramLastDiveHeader_ = h;

        Serial.println("[LOG] last dive header updated in RAM fallback");
        printHeader(ramLastDiveHeader_);
        return true;
    }
#endif

    if (!LittleFS.exists(LAST_DIVE_LOG_PATH)) {
        Serial.println("[LOG] header update failed: no log file");
        return false;
    }

    File file = LittleFS.open(LAST_DIVE_LOG_PATH, "r+");

    if (!file) {
        Serial.println("[LOG] header update failed: cannot open file r+");
        return false;
    }

    size_t written = file.write((const uint8_t*)&h, sizeof(DiveLogHeader));
    file.close();

    if (written != sizeof(DiveLogHeader)) {
        Serial.println("[LOG] header update failed: write size mismatch");
        return false;
    }

    Serial.println("[LOG] last dive header updated");
    printHeader(h);

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
        ramLastDiveHeader_ = {};
        ramLastDiveSampleCount_ = 0;
        ramLastDiveEventCount_ = 0;
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
    Serial.printf("[LOG] magic=0x%08lX\n", (unsigned long)header.magic);
    Serial.printf("[LOG] version=%u\n", header.version);
    Serial.printf("[LOG] headerSize=%u\n", header.headerSize);
    Serial.printf("[LOG] diveNumber=%u\n", header.diveNumber);
    Serial.printf("[LOG] timeStatus=%u\n", header.timeStatus);
    Serial.printf("[LOG] timeSessionId=%lu\n", (unsigned long)header.timeSessionId);
    Serial.printf("[LOG] bootCount=%lu\n", (unsigned long)header.bootCount);
    Serial.printf("[LOG] bootElapsedStart=%lus\n", (unsigned long)header.bootElapsedStartSec);
    Serial.printf("[LOG] bootElapsedEnd=%lus\n", (unsigned long)header.bootElapsedEndSec);
    Serial.printf("[LOG] startEpoch=%lu\n", (unsigned long)header.startEpochSec);
    Serial.printf("[LOG] endEpoch=%lu\n", (unsigned long)header.endEpochSec);
    Serial.printf("[LOG] duration=%lus\n", (unsigned long)header.durationSec);
    Serial.printf("[LOG] maxDepth=%.1fm\n", header.maxDepthCm / 100.0f);
    Serial.printf("[LOG] avgDepth=%.1fm\n", header.avgDepthCm / 100.0f);
    Serial.printf("[LOG] minTemp=%.1fC\n", header.minTempDeciC / 10.0f);
    Serial.printf("[LOG] sampleCount=%u\n", header.sampleCount);
    Serial.printf("[LOG] eventCount=%u\n", header.eventCount);
    Serial.printf("[LOG] noFlyEndEpoch=%lu\n", (unsigned long)header.noFlyEndEpochSec);
    Serial.printf("[LOG] gasFo2=%u%%\n", header.gasFo2Percent);
    Serial.printf("[LOG] ppO2Max=%.2fbar\n", header.ppO2MaxCentiBar / 100.0f);
    Serial.printf("[LOG] decoViolation=%u\n", header.decoViolation);
    Serial.printf("[LOG] postViolationAdvisory=%u\n", header.postViolationAdvisory);
    Serial.printf("[LOG] advisoryEndEpoch=%lu\n", (unsigned long)header.advisoryEndEpochSec);
    Serial.printf("[LOG] reentryCount=%u\n", header.reentryCount);
    Serial.printf("[LOG] missedStopDepth=%.1fm\n", header.missedStopDepthCm / 100.0f);
    Serial.printf("[LOG] missedStopRemain=%lus\n", (unsigned long)header.missedStopRemainSec);
    Serial.println("[LOG] -----------------------");
}

void LogStorage::printDiveLog(const DiveLogHeader& header,
                              const DiveSample* samples,
                              uint16_t sampleCount,
                              const DiveEvent* events,
                              uint16_t eventCount) {
    printHeader(header);

    Serial.println("[LOG] ---- Samples ----");

    for (uint16_t i = 0; i < sampleCount; i++) {
        const DiveSample& s = samples[i];

        // 너무 길어지는 것을 막기 위해 처음/마지막 일부만 출력
        if (i < 5 || i + 5 >= sampleCount) {
            Serial.printf("[LOG] sample[%u] t=%lus depth=%.1fm temp=%.1fC ndlOrTts=%u\n",
                          i,
                          (unsigned long)s.timeSec,
                          s.depthCm / 100.0f,
                          s.tempDeciC / 10.0f,
                          s.ndlOrTtsMin);
        } else if (i == 5) {
            Serial.println("[LOG] ...");
        }
    }

    Serial.println("[LOG] ---- Events ----");

    for (uint16_t i = 0; i < eventCount; i++) {
        const DiveEvent& e = events[i];

        Serial.printf("[LOG] event[%u] t=%lus type=%u value=%ld\n",
                      i,
                      (unsigned long)e.timeSec,
                      e.type,
                      (long)e.value);
    }

    Serial.println("[LOG] ------------------");
}
