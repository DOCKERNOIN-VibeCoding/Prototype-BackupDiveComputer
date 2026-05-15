#include "buhlmann.h"
#include "config.h"
#include <math.h>

static const uint8_t DECO_LADDER_M[] = {
    18, 15, 12, 9, 6, 3
};

static const uint8_t DECO_LADDER_COUNT =
    sizeof(DECO_LADDER_M) / sizeof(DECO_LADDER_M[0]);


const float Buhlmann::N2_HALFTIME[NUM_COMPARTMENTS] = {
    4.0f, 8.0f, 12.5f, 18.5f, 27.0f, 38.3f, 54.3f, 77.0f,
    109.0f, 146.0f, 187.0f, 239.0f, 305.0f, 390.0f, 498.0f, 635.0f
};

const float Buhlmann::N2_A[NUM_COMPARTMENTS] = {
    1.2599f, 1.0000f, 0.8618f, 0.7562f,
    0.6200f, 0.5043f, 0.4410f, 0.4000f,
    0.3750f, 0.3500f, 0.3295f, 0.3065f,
    0.2835f, 0.2610f, 0.2480f, 0.2327f
};

const float Buhlmann::N2_B[NUM_COMPARTMENTS] = {
    0.5050f, 0.6514f, 0.7222f, 0.7825f,
    0.8126f, 0.8434f, 0.8693f, 0.8910f,
    0.9092f, 0.9222f, 0.9319f, 0.9403f,
    0.9477f, 0.9544f, 0.9602f, 0.9653f
};

float Buhlmann::getGasFO2() const {
    return (float)DIVE_GAS_FO2_PERCENT / 100.0f;
}

float Buhlmann::getGasFN2() const {
    float fo2 = getGasFO2();
    float fn2 = 1.0f - fo2;

    if (fn2 < 0.0f) {
        fn2 = 0.0f;
    }

    return fn2;
}

float Buhlmann::calculateMODMeters() const {
    float fo2 = getGasFO2();

    if (fo2 <= 0.0f) {
        return 0.0f;
    }

    return ((DIVE_GAS_PPO2_MAX_BAR / fo2) - 1.0f) * 10.0f;
}

float Buhlmann::calculatePpO2Bar(float depthM) const {
    if (depthM < 0.0f) {
        depthM = 0.0f;
    }

    float ambientBar = surfacePressureBar_ + depthM / 10.0f;
    return ambientBar * getGasFO2();
}

bool Buhlmann::isCeilingBeyondMaxStop(float ceilingDepthM) {
    return ceilingDepthM > DECO_MAX_STOP_DEPTH_M;
}

uint8_t Buhlmann::mapCeilingToDecoStopDepth(float ceilingDepthM) {
    if (ceilingDepthM <= 0.0f) {
        return 0;
    }

    if (ceilingDepthM <= (float)DECO_LAST_STOP_DEPTH_M) {
        return DECO_LAST_STOP_DEPTH_M;
    }

    if (ceilingDepthM <= 6.0f) return 6;
    if (ceilingDepthM <= 9.0f) return 9;
    if (ceilingDepthM <= 12.0f) return 12;
    if (ceilingDepthM <= 15.0f) return 15;
    if (ceilingDepthM <= 18.0f) return 18;

    return 0;
}

uint8_t Buhlmann::getNextShallowerStopDepth(uint8_t stopDepthM) {
    if (stopDepthM <= DECO_LAST_STOP_DEPTH_M) {
        return 0;
    }

    return stopDepthM - 3;
}

void Buhlmann::init(float surfacePressureBar) {
    surfacePressureBar_ = surfacePressureBar;
    gfLow_ = DEFAULT_GF_LOW;
    gfHigh_ = DEFAULT_GF_HIGH;

    float ppN2Surface = (surfacePressureBar_ - 0.0627f) * getGasFN2();

    for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
        tissuePN2_[i] = ppN2Surface;
    }

    Serial.printf("[DECO] init surface=%.4f ppN2=%.4f\n",
                  surfacePressureBar_, ppN2Surface);
}

