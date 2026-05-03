#ifndef MOCK_SERVICES_H
#define MOCK_SERVICES_H

#include <Arduino.h>
#include "sim_sensor.h"
#include "config.h"

class MockServices {
public:
    void begin();
    void update(SimSensor& sensor);

    bool isCharging() const { return charging_; }
    bool isBleConnected() const { return bleConnected_; }
    bool isGpsValid() const { return gpsValid_; }
    uint8_t getBatteryPct() const { return batteryPct_; }

    bool hasNewGF() const { return gfUpdated_; }
    uint8_t getGFLow() const { return gfLow_; }
    uint8_t getGFHigh() const { return gfHigh_; }
    void clearGF() { gfUpdated_ = false; }

    void loggerStartDive(uint16_t diveId);
    void loggerSample(uint16_t sampleCount,
                      float depthM,
                      float tempC,
                      uint16_t ndlOrTts);
    void loggerEndDive(uint16_t diveId,
                       uint32_t durationSec,
                       float maxDepthM);

private:
    void printHelp();
    void handleCommand(String cmd, SimSensor& sensor);

    String line_;

    bool charging_ = false;
    bool bleConnected_ = false;
    bool gpsValid_ = false;

    uint8_t batteryPct_ = DEFAULT_BATTERY_PCT;

    uint8_t gfLow_ = DEFAULT_GF_LOW;
    uint8_t gfHigh_ = DEFAULT_GF_HIGH;
    bool gfUpdated_ = false;
};

extern MockServices mockServices;

#endif
