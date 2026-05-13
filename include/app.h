#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include "buhlmann.h"
#include "log_format.h"

enum class SystemState {
    Surface,
    Dive,
    PostDive,
    Charging
};

enum class DivePhase {
    Normal,
    SafetyStop,
    Deco
};

struct DiveRuntime {
    DivePhase phase = DivePhase::Normal;

    float depthM = 0.0f;
    float prevDepthM = 0.0f;
    float maxDepthM = 0.0f;

    float tempC = 0.0f;
    float minTempC = 99.0f;

    float ascentRateMpm = 0.0f;
    float ascentSampleDepthM = 0.0f;
    uint32_t lastAscentSampleMs = 0;

    uint16_t ndlMin = 99;

    uint32_t diveStartMs = 0;
    uint32_t lastDepthMs = 0;
    uint32_t lastDecoUpdateMs = 0;
    uint32_t lastLogMs = 0;

    uint16_t sampleCount = 0;

    bool decoEntered = false;
    uint8_t decoStopDepthM = 0;
    uint8_t lastDecoStopDepthM = 0;
    uint16_t decoStopTimeMin = 0;
    uint32_t decoStopRemainSec = 0;
    uint32_t lastDecoStopTickMs = 0;
    uint16_t decoTtsMin = 0;
    float ceilingDepthM = 0.0f;
    bool decoViolation = false;
    bool decoCeilingGtMaxStop = false;

    bool safetyTriggered = false;
    bool safetyCompleted = false;
    bool safetySkipped = false;
    bool safetyPaused = false;

    uint32_t safetySkippedAtMs = 0;

    uint32_t safetyElapsedMs = 0;
    uint32_t safetyPauseStartMs = 0;
    uint32_t lastSafetyTickMs = 0;

    // 0~3m shallow deviation duration during safety stop
    uint32_t safetyShallowStartMs = 0;

    // >6m deep deviation duration during safety stop
    uint32_t safetyDeepStartMs = 0;

    uint32_t surfaceStartMs = 0;
};

class DiveComputerApp {
public:
    void begin();
    void update();

private:
    SystemState state_ = SystemState::Surface;
    SystemState previousState_ = SystemState::Surface;

    Buhlmann deco_;
    DiveRuntime dive_;

    uint16_t diveCount_ = 0;

    uint32_t noFlyEndSimSec_ = 0;
    uint32_t postDiveStartMs_ = 0;
    uint32_t lastOffgasMs_ = 0;
    uint32_t lastUiMs_ = 0;

    uint32_t chargingEnterMs_ = 0;
    uint32_t bleAccessWindowUntilMs_ = 0;

    uint32_t surfaceIntervalStartMs_ = 0;
    // Used for scenario preload.
    // Example: previous dive ended 84 minutes before simulation start.
    uint32_t surfaceIntervalOffsetSec_ = 0;

    uint8_t diveDetectCount_ = 0;

    uint32_t currentDiveStartEpochSec_ = 0;

    // Battery low popup state
    uint32_t lastBatteryLowPopupMs_ = 0;
    uint32_t batteryLowPopupUntilMs_ = 0;

    float lastDiveMaxDepthM_ = 0.0f;
    float lastDiveMinTempC_ = 0.0f;
    uint32_t lastDiveStartEpochSec_ = 0;
    uint32_t lastDiveDurationSec_ = 0;
    uint32_t lastDiveEndEpochSec_ = 0;
    uint32_t noFlyEndEpochSec_ = 0;
    bool activeDecoViolation_ = false;
    bool postViolationAdvisory_ = false;
    uint32_t postViolationAdvisoryEndEpochSec_ = 0;
    uint8_t reentryCount_ = 0;
    bool clearedAfterReentry_ = false;
    bool pendingDiveClose_ = false;

    bool gpsLimitedSearchActive_ = false;
    uint8_t gpsSearchAttempt_ = 0;
    uint32_t gpsSearchAttemptStartedMs_ = 0;

private:
    void setState(SystemState newState);

    void handleSurface();
    void handleDive();
    void handlePostDive();
    void handleCharging();

    void startDive();
    void endDive();
    void resumeContinuousDive();
    void finalizeDiveLog();

    void startLimitedGpsSearch();
    void stopLimitedGpsSearch();
    void updateGpsBleAutoPower();

    void updateOffgassing();
    void updateGFIfNeeded();

    void updateBatteryLowPopup();
    bool isBatteryLowPopupActive() const;

    void applyScenarioPreload();

    uint32_t getSimEpochSec() const;
    uint32_t calcNoFlyMinutes() const;

    uint16_t calcNormalTTSMin() const;
    uint16_t calcSafetyTTSMin(uint32_t safetyRemainSec) const;

    void beep(uint32_t freq, uint32_t durationMs);
    void beepTripleWarning();

    uint32_t getCurrentEpochSec() const;
    uint32_t getSurfaceIntervalSec() const;
    uint32_t getNoFlyRemainSec() const;
    void updatePostViolationAdvisory();
    void logDiveEvent(DiveEventType type, const char* eventName);
    void drawSurfaceInfoScreen();
    bool shouldShowChargingSplash(uint32_t now) const;
};

extern DiveComputerApp app;

#endif
