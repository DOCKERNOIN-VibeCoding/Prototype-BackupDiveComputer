#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef WOKWI_SIMULATION
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
#else
extern U8G2_ST7567_OS12864_F_4W_HW_SPI u8g2;
#endif

void uiBegin();
void uiSplash();

void uiDrawSurface(uint32_t currentEpochSec,
                   int16_t tzOffsetMin,
                   uint8_t batteryPct,
                   bool gpsValid,
                   bool charging,
                   bool chargeFull,
                   uint32_t lastDiveStartEpochSec
                   float lastMaxDepthM,
                   float lastMinTempC,
                   uint32_t surfaceIntervalSec,
                   uint32_t noFlyRemainSec);

void uiDrawDiveNormal(float depthM,
                      uint16_t ndlMin,
                      uint16_t ttsMin,
                      uint32_t diveTimeSec,
                      float maxDepthM,
                      float tempC,
                      float ascentRateMpm,
                      uint8_t batteryPct);

void uiDrawDiveSafety(float depthM,
                      uint32_t remainSec,
                      bool paused,
                      uint16_t ttsMin,
                      uint32_t diveTimeSec,
                      float maxDepthM,
                      float tempC,
                      float ascentRateMpm,
                      uint8_t batteryPct);

void uiDrawDiveSkipped(float depthM,
                       uint16_t ttsMin,
                       uint32_t diveTimeSec,
                       float maxDepthM,
                       float tempC,
                       float ascentRateMpm,
                       uint8_t batteryPct);

void uiDrawDiveDeco(float depthM,
                    uint8_t stopDepthM,
                    uint32_t stopRemainSec,
                    uint16_t ttsMin,
                    uint32_t diveTimeSec,
                    float maxDepthM,
                    float tempC,
                    float ascentRateMpm,
                    uint8_t batteryPct);

void uiDrawPostDive(uint16_t diveCount,
                    uint32_t diveTimeSec,
                    float maxDepthM,
                    float minTempC,
                    uint32_t noFlyRemainSec,
                    bool gpsValid,
                    uint8_t batteryPct);

void uiDrawCharging(uint8_t batteryPct,
                    bool bleConnected,
                    uint8_t gfLow,
                    uint8_t gfHigh);

void uiDrawAscBar(float ascentRateMpm);

void uiDrawBatteryLowPopup(uint8_t batteryPct);

#endif
