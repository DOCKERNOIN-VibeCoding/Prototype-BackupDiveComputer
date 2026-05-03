#include "sim_sensor.h"
#include "generated_scenario.h"
#include "config.h"

#include <math.h>

SimSensor simSensor;

void SimSensor::begin() {
    profileStartMs_ = millis();
    pausedStartMs_ = 0;
    pausedAccumMs_ = 0;

    autoProfile_ = true;
    paused_ = false;

    manualDepthM_ = 0.0f;
    depthM_ = 0.0f;
    temperatureC_ = SCENARIO_INITIAL_TEMP_C;
    surfacePressureBar_ = SURFACE_PRESSURE_BAR;

    Serial.println();
    Serial.println("[SIM] Scenario loaded");
    Serial.print("[SIM] Name: ");
    Serial.println(SCENARIO_NAME);
    Serial.print("[SIM] Description: ");
    Serial.println(SCENARIO_DESCRIPTION);
    Serial.print("[SIM] Points: ");
    Serial.println((uint32_t)SCENARIO_POINT_COUNT);
    Serial.print("[SIM] Initial Temp: ");
    Serial.print(SCENARIO_INITIAL_TEMP_C, 1);
    Serial.println(" C");
    Serial.println();
}

void SimSensor::update() {
    if (paused_) {
        return;
    }

    if (!autoProfile_) {
        depthM_ = manualDepthM_;
        temperatureC_ = SCENARIO_INITIAL_TEMP_C;
        return;
    }

    uint32_t elapsedMs = getElapsedProfileMs();
    sampleScenario(elapsedMs, depthM_, temperatureC_);
}

void SimSensor::setAutoProfile(bool enabled) {
    autoProfile_ = enabled;

    if (enabled) {
        resetProfile();
        Serial.println("[SIM] Auto scenario profile enabled");
    } else {
        paused_ = false;
        manualDepthM_ = depthM_;
        Serial.println("[SIM] Manual depth mode enabled");
    }
}

void SimSensor::setManualDepth(float depthM) {
    if (depthM < 0.0f) {
        depthM = 0.0f;
    }

    autoProfile_ = false;
    paused_ = false;

    manualDepthM_ = depthM;
    depthM_ = manualDepthM_;

    Serial.print("[SIM] Manual depth set: ");
    Serial.print(manualDepthM_, 1);
    Serial.println(" m");
}

void SimSensor::pauseProfile() {
    if (!autoProfile_) {
        Serial.println("[SIM] Cannot pause: manual depth mode");
        return;
    }

    if (paused_) {
        Serial.println("[SIM] Profile already paused");
        return;
    }

    paused_ = true;
    pausedStartMs_ = millis();

    Serial.println("[SIM] Auto profile paused");
}

void SimSensor::resumeProfile() {
    if (!autoProfile_) {
        Serial.println("[SIM] Cannot resume: manual depth mode");
        return;
    }

    if (!paused_) {
        Serial.println("[SIM] Profile is not paused");
        return;
    }

    uint32_t now = millis();

    if (now > pausedStartMs_) {
        pausedAccumMs_ += now - pausedStartMs_;
    }

    pausedStartMs_ = 0;
    paused_ = false;

    Serial.println("[SIM] Auto profile resumed");
}

void SimSensor::resetProfile() {
    profileStartMs_ = millis();
    pausedStartMs_ = 0;
    pausedAccumMs_ = 0;

    autoProfile_ = true;
    paused_ = false;

    depthM_ = 0.0f;
    manualDepthM_ = 0.0f;
    temperatureC_ = SCENARIO_INITIAL_TEMP_C;
}

uint32_t SimSensor::getElapsedProfileMs() const {
    uint32_t now = millis();

    if (paused_) {
        now = pausedStartMs_;
    }

    if (now < profileStartMs_) {
        return 0;
    }

    uint32_t rawElapsed = now - profileStartMs_;

    if (rawElapsed <= pausedAccumMs_) {
        return 0;
    }

    return rawElapsed - pausedAccumMs_;
}

uint32_t SimSensor::getElapsedProfileSec() const {
    return getElapsedProfileMs() / 1000UL;
}

void SimSensor::sampleScenario(uint32_t elapsedMs,
                               float& depthM,
                               float& temperatureC) const {
    if (SCENARIO_POINT_COUNT == 0) {
        depthM = 0.0f;
        temperatureC = SCENARIO_INITIAL_TEMP_C;
        return;
    }

    uint32_t t = elapsedMs;

    const uint32_t lastTimeMs =
        SCENARIO_POINTS[SCENARIO_POINT_COUNT - 1].tMs;

    if (SCENARIO_LOOP && lastTimeMs > 0) {
        t = t % lastTimeMs;
    }

    if (t <= SCENARIO_POINTS[0].tMs) {
        depthM = SCENARIO_POINTS[0].depthM;
        temperatureC = SCENARIO_POINTS[0].tempC;
        return;
    }

    for (size_t i = 1; i < SCENARIO_POINT_COUNT; i++) {
        const ScenarioPoint& prev = SCENARIO_POINTS[i - 1];
        const ScenarioPoint& next = SCENARIO_POINTS[i];

        if (t <= next.tMs) {
            const float spanMs = (float)(next.tMs - prev.tMs);

            if (spanMs <= 0.0f) {
                depthM = next.depthM;
                temperatureC = next.tempC;
                return;
            }

            const float ratio = (float)(t - prev.tMs) / spanMs;

            depthM =
                prev.depthM +
                (next.depthM - prev.depthM) * ratio;

            temperatureC =
                prev.tempC +
                (next.tempC - prev.tempC) * ratio;

            if (depthM < 0.0f) {
                depthM = 0.0f;
            }

            return;
        }
    }

    depthM = SCENARIO_POINTS[SCENARIO_POINT_COUNT - 1].depthM;
    temperatureC = SCENARIO_POINTS[SCENARIO_POINT_COUNT - 1].tempC;

    if (depthM < 0.0f) {
        depthM = 0.0f;
    }
}