void Buhlmann::setGF(uint8_t gfLow, uint8_t gfHigh) {
    if (gfLow < GF_LOW_MIN) {
        gfLow = GF_LOW_MIN;
    }

    if (gfLow > GF_LOW_MAX) {
        gfLow = GF_LOW_MAX;
    }

    if (gfHigh < GF_HIGH_MIN) {
        gfHigh = GF_HIGH_MIN;
    }

    if (gfHigh > GF_HIGH_MAX) {
        gfHigh = GF_HIGH_MAX;
    }

#if GF_REQUIRE_LOW_LESS_THAN_HIGH
    if (gfLow >= gfHigh) {
        if (gfHigh > GF_LOW_MIN) {
            gfLow = gfHigh - 1;
        } else {
            gfLow = GF_LOW_MIN;
            gfHigh = GF_LOW_MIN + 1;
        }
    }
#endif

    gfLow_ = gfLow;
    gfHigh_ = gfHigh;

    Serial.printf("[DECO] GF set %u/%u\n", gfLow_, gfHigh_);
}


void Buhlmann::update(float ambientBar, float intervalMin) {
    if (intervalMin <= 0.0f) return;

    float ppN2Inspired = (ambientBar - 0.0627f) * getGasFN2();

    for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
        float k = logf(2.0f) / N2_HALFTIME[i];
        float e = expf(-k * intervalMin);

        tissuePN2_[i] = ppN2Inspired + (tissuePN2_[i] - ppN2Inspired) * e;
    }
}

float Buhlmann::calcMValue(uint8_t comp, float ambientBar) const {
    return N2_A[comp] + ambientBar / N2_B[comp];
}

float Buhlmann::calcCeilingForComp(uint8_t comp, float gf) const {
    return calcCeilingForPN2(comp, tissuePN2_[comp], gf);
}

float Buhlmann::calcCeilingForPN2(uint8_t comp, float pN2, float gf) const {
    float a = N2_A[comp];
    float b = N2_B[comp];

    float denom = gf / b - gf + 1.0f;
    if (denom <= 0.0001f) return 0.0f;

    float ceiling = (pN2 - a * gf) / denom;
    return ceiling;
}

float Buhlmann::getCurrentCeilingBar() const {
    float gf = gfLow_ / 100.0f;
    float maxCeiling = 0.0f;

    for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
        float c = calcCeilingForComp(i, gf);
        if (c > maxCeiling) maxCeiling = c;
    }

    return maxCeiling;
}

float Buhlmann::getCurrentCeilingDepthM() const {
    float depth = (getCurrentCeilingBar() - surfacePressureBar_) * 10.0f;
    if (depth < 0.0f) depth = 0.0f;
    return depth;
}

float Buhlmann::getGF99(float ambientBar) const {
    float maxGF = 0.0f;
    float ppAmbN2 = (ambientBar - 0.0627f) * getGasFN2();

    for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
        float m = calcMValue(i, ambientBar);
        float denom = m - ppAmbN2;

        if (denom > 0.001f) {
            float gf = (tissuePN2_[i] - ppAmbN2) / denom * 100.0f;
            if (gf > maxGF) maxGF = gf;
        }
    }

    return maxGF;
}

uint16_t Buhlmann::calculateNDL(float ambientBar) const {
    float ppN2Inspired = (ambientBar - 0.0627f) * getGasFN2();
    float gfHigh = gfHigh_ / 100.0f;
    float minTime = 9999.0f;

    for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
        float a = N2_A[i];
        float b = N2_B[i];

        float pN2Limit =
            a * gfHigh +
            surfacePressureBar_ * (gfHigh / b - gfHigh + 1.0f);

        if (tissuePN2_[i] >= pN2Limit) {
            return 0;
        }

        if (ppN2Inspired <= pN2Limit) {
            continue;
        }

        float k = logf(2.0f) / N2_HALFTIME[i];
        float numerator = pN2Limit - ppN2Inspired;
        float denominator = tissuePN2_[i] - ppN2Inspired;

        if (fabsf(denominator) < 0.0001f) continue;

        float ratio = numerator / denominator;

        if (ratio <= 0.0f || ratio >= 1.0f) {
            continue;
        }

        float t = -logf(ratio) / k;

        if (t >= 0.0f && t < minTime) {
            minTime = t;
        }
    }

    if (minTime >= 9999.0f) return 99;
    if (minTime < 0.0f) return 0;

    uint16_t ndl = (uint16_t)floorf(minTime);
    if (ndl > 99) ndl = 99;
    return ndl;
}

