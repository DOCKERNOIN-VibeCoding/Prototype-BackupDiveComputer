#include "app.h"
#include "config.h"
#include "sim_sensor.h"
#include "ui.h"
#include "mock_services.h"
#include "generated_scenario.h"
#include "log_storage.h"

#include <math.h>

DiveComputerApp app;

void DiveComputerApp::begin() {
    Serial.println();
    Serial.println("====================================");
    Serial.println(FW_NAME " " FW_VERSION);
    Serial.println("====================================");

    pinMode(PIN_BUZZER, OUTPUT);

    uiBegin();
    uiSplash();

    simSensor.begin();
    mockServices.begin();
    logStorage.begin();

    deco_.init(SURFACE_PRESSURE_BAR);
    deco_.setGF(DEFAULT_GF_LOW, DEFAULT_GF_HIGH);

    lastOffgasMs_ = millis();
    lastUiMs_ = 0;
    surfaceIntervalStartMs_ = millis();
    surfaceIntervalOffsetSec_ = 0;

    setState(SystemState::Surface);

    // Apply virtual previous dive log from scenario file.
    applyScenarioPreload();

    delay(SPLASH_BOOT_DISPLAY_MS);
}

void DiveComputerApp::update() {
    mockServices.update(simSensor);
    simSensor.update();

    updateGFIfNeeded();
    updateBatteryLowPopup();
    updatePostViolationAdvisory();
    updateGpsBleAutoPower();

    if (mockServices.isCharging() && state_ != SystemState::Charging) {
        setState(SystemState::Charging);
    }

    if (!mockServices.isCharging() && state_ == SystemState::Charging) {
        setState(SystemState::Surface);
    }

    switch (state_) {
        case SystemState::Surface:
            handleSurface();
            break;

        case SystemState::Dive:
            handleDive();
            break;

        case SystemState::PostDive:
            handlePostDive();
            break;

        case SystemState::Charging:
            handleCharging();
            break;
    }

    if (isBatteryLowPopupActive()) {
        uiDrawBatteryLowPopup(mockServices.getBatteryPct());
    }
}

void DiveComputerApp::startLimitedGpsSearch() {
    gpsLimitedSearchActive_ = true;
    gpsSearchAttempt_ = 1;
    gpsSearchAttemptStartedMs_ = millis();

    mockServices.setGpsSearching();

    Serial.printf("[GPS] surface search attempt %u/%u\n",
                  gpsSearchAttempt_,
                  GPS_SURFACE_MAX_ATTEMPTS);
}

void DiveComputerApp::stopLimitedGpsSearch() {
    gpsLimitedSearchActive_ = false;
    gpsSearchAttempt_ = 0;
    gpsSearchAttemptStartedMs_ = 0;
}

void DiveComputerApp::updateGpsBleAutoPower() {
    uint32_t now = millis();

    if (state_ == SystemState::Dive) {
        return;
    }

    if (state_ == SystemState::Charging) {
        if (!mockServices.isGpsValid()) {
            mockServices.setGpsSearching();
        }

        if (!mockServices.isBleConnected() &&
            !mockServices.isBleAdvertising()) {
            mockServices.setBleAdvertising();
        }

        return;
    }

    if (!gpsLimitedSearchActive_) {
        return;
    }

    if (mockServices.isGpsValid()) {
        Serial.println("[GPS] surface search complete");
        stopLimitedGpsSearch();
        return;
    }

    if (now - gpsSearchAttemptStartedMs_ < GPS_SURFACE_RETRY_INTERVAL_MS) {
        return;
    }

    if (gpsSearchAttempt_ >= GPS_SURFACE_MAX_ATTEMPTS) {
        Serial.println("[GPS] surface search failed -> GPS OFF");
        mockServices.setGpsOff();
        stopLimitedGpsSearch();
        return;
    }

    gpsSearchAttempt_++;
    gpsSearchAttemptStartedMs_ = now;

    mockServices.setGpsSearching();

    Serial.printf("[GPS] surface search attempt %u/%u\n",
                  gpsSearchAttempt_,
                  GPS_SURFACE_MAX_ATTEMPTS);
}

void DiveComputerApp::setState(SystemState newState) {
    if (state_ == newState && previousState_ == newState) {
        return;
    }

    previousState_ = state_;
    state_ = newState;

    switch (state_) {
        case SystemState::Surface:
            Serial.println("[STATE] SURFACE");

            surfaceIntervalStartMs_ = millis();

            mockServices.setBleOff();
            startLimitedGpsSearch();
            break;

        case SystemState::Dive:
            Serial.println("[STATE] DIVE");

            stopLimitedGpsSearch();
            mockServices.setGpsOff();
            mockServices.setBleOff();
            break;

        case SystemState::PostDive:
            Serial.println("[STATE] POST_DIVE");

            postDiveStartMs_ = millis();

            mockServices.setBleOff();
            startLimitedGpsSearch();
            break;

        case SystemState::Charging:
            Serial.println("[STATE] CHARGING");

            chargingEnterMs_ = millis();
            lastUiMs_ = 0;

            stopLimitedGpsSearch();
            mockServices.setGpsSearching();
            mockServices.setBleAdvertising();
            break;
    }
}

