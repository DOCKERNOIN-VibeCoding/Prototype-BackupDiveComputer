#ifndef SIM_SENSOR_H
#define SIM_SENSOR_H

#include <Arduino.h>

class SimSensor {
public:
    void begin();
    void update();

    void setAutoProfile(bool enabled);
    bool isAutoProfile() const { return autoProfile_; }

    void setManualDepth(float depthM);

    void pauseProfile();
    void resumeProfile();
    bool isPaused() const { return paused_; }

    float getDepthM() const { return depthM_; }
    float getTemperatureC() const { return temperatureC_; }
    float getTempC() const { return temperatureC_; }

    float getSurfacePressureBar() const { return surfacePressureBar_; }

    uint32_t getElapsedProfileMs() const;
    uint32_t getElapsedProfileSec() const;

private:
    void resetProfile();
    void sampleScenario(uint32_t elapsedMs, float& depthM, float& temperatureC) const;

private:
    uint32_t profileStartMs_ = 0;
    uint32_t pausedStartMs_ = 0;
    uint32_t pausedAccumMs_ = 0;

    bool autoProfile_ = true;
    bool paused_ = false;

    float manualDepthM_ = 0.0f;
    float depthM_ = 0.0f;
    float temperatureC_ = 24.0f;
    float surfacePressureBar_ = 1.01325f;
};

extern SimSensor simSensor;

#endif