DecoInfo Buhlmann::calculateDeco(float ambientBar, float ascentRateBarPerMin) const {
    DecoInfo info = {};
    info.ceiling_bar = getCurrentCeilingBar();
    info.ceiling_depth_m = getCurrentCeilingDepthM();
    info.stop_depth_m = 0;
    info.next_stop_depth_m = 0;
    info.stop_time_min = 0;
    info.stop_time_sec = 0;
    info.tts_min = 0;
    info.ceiling_gt_max_stop = false;

    if (info.ceiling_depth_m <= 0.0f) {
        return info;
    }

    if (isCeilingBeyondMaxStop(info.ceiling_depth_m)) {
        info.ceiling_gt_max_stop = true;
        return info;
    }

    uint8_t stopDepth = mapCeilingToDecoStopDepth(info.ceiling_depth_m);

    if (stopDepth == 0) {
        return info;
    }

    info.stop_depth_m = stopDepth;
    info.next_stop_depth_m = getNextShallowerStopDepth(stopDepth);


    float simPN2[NUM_COMPARTMENTS];
    for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
        simPN2[i] = tissuePN2_[i];
    }

    float gfLow = gfLow_ / 100.0f;
    float gfHigh = gfHigh_ / 100.0f;

    float currentDepthM = (ambientBar - surfacePressureBar_) * 10.0f;
    if (currentDepthM < 0.0f) currentDepthM = 0.0f;

    float ascentRateMpm = ascentRateBarPerMin * 10.0f;
    if (ascentRateMpm < 1.0f) ascentRateMpm = 10.0f;

    const uint16_t DECO_SIM_STEP_SEC = 20;

    uint32_t totalTimeSec = 0;

    if (currentDepthM > stopDepth) {
        totalTimeSec += (uint32_t)ceilf(
            ((currentDepthM - stopDepth) / ascentRateMpm) * 60.0f
        );
    }

    int currentStop = stopDepth;

    while (currentStop > 0 && totalTimeSec < 999UL * 60UL) {
        // Conservative stop-time policy:
        // The displayed stop depth remains the nominal ladder stop
        // such as 15m, 12m, 9m, 6m, or 3m.
        //
        // However, the stop-time simulation uses the deepest depth that
        // is still accepted as HOLD for that stop.
        //
        // Example:
        //   15m stop with +1.8m deep margin is calculated as 16.8m.
        //
        // This avoids underestimating the required stop time when the diver
        // stays near the deep edge of the valid stop window, and prevents
        // timer jitter caused by using instantaneous actual depth.
        float effectiveStopDepthM =
            (float)currentStop + DECO_STOP_DEEP_MARGIN_M;

        float stopAmbient = surfacePressureBar_ + effectiveStopDepthM / 10.0f;
        float ppN2Stop = (stopAmbient - 0.0627f) * getGasFN2();

        uint32_t stopTimeSec = 0;
        bool canAscend = false;

        while (!canAscend && stopTimeSec < 180UL * 60UL) {
            float stepMin = (float)DECO_SIM_STEP_SEC / 60.0f;

            for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
                float k = logf(2.0f) / N2_HALFTIME[i];
                float e = expf(-k * stepMin);
                simPN2[i] = ppN2Stop + (simPN2[i] - ppN2Stop) * e;
            }

            stopTimeSec += DECO_SIM_STEP_SEC;
            totalTimeSec += DECO_SIM_STEP_SEC;

            int nextStop = currentStop - 3;

            if (nextStop < (int)DECO_LAST_STOP_DEPTH_M) {
                nextStop = 0;
            }

            float nextAmbient = surfacePressureBar_ + nextStop / 10.0f;
            float fraction = 0.0f;

            if (stopDepth > 0) {
                fraction = (float)nextStop / (float)stopDepth;
                if (fraction < 0.0f) fraction = 0.0f;
                if (fraction > 1.0f) fraction = 1.0f;
            }

            float gfAtNext = gfHigh + (gfLow - gfHigh) * fraction;

            canAscend = true;

            for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
                float c = calcCeilingForPN2(i, simPN2[i], gfAtNext);
                if (c > nextAmbient) {
                    canAscend = false;
                    break;
                }
            }
        }

        if (currentStop == stopDepth) {
            uint32_t roundedStopTimeSec =
                ((stopTimeSec + DECO_SIM_STEP_SEC - 1) / DECO_SIM_STEP_SEC) *
                DECO_SIM_STEP_SEC;

            info.stop_time_sec = roundedStopTimeSec;
            info.stop_time_min = (uint16_t)ceilf((float)roundedStopTimeSec / 60.0f);
        }

        int nextStopAfterCurrent = currentStop - 3;

        if (nextStopAfterCurrent < (int)DECO_LAST_STOP_DEPTH_M) {
            nextStopAfterCurrent = 0;
        }

        currentStop = nextStopAfterCurrent;

        if (currentStop > 0) {
            totalTimeSec += 60UL;
        }
    }

    if (stopDepth > 0) {
        totalTimeSec += 60UL;
    }

    info.tts_min = (uint16_t)ceilf((float)totalTimeSec / 60.0f);
    return info;

}