void DiveComputerApp::finalizeDiveLog() {
    if (!pendingDiveClose_) {
        return;
    }

    uint32_t now = millis();

    uint32_t durationSec = 0;

    if (dive_.diveStartMs > 0) {
        durationSec = (now - dive_.diveStartMs) / 1000UL;
    }

    lastDiveStartEpochSec_ = currentDiveStartEpochSec_;
    lastDiveDurationSec_ = durationSec;
    lastDiveEndEpochSec_ = getCurrentEpochSec();

    lastDiveMaxDepthM_ = dive_.maxDepthM;
    lastDiveMinTempC_ = dive_.minTempC;

    uint32_t noFlyMinutes = calcNoFlyMinutes();
    noFlyEndEpochSec_ = lastDiveEndEpochSec_ + noFlyMinutes * 60UL;

    DiveLogHeader header = {};

    header.magic = BDC_LOG_MAGIC;
    header.version = BDC_LOG_VERSION;
    header.headerSize = sizeof(DiveLogHeader);

    header.diveNumber = diveCount_;
    header.timeStatus = (uint8_t)LogTimeStatus::TimeSynced;
    header.timeSessionId = 0;

    header.startEpochSec = lastDiveStartEpochSec_;
    header.endEpochSec = lastDiveEndEpochSec_;
    header.durationSec = lastDiveDurationSec_;

    header.noFlyEndEpochSec = noFlyEndEpochSec_;

    header.maxDepthCm = (int16_t)(lastDiveMaxDepthM_ * 100.0f);
    header.avgDepthCm = 0;
    header.minTempDeciC = (int16_t)(lastDiveMinTempC_ * 10.0f);

    header.sampleCount = dive_.sampleCount;
    header.eventCount = 0;

    if (mockServices.isGpsValid()) {
        header.gpsValid = 1;
        header.gpsLatE7 = 0;
        header.gpsLonE7 = 0;
    } else {
        header.gpsValid = 0;
        header.gpsLatE7 = 0;
        header.gpsLonE7 = 0;
    }

    logStorage.saveLastDive(header);

    Serial.printf("[DIVE] final close #%u duration=%lus max=%.1fm samples=%u\n",
                  diveCount_,
                  durationSec,
                  dive_.maxDepthM,
                  dive_.sampleCount);

    Serial.printf("[DIVE] no-fly %luh %02lum\n",
                  noFlyMinutes / 60UL,
                  noFlyMinutes % 60UL);

    mockServices.loggerEndDive(diveCount_, durationSec, dive_.maxDepthM);

    pendingDiveClose_ = false;
}

void DiveComputerApp::updateGFIfNeeded() {
    if (!mockServices.hasNewGF()) {
        return;
    }

    deco_.setGF(mockServices.getGFLow(), mockServices.getGFHigh());
    mockServices.clearGF();
}

void DiveComputerApp::applyScenarioPreload() {
    // ------------------------------------------------------------
    // v7.3:
    // 실제 저장된 마지막 로그가 있으면 먼저 사용합니다.
    // 저장된 로그가 없을 때만 기존 JSON scenario preload를 사용합니다.
    // ------------------------------------------------------------
    DiveLogHeader savedHeader;

    if (logStorage.loadLastDive(savedHeader)) {
        diveCount_ = savedHeader.diveNumber;

        lastDiveStartEpochSec_ = savedHeader.startEpochSec;
        lastDiveDurationSec_ = savedHeader.durationSec;
        lastDiveEndEpochSec_ = savedHeader.endEpochSec;

        lastDiveMaxDepthM_ = savedHeader.maxDepthCm / 100.0f;
        lastDiveMinTempC_ = savedHeader.minTempDeciC / 10.0f;

        noFlyEndEpochSec_ = savedHeader.noFlyEndEpochSec;

        Serial.println("[LOG] Surface preload from saved compact log");
        return;
    }

    if (!SCENARIO_PRELOAD_ENABLED) {
        Serial.println("[SCENARIO] No preload log");
        return;
    }

    // ------------------------------------------------------------
    // 1. Scenario에서 이전 다이빙 정보를 읽어옵니다.
    // ------------------------------------------------------------
    diveCount_ = SCENARIO_PRELOAD_DIVE_COUNT;

    lastDiveStartEpochSec_ = SCENARIO_PRELOAD_LAST_START_EPOCH;
    lastDiveDurationSec_ = SCENARIO_PRELOAD_LAST_DURATION_SEC;
    lastDiveEndEpochSec_ = SCENARIO_PRELOAD_LAST_END_EPOCH;

    lastDiveMaxDepthM_ = SCENARIO_PRELOAD_LAST_MAX_DEPTH_M;
    lastDiveMinTempC_ = SCENARIO_PRELOAD_LAST_MIN_TEMP_C;

    noFlyEndEpochSec_ = SCENARIO_PRELOAD_NO_FLY_END_EPOCH;

    surfaceIntervalStartMs_ = millis();

    // 현재 시뮬레이션상의 실제 epoch 시간입니다.
    uint32_t nowEpoch = getCurrentEpochSec();

    // ------------------------------------------------------------
    // 2. Surface Interval 계산
    //
    // v7.2 원칙:
    // surfaceIntervalSec를 미리 저장해서 쓰지 않고,
    // 현재 시간 - 마지막 다이빙 종료 시간으로 계산합니다.
    // ------------------------------------------------------------
    if (lastDiveEndEpochSec_ > 0 && nowEpoch > lastDiveEndEpochSec_) {
        surfaceIntervalOffsetSec_ = nowEpoch - lastDiveEndEpochSec_;
    } else if (SCENARIO_PRELOAD_SURFACE_INTERVAL_SEC > 0) {
        // 구형 JSON 시나리오 호환용 fallback입니다.
        surfaceIntervalOffsetSec_ = SCENARIO_PRELOAD_SURFACE_INTERVAL_SEC;

        if (nowEpoch > surfaceIntervalOffsetSec_) {
            lastDiveEndEpochSec_ = nowEpoch - surfaceIntervalOffsetSec_;
        }

        if (lastDiveDurationSec_ > 0 &&
            lastDiveEndEpochSec_ > lastDiveDurationSec_) {
            lastDiveStartEpochSec_ = lastDiveEndEpochSec_ - lastDiveDurationSec_;
        }
    } else {
        surfaceIntervalOffsetSec_ = 0;
    }

    // ------------------------------------------------------------
    // 3. No-Fly 계산
    //
    // v7.2 원칙:
    // noFlyRemainSec를 미리 저장해서 쓰지 않고,
    // noFlyEndEpoch - 현재 epoch로 남은 시간을 계산합니다.
    // ------------------------------------------------------------
    if (noFlyEndEpochSec_ > nowEpoch) {
        noFlyEndSimSec_ = noFlyEndEpochSec_ - SCENARIO_START_EPOCH;
    } else if (SCENARIO_PRELOAD_NO_FLY_REMAIN_SEC > 0) {
        // 구형 JSON 시나리오 호환용 fallback입니다.
        noFlyEndEpochSec_ = nowEpoch + SCENARIO_PRELOAD_NO_FLY_REMAIN_SEC;
        noFlyEndSimSec_ = getSimEpochSec() + SCENARIO_PRELOAD_NO_FLY_REMAIN_SEC;
    } else {
        noFlyEndEpochSec_ = 0;
        noFlyEndSimSec_ = 0;
    }

    // ------------------------------------------------------------
    // 4. Serial 출력
    // ------------------------------------------------------------
    Serial.println("[SCENARIO] Preloaded previous dive log");
    Serial.printf("[SCENARIO] diveCount=%u\n", diveCount_);
    Serial.printf("[SCENARIO] lastStartEpoch=%lu\n", lastDiveStartEpochSec_);
    Serial.printf("[SCENARIO] lastEndEpoch=%lu\n", lastDiveEndEpochSec_);
    Serial.printf("[SCENARIO] lastDuration=%lus\n", lastDiveDurationSec_);
    Serial.printf("[SCENARIO] lastMax=%.1fm\n", lastDiveMaxDepthM_);
    Serial.printf("[SCENARIO] lastMinTemp=%.1fC\n", lastDiveMinTempC_);
    Serial.printf("[SCENARIO] surfaceInterval=%lus\n", surfaceIntervalOffsetSec_);
    Serial.printf("[SCENARIO] noFlyEndEpoch=%lu\n", noFlyEndEpochSec_);

    if (SCENARIO_PRELOAD_GPS_VALID) {
        Serial.printf("[SCENARIO] GPS %.7f, %.7f %s\n",
                      SCENARIO_PRELOAD_GPS_LAT,
                      SCENARIO_PRELOAD_GPS_LON,
                      SCENARIO_PRELOAD_GPS_PLACE);
    }
}


