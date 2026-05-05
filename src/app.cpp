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

    delay(1000);
}

void DiveComputerApp::update() {
    mockServices.update(simSensor);
    simSensor.update();

    updateGFIfNeeded();
    updateBatteryLowPopup();

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
            break;

        case SystemState::Dive:
            Serial.println("[STATE] DIVE");
            break;

        case SystemState::PostDive:
            Serial.println("[STATE] POST_DIVE");
            postDiveStartMs_ = millis();
            break;

        case SystemState::Charging:
            Serial.println("[STATE] CHARGING");
            break;
    }
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

    Serial.printf("[DIVE] start #%u depth=%.1fm\n", diveCount_, dive_.depthM);
    Serial.printf("[DIVE] tissue pN2 c0=%.3f c4=%.3f c8=%.3f c15=%.3f\n",
                  deco_.getTissuePressure(0),
                  deco_.getTissuePressure(4),
                  deco_.getTissuePressure(8),
                  deco_.getTissuePressure(15));

    beep(1200, 80);

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
    if (dive_.depthM < DIVE_END_DEPTH_M) {
        if (dive_.surfaceStartMs == 0) {
            dive_.surfaceStartMs = now;
            Serial.println("[DIVE] surface detection started");
        }

        if ((now - dive_.surfaceStartMs) / 1000UL >= DIVE_END_SURFACE_SEC) {
            endDive();
            return;
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
            beep(ALARM_FREQ_ASCENT_WARN, 120);
        }
    }

    // ------------------------------------------------------------
    // Dive phase logic
    // ------------------------------------------------------------
    switch (dive_.phase) {
        case DivePhase::Normal: {
            if (dive_.ndlMin == 0) {
                dive_.phase = DivePhase::Deco;
                dive_.decoEntered = true;

                Serial.println("[DIVE] Enter DECO");
                beep(ALARM_FREQ_DECO_ENTER, 150);
                delay(100);
                beep(ALARM_FREQ_DECO_ENTER, 150);
                delay(100);
                beep(ALARM_FREQ_DECO_ENTER, 150);
                break;
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
                beep(1000, 150);
                break;
            }

            break;
        }

        case DivePhase::SafetyStop: {
            bool inRange =
                dive_.depthM >= SAFETY_STOP_MIN_DEPTH_M &&
                dive_.depthM <= SAFETY_STOP_MAX_DEPTH_M;

            bool shallowOut =
                dive_.depthM > 0.0f &&
                dive_.depthM < SAFETY_STOP_MIN_DEPTH_M;

            bool deepOut =
                dive_.depthM > SAFETY_STOP_MAX_DEPTH_M;

            if (inRange) {
                dive_.safetyShallowStartMs = 0;
                dive_.safetyDeepStartMs = 0;

                if (dive_.safetyPaused) {
                    dive_.safetyPaused = false;
                    Serial.println("[DIVE] Safety stop RESUMED");
                    beep(1000, 100);
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
                    beep(ALARM_FREQ_SAFETY_DONE, 150);
                    delay(100);
                    beep(ALARM_FREQ_SAFETY_DONE, 150);
                    delay(100);
                    beep(ALARM_FREQ_SAFETY_DONE, 150);
                    break;
                }
            } else {
                if (!dive_.safetyPaused) {
                    dive_.safetyPaused = true;
                    Serial.printf("[DIVE] Safety stop PAUSED at %.1fm\n", dive_.depthM);
                    beep(800, 100);
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
                        beep(900, 120);
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
            DecoInfo info = deco_.calculateDeco(ambientBar, 1.0f);

            dive_.decoStopDepthM = info.stop_depth_m;
            dive_.decoStopTimeMin = info.stop_time_min;
            dive_.decoTtsMin = info.tts_min;
            dive_.ceilingDepthM = info.ceiling_depth_m;

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
                dive_.depthM <= (float)dive_.decoStopDepthM + 1.0f &&
                dive_.depthM >= (float)dive_.decoStopDepthM - 0.5f;

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

            static uint32_t lastCeilingAlarmMs = 0;

            if (dive_.ceilingDepthM > 0.5f &&
                dive_.depthM < dive_.ceilingDepthM - 0.5f) {

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

                beep(ALARM_FREQ_DECO_ENTER, 150);
                delay(100);
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
                                mockServices.getBatteryPct());
                break;
        }
    }
}

void DiveComputerApp::endDive() {
    uint32_t now = millis();

    if (dive_.decoEntered && dive_.ceilingDepthM > 0.5f) {
        dive_.decoViolation = true;
        Serial.println("[DIVE] DECO violation: surfaced with active ceiling");
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

    // ------------------------------------------------------------
    // v7.3:
    // 다이빙 종료 시 compact log header 저장
    // ------------------------------------------------------------
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


    Serial.printf("[DIVE] end #%u duration=%lus max=%.1fm samples=%u\n",
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

    mockServices.loggerEndDive(diveCount_, durationSec, dive_.maxDepthM);

    beep(1000, 120);
    delay(100);
    beep(1000, 120);

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

            startDive();
            return;
        }
    } else {
        diveDetectCount_ = 0;
    }

    if (now - lastUiMs_ >= UI_UPDATE_INTERVAL_MS) {
        lastUiMs_ = now;

        uint32_t remain = getNoFlyRemainSec();

        uiDrawPostDive(diveCount_,
                       lastDiveDurationSec_,
                       lastDiveMaxDepthM_,
                       dive_.minTempC,
                       remain,
                       mockServices.isGpsValid(),
                       mockServices.getBatteryPct());
    }

    if (now - postDiveStartMs_ >= POST_DIVE_DISPLAY_MS) {
        Serial.println("[POST] timeout -> SURFACE");

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
        drawSurfaceInfoScreen();
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

        // Optional beep.
        // beep(900, 120);
    }
}

bool DiveComputerApp::isBatteryLowPopupActive() const {
    const uint32_t now = millis();

    return batteryLowPopupUntilMs_ != 0 &&
           now < batteryLowPopupUntilMs_;
}

void DiveComputerApp::beep(uint32_t freq, uint32_t durationMs) {
    tone(PIN_BUZZER, freq, durationMs);
}

void DiveComputerApp::drawSurfaceInfoScreen() {
    uint32_t surfaceIntervalSec = getSurfaceIntervalSec();
    uint32_t noFlyRemainSec = getNoFlyRemainSec();

    uint8_t batteryPct = mockServices.getBatteryPct();
    bool charging = mockServices.isCharging();
    bool chargeFull = charging && batteryPct >= BATTERY_FULL_THRESHOLD_PCT;

    uiDrawSurface(getCurrentEpochSec(),
                SCENARIO_TZ_OFFSET_MIN,
                batteryPct,
                mockServices.isGpsValid(),
                charging,
                chargeFull,
                lastDiveStartEpochSec_,
                lastDiveMaxDepthM_,
                lastDiveMinTempC_,
                surfaceIntervalSec,
                noFlyRemainSec);

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
