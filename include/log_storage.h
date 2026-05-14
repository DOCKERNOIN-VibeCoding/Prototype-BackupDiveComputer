#ifndef LOG_STORAGE_H
#define LOG_STORAGE_H

#include <Arduino.h>
#include "log_format.h"

class LogStorage {
public:
    bool begin();

    // Header-only compatibility/read path
    bool saveLastDive(const DiveLogHeader& header);
    bool loadLastDive(DiveLogHeader& header);

    // Full native BDC log path
    bool saveLastDive(const DiveLogHeader& header,
                      const DiveSample* samples,
                      uint16_t sampleCount,
                      const DiveEvent* events,
                      uint16_t eventCount);

    bool loadLastDive(DiveLogHeader& header,
                      DiveSample* samples,
                      uint16_t maxSamples,
                      uint16_t& sampleCountOut,
                      DiveEvent* events,
                      uint16_t maxEvents,
                      uint16_t& eventCountOut);

    // Update only the header while preserving existing sample/event payload.
    // Used for RTS/GPS time correction after the dive.
    bool updateLastDiveHeader(const DiveLogHeader& header);

    bool hasLastDive();
    bool clearLastDive();

    void printHeader(const DiveLogHeader& header);
    void printDiveLog(const DiveLogHeader& header,
                      const DiveSample* samples,
                      uint16_t sampleCount,
                      const DiveEvent* events,
                      uint16_t eventCount);

private:
    bool ready_ = false;

#ifdef WOKWI_SIMULATION
    bool ramFallback_ = false;
    bool ramHasLastDive_ = false;

    DiveLogHeader ramLastDiveHeader_ = {};
    DiveSample ramLastDiveSamples_[BDC_MAX_DIVE_SAMPLES] = {};
    DiveEvent ramLastDiveEvents_[BDC_MAX_DIVE_EVENTS] = {};
    uint16_t ramLastDiveSampleCount_ = 0;
    uint16_t ramLastDiveEventCount_ = 0;
#endif
};

extern LogStorage logStorage;

#endif