uint32_t DiveComputerApp::getSimEpochSec() const {
    return millis() / 1000UL;
}

uint32_t DiveComputerApp::getCurrentEpochSec() const {
    return SCENARIO_START_EPOCH + getSimEpochSec();
}


void DiveComputerApp::updateOffgassing() {
    uint32_t now = millis();

    if (now - lastOffgasMs_ < 1000UL) {
        return;
    }

    float intervalMin = (float)(now - lastOffgasMs_) / 60000.0f;
    lastOffgasMs_ = now;

    deco_.update(SURFACE_PRESSURE_BAR, intervalMin);
}

void DiveComputerApp::handleSurface() {
    updateOffgassing();

    float depth = simSensor.getDepthM();

    if (depth >= DIVE_START_DEPTH_M) {
        diveDetectCount_++;

        if (diveDetectCount_ >= DIVE_START_CONFIRM_COUNT) {
            diveDetectCount_ = 0;
            startDive();
            return;
        }
    } else {
        diveDetectCount_ = 0;
    }

    uint32_t now = millis();

    if (now - lastUiMs_ >= UI_UPDATE_INTERVAL_MS) {
        lastUiMs_ = now;
        drawSurfaceInfoScreen();
    }
}

bool DiveComputerApp::shouldShowChargingSplash(uint32_t now) const {
    if (state_ != SystemState::Charging) {
        return false;
    }

    // DECO violation advisory가 있으면 로고보다 안전 정보 우선
    if (postViolationAdvisory_ || activeDecoViolation_) {
        return false;
    }

    // Battery low popup이 활성화되어 있으면 로고 생략
    if (batteryLowPopupUntilMs_ != 0 &&
        now < batteryLowPopupUntilMs_) {
        return false;
    }

    if (chargingEnterMs_ == 0) {
        return false;
    }

    uint32_t elapsed = now - chargingEnterMs_;

    // Charging 상태 진입 직후 3초 표시
    if (elapsed < CHARGING_WAKE_SPLASH_DURATION_MS) {
        return true;
    }

    // 이후 1분마다 3초 표시
    return (elapsed % CHARGING_SPLASH_INTERVAL_MS) <
           CHARGING_SPLASH_DURATION_MS;
}

void DiveComputerApp::startDive() {
    dive_ = DiveRuntime();

    // Once a new dive starts, previous-surface preload offset is no longer used.
    surfaceIntervalOffsetSec_ = 0;

    diveCount_++;

    dive_.phase = DivePhase::Normal;
    dive_.diveStartMs = millis();
    currentDiveStartEpochSec_ = getCurrentEpochSec();

    dive_.lastDepthMs = millis();
    dive_.lastDecoUpdateMs = millis();
    dive_.lastLogMs = millis();
    dive_.lastSafetyTickMs = millis();

    dive_.depthM = simSensor.getDepthM();
    dive_.prevDepthM = dive_.depthM;
    dive_.maxDepthM = dive_.depthM;

    dive_.ascentSampleDepthM = dive_.depthM;
    dive_.lastAscentSampleMs = millis();

    dive_.safetyShallowStartMs = 0;
    dive_.safetyDeepStartMs = 0;

    dive_.tempC = simSensor.getTemperatureC();
    dive_.minTempC = dive_.tempC;

    mockServices.loggerStartDive(diveCount_);

    if (activeDecoViolation_) {
        reentryCount_++;
        clearedAfterReentry_ = false;

        Serial.printf("[DECO] Re-entry after missed DECO count=%u\n", reentryCount_);
        logDiveEvent(DiveEventType::EVENT_DECO_REENTRY, "EVENT_DECO_REENTRY");
    }


    Serial.printf("[DIVE] start #%u depth=%.1fm\n", diveCount_, dive_.depthM);
    Serial.printf("[DIVE] tissue pN2 c0=%.3f c4=%.3f c8=%.3f c15=%.3f\n",
                  deco_.getTissuePressure(0),
                  deco_.getTissuePressure(4),
                  deco_.getTissuePressure(8),
                  deco_.getTissuePressure(15));

    beep(1200, BUZZER_SHORT_BEEP_MS);

    setState(SystemState::Dive);
}