float Buhlmann::getTissuePressure(uint8_t i) const {
    if (i >= NUM_COMPARTMENTS) return 0.0f;
    return tissuePN2_[i];
}

void Buhlmann::setTissuePressure(uint8_t i, float value) {
    if (i >= NUM_COMPARTMENTS) return;
    tissuePN2_[i] = value;
}

uint32_t Buhlmann::calculateNoFlyMinutes(float cabinPressureBar) const {
    float gfHigh = gfHigh_ / 100.0f;

    // 수면에서의 평형 질소분압
    float ppN2Surface = (surfacePressureBar_ - 0.0627f) * getGasFN2();

    float maxTimeMin = 0.0f;

    for (uint8_t i = 0; i < NUM_COMPARTMENTS; i++) {
        float p0 = tissuePN2_[i];

        // 기내압 기준 M-value
        float mValueCabin = N2_A[i] + cabinPressureBar / N2_B[i];

        // GF High 적용 기내압 기준 허용 조직 질소분압
        float gfLimit =
            cabinPressureBar +
            gfHigh * (mValueCabin - cabinPressureBar);

        // 이미 기내압 기준 허용 범위 이내이면 이 조직은 대기시간 0
        if (p0 <= gfLimit) {
            continue;
        }

        // 조직압이 수면 평형압보다 낮거나 같으면 더 이상 off-gassing으로 내려갈 필요 없음
        if (p0 <= ppN2Surface) {
            continue;
        }

        // 만약 gfLimit이 수면 평형압보다 낮거나 같다면, 이론적으로 수면 off-gassing만으로는
        // 해당 기준까지 떨어지지 않을 수 있음. 안전장치로 최대 시간 처리.
        if (gfLimit <= ppN2Surface) {
            maxTimeMin = (float)(NO_FLY_MAX_HOURS * 60UL);
            continue;
        }

        float halfTime = N2_HALFTIME[i];
        float k = logf(2.0f) / halfTime;

        // p(t) = ppSurf + (p0 - ppSurf) * exp(-k t)
        // gfLimit = ppSurf + (p0 - ppSurf) * exp(-k t)
        // exp(-k t) = (gfLimit - ppSurf) / (p0 - ppSurf)

        float ratio = (gfLimit - ppN2Surface) / (p0 - ppN2Surface);

        if (ratio <= 0.0f) {
            maxTimeMin = (float)(NO_FLY_MAX_HOURS * 60UL);
            continue;
        }

        if (ratio >= 1.0f) {
            continue;
        }

        float tMin = -logf(ratio) / k;

        if (tMin > maxTimeMin) {
            maxTimeMin = tMin;
        }
    }

    if (maxTimeMin < 0.0f) {
        maxTimeMin = 0.0f;
    }

    uint32_t result = (uint32_t)ceilf(maxTimeMin);

    uint32_t maxMinutes = NO_FLY_MAX_HOURS * 60UL;

    if (result > maxMinutes) {
        result = maxMinutes;
    }

    return result;
}
