#ifndef BUHLMANN_H
#define BUHLMANN_H

#include <Arduino.h>
#include <stdint.h>

#define NUM_COMPARTMENTS 16

struct DecoInfo {
    float ceiling_bar;
    float ceiling_depth_m;
    uint8_t stop_depth_m;
    uint8_t next_stop_depth_m;
    uint16_t stop_time_min;
    uint32_t stop_time_sec;
    uint16_t tts_min;
    bool ceiling_gt_max_stop;
};

class Buhlmann {
public:
    void init(float surfacePressureBar);
    void setGF(uint8_t gfLow, uint8_t gfHigh);

    void update(float ambientBar, float intervalMin);

    uint16_t calculateNDL(float ambientBar) const;
    DecoInfo calculateDeco(float ambientBar, float ascentRateBarPerMin) const;

    // 현재 조직 분압 기준으로 항공기 기내압에서 안전해질 때까지의 시간
    uint32_t calculateNoFlyMinutes(float cabinPressureBar) const;

    float getCurrentCeilingBar() const;
    float getCurrentCeilingDepthM() const;
    float getGF99(float ambientBar) const;

    float getGasFO2() const;
    float getGasFN2() const;

    float calculateMODMeters() const;
    float calculatePpO2Bar(float depthM) const;

    static uint8_t mapCeilingToDecoStopDepth(float ceilingDepthM);
    static uint8_t getNextShallowerStopDepth(uint8_t stopDepthM);
    static bool isCeilingBeyondMaxStop(float ceilingDepthM);

    float getTissuePressure(uint8_t i) const;
    void setTissuePressure(uint8_t i, float value);

    uint8_t getGFLow() const { return gfLow_; }
    uint8_t getGFHigh() const { return gfHigh_; }
    float getSurfacePressureBar() const { return surfacePressureBar_; }

private:
    float tissuePN2_[NUM_COMPARTMENTS];
    float surfacePressureBar_ = 1.01325f;
    uint8_t gfLow_ = 40;
    uint8_t gfHigh_ = 85;

    static const float N2_HALFTIME[NUM_COMPARTMENTS];
    static const float N2_A[NUM_COMPARTMENTS];
    static const float N2_B[NUM_COMPARTMENTS];

    float calcMValue(uint8_t comp, float ambientBar) const;
    float calcCeilingForComp(uint8_t comp, float gf) const;
    float calcCeilingForPN2(uint8_t comp, float pN2, float gf) const;
};

#endif