void DiveComputerApp::handleDive() {
    uint32_t now = millis();

    dive_.depthM = simSensor.getDepthM();
    dive_.tempC = simSensor.getTemperatureC();

    if (dive_.depthM > dive_.maxDepthM) {
        dive_.maxDepthM = dive_.depthM;
    }

    if (dive_.tempC < dive_.minTempC) {
        dive_.minTempC = dive_.tempC;
    }

    // ------------------------------------------------------------
    // Ascent rate calculation
    // ------------------------------------------------------------
    if (now - dive_.lastAscentSampleMs >= ASC_RATE_SAMPLE_INTERVAL_MS) {
        float dtMin = (float)(now - dive_.lastAscentSampleMs) / 60000.0f;

        if (dtMin > 0.0001f) {
            float rawRate = (dive_.ascentSampleDepthM - dive_.depthM) / dtMin;

            if (rawRate < 0.0f) {
                rawRate = 0.0f;
            }

            if (rawRate > 40.0f) {
                rawRate = 40.0f;
            }

            dive_.ascentRateMpm =
                dive_.ascentRateMpm * 0.6f +
                rawRate * 0.4f;

            if (dive_.ascentRateMpm < 0.3f) {
                dive_.ascentRateMpm = 0.0f;
            }
        }

        dive_.ascentSampleDepthM = dive_.depthM;
        dive_.lastAscentSampleMs = now;
    }

    float ambientBar = SURFACE_PRESSURE_BAR + dive_.depthM / 10.0f;

    if (now > dive_.lastDecoUpdateMs) {
        float intervalMin = (float)(now - dive_.lastDecoUpdateMs) / 60000.0f;
        dive_.lastDecoUpdateMs = now;
        deco_.update(ambientBar, intervalMin);
    }

    dive_.ndlMin = deco_.calculateNDL(ambientBar);
    dive_.ceilingDepthM = deco_.getCurrentCeilingDepthM();

// ------------------------------------------------------------
// Surface detection during dive
// ------------------------------------------------------------
//
// v1.3 safety stop skip policy:
//
// 안전정지가 필요한 상태에서 출수한 경우,
// 곧바로 endDive() 하지 않습니다.
//
// 흐름:
//   S.STOP PAUSED
//     ↓
//   일정 시간 동안 안전정지 수심으로 복귀하지 않음
//     ↓
//   S.STOP SKIPPED
//     ↓
//   30초 표시
//     ↓
//   endDive()
//
// ------------------------------------------------------------
bool unresolvedSafetyStop =
    dive_.safetyTriggered &&
    !dive_.safetyCompleted &&
    !dive_.safetySkipped;

bool skippedDisplayActive =
    dive_.safetySkipped &&
    dive_.safetySkippedAtMs > 0 &&
    (now - dive_.safetySkippedAtMs <
        SAFETY_STOP_SKIPPED_DISPLAY_SEC * 1000UL);

if (dive_.depthM < DIVE_END_DEPTH_M) {
    if (dive_.surfaceStartMs == 0) {
        dive_.surfaceStartMs = now;
        Serial.println("[DIVE] surface detection started");
    }

    if ((now - dive_.surfaceStartMs) / 1000UL >= DIVE_END_SURFACE_SEC) {
        if (unresolvedSafetyStop) {
            // 안전정지가 아직 완료/스킵 처리되지 않았으므로
            // 다이빙 종료를 보류합니다.
            // 아래 SafetyStop 로직에서 SKIPPED 판정을 하게 둡니다.
        } else if (skippedDisplayActive) {
            // S.STOP SKIPPED 표시 시간이 아직 남았으므로
            // 다이빙 종료를 조금 더 기다립니다.
        } else {
            endDive();
            return;
        }
    }
} else {
    dive_.surfaceStartMs = 0;
}


    // ------------------------------------------------------------
    // Fast ascent alarm
    // ------------------------------------------------------------
    static uint32_t lastAscentAlarmMs = 0;

    if (dive_.ascentRateMpm >= ASC_DANGER_RATE_MPM) {
        if (now - lastAscentAlarmMs >= 3000UL) {
            lastAscentAlarmMs = now;
            Serial.printf("[ALARM] Fast ascent %.1f m/min\n", dive_.ascentRateMpm);
            beepTripleWarning();
        }
    }

    // ------------------------------------------------------------
    // Dive phase logic
    // ------------------------------------------------------------
    switch (dive_.phase) {
        case DivePhase::Normal: {
            if (dive_.ndlMin == 0) {
                DecoInfo info = deco_.calculateDeco(ambientBar, 1.0f);

                bool decoRequired =
                    info.ceiling_depth_m > 0.5f ||
                    info.stop_depth_m > 0 ||
                    info.ceiling_gt_max_stop;

                if (decoRequired) {
                    dive_.phase = DivePhase::Deco;
                    dive_.decoEntered = true;

                    dive_.decoStopDepthM = info.stop_depth_m;
                    dive_.decoStopTimeMin = info.stop_time_min;
                    dive_.decoTtsMin = info.tts_min;
                    dive_.ceilingDepthM = info.ceiling_depth_m;
                    dive_.decoCeilingGtMaxStop = info.ceiling_gt_max_stop;

                    Serial.println("[DIVE] Enter DECO");
                    logDiveEvent(DiveEventType::EVENT_DECO_REQUIRED,
                                "EVENT_DECO_REQUIRED");

                    beep(ALARM_FREQ_DECO_ENTER, 150);
                    delay(100);
                    beep(ALARM_FREQ_DECO_ENTER, 150);
                    delay(100);
                    beep(ALARM_FREQ_DECO_ENTER, 150);

                    break;
                }
            }


            if (!dive_.safetyTriggered &&
                !dive_.safetyCompleted &&
                !dive_.safetySkipped &&
                dive_.maxDepthM >= SAFETY_STOP_TRIGGER_DEPTH_M &&
                dive_.depthM >= SAFETY_STOP_MIN_DEPTH_M &&
                dive_.depthM <= SAFETY_STOP_MAX_DEPTH_M) {

                dive_.safetyTriggered = true;
                dive_.safetyPaused = false;
                dive_.safetyElapsedMs = 0;
                dive_.safetyPauseStartMs = 0;
                dive_.lastSafetyTickMs = now;
                dive_.phase = DivePhase::SafetyStop;

                Serial.println("[DIVE] Enter SAFETY STOP");
                break;
            }

            break;
        }

        case DivePhase::SafetyStop: {
            bool inRange =
                dive_.depthM >= SAFETY_STOP_MIN_DEPTH_M &&
                dive_.depthM <= SAFETY_STOP_MAX_DEPTH_M;

            bool shallowOut =
                dive_.depthM < SAFETY_STOP_MIN_DEPTH_M;

            bool deepOut =
                dive_.depthM > SAFETY_STOP_MAX_DEPTH_M;

            if (inRange) {
                dive_.safetyShallowStartMs = 0;
                dive_.safetyDeepStartMs = 0;

                if (dive_.safetyPaused) {
                    dive_.safetyPaused = false;
                    Serial.println("[DIVE] Safety stop RESUMED");
                }

                if (now > dive_.lastSafetyTickMs) {
                    dive_.safetyElapsedMs += now - dive_.lastSafetyTickMs;
                }

                dive_.lastSafetyTickMs = now;

                if (dive_.safetyElapsedMs / 1000UL >= SAFETY_STOP_DURATION_S) {
                    dive_.safetyCompleted = true;
                    dive_.safetyPaused = false;
                    dive_.phase = DivePhase::Normal;

                    Serial.println("[DIVE] Safety stop COMPLETE");
                    break;
                }
            } else {
                if (!dive_.safetyPaused) {
                    dive_.safetyPaused = true;
                    Serial.printf("[DIVE] Safety stop PAUSED at %.1fm\n", dive_.depthM);
                }

                dive_.lastSafetyTickMs = now;

                if (shallowOut) {
                    dive_.safetyDeepStartMs = 0;

                    if (dive_.safetyShallowStartMs == 0) {
                        dive_.safetyShallowStartMs = now;
                    }

                    if ((now - dive_.safetyShallowStartMs) / 1000UL >= SAFETY_STOP_REEVAL_SEC) {
                        dive_.safetySkipped = true;
                        dive_.safetyPaused = false;
                        dive_.safetySkippedAtMs = now;
                        dive_.phase = DivePhase::Normal;

                        Serial.println("[DIVE] Safety stop SKIPPED: shallow for 30s");
                        beep(700, 250);
                        break;
                    }
                } else if (deepOut) {
                    dive_.safetyShallowStartMs = 0;

                    if (dive_.safetyDeepStartMs == 0) {
                        dive_.safetyDeepStartMs = now;
                    }

                    if ((now - dive_.safetyDeepStartMs) / 1000UL >= SAFETY_STOP_REEVAL_SEC) {
                        dive_.safetyTriggered = false;
                        dive_.safetyPaused = false;
                        dive_.safetyElapsedMs = 0;
                        dive_.safetyShallowStartMs = 0;
                        dive_.safetyDeepStartMs = 0;
                        dive_.phase = DivePhase::Normal;

                        Serial.println("[DIVE] Safety stop cancelled: dive resumed deeper than 6m for 30s");
                        break;
                    }
                } else {
                    dive_.safetyShallowStartMs = 0;
                    dive_.safetyDeepStartMs = 0;
                }
            }

            break;
        }

        case DivePhase::Deco: {
            bool wasCeilingGtMaxStop = dive_.decoCeilingGtMaxStop;

            DecoInfo info = deco_.calculateDeco(ambientBar, 1.0f);

            dive_.decoStopDepthM = info.stop_depth_m;
            dive_.decoStopTimeMin = info.stop_time_min;
            dive_.decoTtsMin = info.tts_min;
            dive_.ceilingDepthM = info.ceiling_depth_m;
            dive_.decoCeilingGtMaxStop = info.ceiling_gt_max_stop;
            if (!dive_.decoCeilingGtMaxStop &&
                dive_.ceilingDepthM <= 0.5f &&
                dive_.decoStopDepthM == 0) {

                dive_.phase = DivePhase::Normal;

                dive_.decoStopDepthM = 0;
                dive_.lastDecoStopDepthM = 0;
                dive_.decoStopTimeMin = 0;
                dive_.decoStopRemainSec = 0;
                dive_.decoTtsMin = 0;

                Serial.println("[DIVE] DECO cleared: no active ceiling");
                break;
            }

            if (dive_.decoCeilingGtMaxStop && !wasCeilingGtMaxStop) {
            logDiveEvent(DiveEventType::EVENT_CEIL_GT_18M, "EVENT_CEIL_GT_18M");
            }

            if (dive_.decoCeilingGtMaxStop) {
                dive_.decoStopDepthM = 0;
                dive_.decoStopTimeMin = 0;
                dive_.decoStopRemainSec = 0;
                dive_.lastDecoStopTickMs = now;
            }

            if (!dive_.decoCeilingGtMaxStop) {

            if (dive_.lastDecoStopDepthM != dive_.decoStopDepthM) {
                dive_.lastDecoStopDepthM = dive_.decoStopDepthM;
                dive_.decoStopRemainSec = (uint32_t)dive_.decoStopTimeMin * 60UL;
                dive_.lastDecoStopTickMs = now;
            }

            if (dive_.decoStopRemainSec == 0 && dive_.decoStopTimeMin > 0) {
                dive_.decoStopRemainSec = (uint32_t)dive_.decoStopTimeMin * 60UL;
                dive_.lastDecoStopTickMs = now;
            }

            uint32_t modelRemainSec = (uint32_t)dive_.decoStopTimeMin * 60UL;

            if (modelRemainSec > 0 && modelRemainSec < dive_.decoStopRemainSec) {
                dive_.decoStopRemainSec = modelRemainSec;
            }

            bool atDecoStop =
                dive_.decoStopDepthM > 0 &&
                dive_.depthM <= (float)dive_.decoStopDepthM + DECO_STOP_DEEP_MARGIN_M &&
                dive_.depthM >= (float)dive_.decoStopDepthM - DECO_STOP_SHALLOW_MARGIN_M;

            if (atDecoStop) {
                if (now > dive_.lastDecoStopTickMs) {
                    uint32_t deltaSec =
                        (now - dive_.lastDecoStopTickMs) / 1000UL;

                    if (deltaSec > 0) {
                        if (deltaSec >= dive_.decoStopRemainSec) {
                            dive_.decoStopRemainSec = 0;
                        } else {
                            dive_.decoStopRemainSec -= deltaSec;
                        }

                        dive_.lastDecoStopTickMs += deltaSec * 1000UL;
                    }
                }
            } else {
                dive_.lastDecoStopTickMs = now;
            }
        }

            static uint32_t lastCeilingAlarmMs = 0;

                if (dive_.ceilingDepthM > 0.5f &&
                    dive_.depthM < dive_.ceilingDepthM - 0.5f) {

                    if (!dive_.decoViolation) {
                        logDiveEvent(DiveEventType::EVENT_CEILING_EXCEEDED, "EVENT_CEILING_EXCEEDED");
                    }

                    dive_.decoViolation = true;

                if (now - lastCeilingAlarmMs >= 2000UL) {
                    lastCeilingAlarmMs = now;

                    Serial.printf("[ALARM] Ceiling violation depth=%.1f ceiling=%.1f\n",
                                  dive_.depthM,
                                  dive_.ceilingDepthM);

                    beep(ALARM_FREQ_CEILING_VIOL, 200);
                }
            }

            if (dive_.ceilingDepthM <= 0.0f && dive_.ndlMin > 0) {
                dive_.phase = DivePhase::Normal;

                dive_.decoStopDepthM = 0;
                dive_.lastDecoStopDepthM = 0;
                dive_.decoStopTimeMin = 0;
                dive_.decoStopRemainSec = 0;
                dive_.decoTtsMin = 0;

                Serial.println("[DIVE] DECO cleared");
                    if (activeDecoViolation_) {
                        activeDecoViolation_ = false;
                        clearedAfterReentry_ = true;

                        Serial.println("[DECO] Active violation cleared after re-entry");
                        logDiveEvent(DiveEventType::EVENT_DECO_CLEARED_AFTER_REENTRY,
                                    "EVENT_DECO_CLEARED_AFTER_REENTRY");
                    } else {
                        logDiveEvent(DiveEventType::EVENT_DECO_CLEARED,
                                    "EVENT_DECO_CLEARED");
                    }


                beep(ALARM_FREQ_DECO_ENTER, 150);
                delay(100);
                beep(ALARM_FREQ_DECO_ENTER, 150);

                break;
            }

            break;
        }
    }

    // ------------------------------------------------------------
    // Dive log sample
    // ------------------------------------------------------------
    if (now - dive_.lastLogMs >= LOG_RECORD_INTERVAL_S * 1000UL) {
        dive_.lastLogMs = now;
        dive_.sampleCount++;

        uint16_t ndlOrTts = dive_.ndlMin;

        if (dive_.phase == DivePhase::Deco) {
            ndlOrTts = dive_.decoTtsMin;
        }

        mockServices.loggerSample(dive_.sampleCount,
                                  dive_.depthM,
                                  dive_.tempC,
                                  ndlOrTts);
    }

    // ------------------------------------------------------------
    // UI
    // ------------------------------------------------------------
    if (now - lastUiMs_ >= UI_UPDATE_INTERVAL_MS) {
        lastUiMs_ = now;

        uint32_t diveTimeSec = (now - dive_.diveStartMs) / 1000UL;

        switch (dive_.phase) {
            case DivePhase::Normal: {
                uint16_t ttsMin = calcNormalTTSMin();

                if (dive_.safetySkipped) {
                    uiDrawDiveSkipped(dive_.depthM,
                                      ttsMin,
                                      diveTimeSec,
                                      dive_.maxDepthM,
                                      dive_.tempC,
                                      dive_.ascentRateMpm,
                                      mockServices.getBatteryPct());
                } else {
                    uiDrawDiveNormal(dive_.depthM,
                                     dive_.ndlMin,
                                     ttsMin,
                                     diveTimeSec,
                                     dive_.maxDepthM,
                                     dive_.tempC,
                                     dive_.ascentRateMpm,
                                     mockServices.getBatteryPct());
                }

                break;
            }

            case DivePhase::SafetyStop: {
                uint32_t elapsedSec = dive_.safetyElapsedMs / 1000UL;
                uint32_t remainSec = 0;

                if (elapsedSec < SAFETY_STOP_DURATION_S) {
                    remainSec = SAFETY_STOP_DURATION_S - elapsedSec;
                }

                uint16_t ttsMin = calcSafetyTTSMin(remainSec);

                uiDrawDiveSafety(dive_.depthM,
                                  remainSec,
                                  dive_.safetyPaused,
                                  ttsMin,
                                  diveTimeSec,
                                  dive_.maxDepthM,
                                  dive_.tempC,
                                  dive_.ascentRateMpm,
                                  mockServices.getBatteryPct());
                break;
            }

            case DivePhase::Deco:
                uiDrawDiveDeco(dive_.depthM,
                                dive_.decoStopDepthM,
                                dive_.decoStopRemainSec,
                                dive_.decoTtsMin,
                                diveTimeSec,
                                dive_.maxDepthM,
                                dive_.tempC,
                                dive_.ascentRateMpm,
                                mockServices.getBatteryPct(),
                                dive_.decoCeilingGtMaxStop,
                                dive_.ceilingDepthM);

                break;
        }
    }
}
void DiveComputerApp::resumeContinuousDive() {
    uint32_t now = millis();
    float depth = simSensor.getDepthM();

    Serial.printf("[POST] continuous dive resumed depth=%.1fm\n", depth);

    pendingDiveClose_ = false;
    postDiveStartMs_ = 0;

    dive_.surfaceStartMs = 0;
    dive_.depthM = depth;
    dive_.prevDepthM = depth;
    dive_.ascentSampleDepthM = depth;

    dive_.lastDepthMs = now;
    dive_.lastDecoUpdateMs = now;
    dive_.lastAscentSampleMs = now;
    dive_.lastLogMs = now;

    if (activeDecoViolation_) {
        reentryCount_++;
        clearedAfterReentry_ = false;

        Serial.printf("[DECO] Continuous re-entry after missed DECO count=%u\n",
                      reentryCount_);

        logDiveEvent(DiveEventType::EVENT_DECO_REENTRY,
                     "EVENT_DECO_REENTRY");
    }

    setState(SystemState::Dive);
}

