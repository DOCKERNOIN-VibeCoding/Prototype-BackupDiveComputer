#ifndef LOG_STORAGE_H
#define LOG_STORAGE_H

#include <Arduino.h>
#include "log_format.h"

// ------------------------------------------------------------
// LogStorage
// ------------------------------------------------------------
//
// 이 파일은 "로그 저장 기능의 사용법"을 정하는 파일입니다.
//
// app.cpp에서는 복잡한 저장 방식을 몰라도 됩니다.
// 그냥 이렇게만 부르면 됩니다.
//
//   logStorage.begin();
//   logStorage.saveLastDive(header);
//   logStorage.loadLastDive(header);
//
// 실제 저장 방식은 src/log_storage.cpp 안에 들어갑니다.
// ------------------------------------------------------------

class LogStorage {
public:
    bool begin();

    bool saveLastDive(const DiveLogHeader& header);
    bool loadLastDive(DiveLogHeader& header);

    void printHeader(const DiveLogHeader& header);

private:
    bool ready_ = false;
};

extern LogStorage logStorage;

#endif