void DiveComputerApp::endDive() {
    uint32_t now = millis();

    if (dive_.decoEntered && dive_.ceilingDepthM > 0.5f) {
        dive_.decoViolation = true;
        activeDecoViolation_ = true;
        clearedAfterReentry_ = false;

        Serial.println("[DIVE] DECO violation: surfaced with active ceiling");

        beep(ALARM_FREQ_CEILING_VIOL, 150);
        delay(100);
        beep(ALARM_FREQ_CEILING_VIOL, 150);
        delay(100);
        beep(ALARM_FREQ_CEILING_VIOL, 150);

        logDiveEvent(DiveEventType::EVENT_DECO_MISSED,
                     "EVENT_DECO_MISSED");

        logDiveEvent(DiveEventType::EVENT_DECO_VIOLATION_SURFACED,
                     "EVENT_DECO_VIOLATION_SURFACED");

        bool advisoryWasActive = postViolationAdvisory_;

        postViolationAdvisory_ = true;

        uint32_t newAdvisoryEnd =
            getCurrentEpochSec() + POST_VIOLATION_ADVISORY_HOURS * 3600UL;

        if (newAdvisoryEnd > postViolationAdvisoryEndEpochSec_) {
            postViolationAdvisoryEndEpochSec_ = newAdvisoryEnd;
        }

        if (!advisoryWasActive) {
            logDiveEvent(DiveEventType::EVENT_POST_VIOLATION_ADVISORY_STARTED,
                         "EVENT_POST_VIOLATION_ADVISORY_STARTED");
        }

        Serial.printf("[DECO] Post violation advisory until epoch=%lu\n",
                      postViolationAdvisoryEndEpochSec_);
    }

    uint32_t durationSec = 0;

    if (dive_.diveStartMs > 0) {
        durationSec = (now - dive_.diveStartMs) / 1000UL;
    }

    lastDiveStartEpochSec_ = currentDiveStartEpochSec_;
    lastDiveDurationSec_ = durationSec;
    lastDiveEndEpochSec_ = getCurrentEpochSec();

    lastDiveMaxDepthM_ = dive_.maxDepthM;
    lastDiveMinTempC_ = dive_.minTempC;

    uint32_t noFlyMinutes = calcNoFlyMinutes();
    noFlyEndEpochSec_ = lastDiveEndEpochSec_ + noFlyMinutes * 60UL;

    pendingDiveClose_ = true;

    Serial.printf("[DIVE] tentative end #%u duration=%lus max=%.1fm samples=%u\n",
                  diveCount_,
                  durationSec,
                  dive_.maxDepthM,
                  dive_.sampleCount);

    Serial.printf("[DIVE] no-fly %luh %02lum\n",
                  noFlyMinutes / 60UL,
                  noFlyMinutes % 60UL);

    Serial.printf("[DIVE] end tissue pN2 c0=%.3f c4=%.3f c8=%.3f c15=%.3f\n",
                  deco_.getTissuePressure(0),
                  deco_.getTissuePressure(4),
                  deco_.getTissuePressure(8),
                  deco_.getTissuePressure(15));
    
    postDiveStartMs_ = millis();
    lastOffgasMs_ = millis();

    setState(SystemState::PostDive);
}

uint32_t DiveComputerApp::calcNoFlyMinutes() const {
    if (dive_.decoViolation) {
        Serial.println("[NOFLY] DECO violation -> fixed 48h");
        return NO_FLY_DECO_VIOLATION_HOURS * 60UL;
    }

    uint32_t dynamicMinutes =
        deco_.calculateNoFlyMinutes(NO_FLY_CABIN_PRESSURE_BAR);

    uint32_t minMinutes = NO_FLY_MIN_HOURS * 60UL;
    uint32_t result = dynamicMinutes;

    if (result < minMinutes) {
        result = minMinutes;
    }

    uint32_t maxMinutes = NO_FLY_MAX_HOURS * 60UL;

    if (result > maxMinutes) {
        result = maxMinutes;
    }

    Serial.printf("[NOFLY] dynamic=%luh %02lum, final=%luh %02lum\n",
                  dynamicMinutes / 60UL,
                  dynamicMinutes % 60UL,
                  result / 60UL,
                  result % 60UL);

    return result;
}

void DiveComputerApp::handlePostDive() {
    updateOffgassing();

    uint32_t now = millis();

    float depth = simSensor.getDepthM();

    if (depth >= DIVE_START_DEPTH_M) {
        diveDetectCount_++;

        if (diveDetectCount_ >= DIVE_START_CONFIRM_COUNT) {
            diveDetectCount_ = 0;

            Serial.printf("[POST] Re-dive detected depth=%.1fm\n", depth);

            resumeContinuousDive();
            return;
        }
    } else {
        diveDetectCount_ = 0;
    }

    if (now - lastUiMs_ >= UI_UPDATE_INTERVAL_MS) {
        lastUiMs_ = now;

        uint32_t remain = getNoFlyRemainSec();

        if (postViolationAdvisory_) {
            uint32_t advisoryRemainSec = 0;
            uint32_t nowEpoch = getCurrentEpochSec();

            if (postViolationAdvisoryEndEpochSec_ > nowEpoch) {
                advisoryRemainSec = postViolationAdvisoryEndEpochSec_ - nowEpoch;
            }

            uiDrawDecoViolationAlert(getCurrentEpochSec(),
                                      SCENARIO_TZ_OFFSET_MIN,
                                      mockServices.getBatteryPct(),
                                      mockServices.isGpsValid(),
                                      mockServices.isGpsSearching(),
                                      mockServices.isBleConnected(),
                                      mockServices.isBleAdvertising(),
                                      mockServices.isCharging(),
                                      false,
                                      advisoryRemainSec,
                                      activeDecoViolation_);

        } else {
            uiDrawPostDive(diveCount_,
                           lastDiveDurationSec_,
                           lastDiveMaxDepthM_,
                           dive_.minTempC,
                           remain,
                           mockServices.isGpsValid(),
                           mockServices.getBatteryPct());
        }

    }

    if (now - postDiveStartMs_ >= POST_DIVE_DISPLAY_MS) {
        Serial.println("[POST] timeout -> final close -> SURFACE");

        finalizeDiveLog();

        postDiveStartMs_ = 0;
        surfaceIntervalStartMs_ = millis();
        surfaceIntervalOffsetSec_ = 0;

        setState(SystemState::Surface);
    }
}

void DiveComputerApp::handleCharging() {
    updateOffgassing();

    uint32_t now = millis();

    if (now - lastUiMs_ >= UI_UPDATE_INTERVAL_MS) {
        lastUiMs_ = now;

        if (shouldShowChargingSplash(now)) {
            uiDrawSplashLogo();
        } else {
            drawSurfaceInfoScreen();
        }
    }
}


uint16_t DiveComputerApp::calcNormalTTSMin() const {
    float depth = dive_.depthM;

    if (depth <= 0.5f) {
        return 0;
    }

    float tts = 0.0f;

    if (dive_.maxDepthM >= SAFETY_STOP_TRIGGER_DEPTH_M &&
        !dive_.safetyCompleted &&
        !dive_.safetySkipped) {

        if (depth > 5.0f) {
            tts = ((depth - 5.0f) / 10.0f) + 3.0f + 0.5f;
        } else {
            tts = 3.0f + (depth / 10.0f);
        }
    } else {
        tts = depth / 10.0f;
    }

    uint16_t result = (uint16_t)ceilf(tts);

    if (result > 99) {
        result = 99;
    }

    return result;
}

uint16_t DiveComputerApp::calcSafetyTTSMin(uint32_t safetyRemainSec) const {
    float ascentMin = dive_.depthM / 10.0f;
    float safetyMin = (float)safetyRemainSec / 60.0f;

    uint16_t result = (uint16_t)ceilf(ascentMin + safetyMin);

    if (result > 99) {
        result = 99;
    }

    return result;
}

void DiveComputerApp::updateBatteryLowPopup() {
    const uint32_t now = millis();
    const uint8_t batteryPct = mockServices.getBatteryPct();

    if (batteryPct > BATTERY_LOW_THRESHOLD_PCT) {
        lastBatteryLowPopupMs_ = 0;
        batteryLowPopupUntilMs_ = 0;
        return;
    }

    if (lastBatteryLowPopupMs_ == 0 ||
        now - lastBatteryLowPopupMs_ >= BATTERY_LOW_POPUP_INTERVAL_MS) {

        lastBatteryLowPopupMs_ = now;
        batteryLowPopupUntilMs_ = now + BATTERY_LOW_POPUP_DURATION_MS;

        Serial.print("[WARN] BATTERY LOW: ");
        Serial.print(batteryPct);
        Serial.println("%");

        beep(ALARM_FREQ_BATTERY_LOW, 80);

    }
}

void DiveComputerApp::updatePostViolationAdvisory() {
    if (!postViolationAdvisory_) {
        return;
    }

    if (postViolationAdvisoryEndEpochSec_ == 0) {
        return;
    }

    uint32_t nowEpoch = getCurrentEpochSec();

    if (nowEpoch < postViolationAdvisoryEndEpochSec_) {
        return;
    }

    postViolationAdvisory_ = false;
    postViolationAdvisoryEndEpochSec_ = 0;

    Serial.println("[DECO] Post violation advisory ended");

    logDiveEvent(DiveEventType::EVENT_POST_VIOLATION_ADVISORY_ENDED,
                 "EVENT_POST_VIOLATION_ADVISORY_ENDED");
}


void DiveComputerApp::logDiveEvent(DiveEventType type, const char* eventName) {
    uint32_t elapsedSec = 0;

    if (dive_.diveStartMs > 0) {
        elapsedSec = (millis() - dive_.diveStartMs) / 1000UL;
    }

    Serial.printf("[EVENT] %s type=%u elapsed=%lus depth=%.1fm ceiling=%.1fm stop=%um remain=%lus\n",
                  eventName,
                  (unsigned)static_cast<uint8_t>(type),
                  (unsigned long)elapsedSec,
                  dive_.depthM,
                  dive_.ceilingDepthM,
                  dive_.decoStopDepthM,
                  (unsigned long)dive_.decoStopRemainSec);
}

bool DiveComputerApp::isBatteryLowPopupActive() const {
    const uint32_t now = millis();

    return batteryLowPopupUntilMs_ != 0 &&
           now < batteryLowPopupUntilMs_;
}

void DiveComputerApp::beep(uint32_t freq, uint32_t durationMs) {
    if (freq == 0 || durationMs == 0) {
        return;
    }

    if (durationMs < BUZZER_MIN_BEEP_MS) {
        durationMs = BUZZER_MIN_BEEP_MS;
    }

    tone(PIN_BUZZER, freq);
    delay(durationMs);
    noTone(PIN_BUZZER);
}


void DiveComputerApp::beepTripleWarning() {
    for (uint8_t i = 0; i < 3; i++) {
        beep(ALARM_FREQ_ASCENT_WARN, BUZZER_SHORT_BEEP_MS);
        delay(BUZZER_PATTERN_GAP_MS);
    }
}



void DiveComputerApp::drawSurfaceInfoScreen() {
    uint32_t surfaceIntervalSec = getSurfaceIntervalSec();
    uint32_t noFlyRemainSec = getNoFlyRemainSec();

    uint8_t batteryPct = mockServices.getBatteryPct();
    bool charging = mockServices.isCharging();
    bool chargeFull = charging && batteryPct >= BATTERY_FULL_THRESHOLD_PCT;

    uint32_t advisoryRemainSec = 0;
    uint32_t nowEpoch = getCurrentEpochSec();

    if (postViolationAdvisory_ &&
        postViolationAdvisoryEndEpochSec_ > nowEpoch) {
        advisoryRemainSec = postViolationAdvisoryEndEpochSec_ - nowEpoch;
    }

    uiDrawSurface(getCurrentEpochSec(),
                  SCENARIO_TZ_OFFSET_MIN,
                  batteryPct,
                  mockServices.isGpsValid(),
                  mockServices.isGpsSearching(),
                  mockServices.isBleConnected(),
                  mockServices.isBleAdvertising(),
                  charging,
                  chargeFull,
                  lastDiveStartEpochSec_,
                  lastDiveMaxDepthM_,
                  lastDiveMinTempC_,
                  surfaceIntervalSec,
                  noFlyRemainSec,
                  postViolationAdvisory_,
                  advisoryRemainSec,
                  activeDecoViolation_);
}

uint32_t DiveComputerApp::getSurfaceIntervalSec() const {
    uint32_t nowEpoch = getCurrentEpochSec();

    if (lastDiveEndEpochSec_ == 0 || nowEpoch <= lastDiveEndEpochSec_) {
        return 0;
    }

    return nowEpoch - lastDiveEndEpochSec_;
}

uint32_t DiveComputerApp::getNoFlyRemainSec() const {
    uint32_t nowEpoch = getCurrentEpochSec();

    if (noFlyEndEpochSec_ == 0 || nowEpoch >= noFlyEndEpochSec_) {
        return 0;
    }

    return noFlyEndEpochSec_ - nowEpoch;
}
