#include "mock_services.h"
#include "config.h"
#include "log_storage.h"


MockServices mockServices;

void MockServices::begin() {
    line_.reserve(80);

    charging_ = false;

    bleConnected_ = false;
    bleAdvertising_ = false;

    gpsValid_ = false;
    gpsSearching_ = false;
    gpsSearchStartedMs_ = 0;

    batteryPct_ = DEFAULT_BATTERY_PCT;
    gfLow_ = DEFAULT_GF_LOW;
    gfHigh_ = DEFAULT_GF_HIGH;
    gfUpdated_ = false;

    Serial.println("[MOCK] Services begin");
    printHelp();
}

void MockServices::update(SimSensor& sensor) {
    while (Serial.available()) {
        char c = (char)Serial.read();

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            line_.trim();

            if (line_.length() > 0) {
                handleCommand(line_, sensor);
            }

            line_ = "";
        } else {
            line_ += c;
        }
    }
        if (gpsSearching_ &&
        !gpsValid_ &&
        gpsSearchStartedMs_ > 0 &&
        millis() - gpsSearchStartedMs_ >= MOCK_GPS_AUTO_FIX_DELAY_MS) {

        gpsValid_ = true;
        gpsSearching_ = false;
        gpsSearchStartedMs_ = 0;

        Serial.println("[MOCK_GPS] auto valid");
    }
}

void MockServices::setGpsSearching() {
    if (gpsValid_ || gpsSearching_) {
        return;
    }

    gpsValid_ = false;
    gpsSearching_ = true;
    gpsSearchStartedMs_ = millis();
    Serial.println("[MOCK_GPS] auto search");
}

void MockServices::setGpsOff() {
    if (!gpsValid_ && !gpsSearching_) {
        return;
    }

    gpsValid_ = false;
    gpsSearching_ = false;
    gpsSearchStartedMs_ = 0;
    Serial.println("[MOCK_GPS] auto off");
}

void MockServices::setBleAdvertising() {
    if (bleConnected_ || bleAdvertising_) {
        return;
    }

    bleConnected_ = false;
    bleAdvertising_ = true;
    Serial.println("[MOCK_BLE] auto advertising");
}

void MockServices::setBleOff() {
    if (!bleConnected_ && !bleAdvertising_) {
        return;
    }

    bleConnected_ = false;
    bleAdvertising_ = false;
    Serial.println("[MOCK_BLE] auto off");
}

void MockServices::printHelp() {
    Serial.println();
    Serial.println("========== SIM COMMANDS ==========");
    Serial.println("help");
    Serial.println("profile auto");
    Serial.println("profile pause");
    Serial.println("profile resume");
    Serial.println("depth 20.0");
    Serial.println("gf 40 85");
    Serial.println("charge on");
    Serial.println("charge off");
    Serial.println("ble ready");
    Serial.println("ble on");
    Serial.println("ble off");
    Serial.println("gps search");
    Serial.println("gps ok");
    Serial.println("gps fail");
    Serial.println("battery 75");
    Serial.println("beep test");
    Serial.println("log info");
    Serial.println("log clear");
    Serial.println("==================================");
    Serial.println();
}

void MockServices::handleCommand(String cmd, SimSensor& sensor) {
    cmd.trim();
    cmd.toLowerCase();

    Serial.print("[CMD] ");
    Serial.println(cmd);

    if (cmd == "help") {
        printHelp();
        return;
    }

    if (cmd == "profile auto") {
        sensor.setAutoProfile(true);
        return;
    }

    if (cmd == "profile pause") {
        sensor.pauseProfile();
        return;
    }

    if (cmd == "profile resume") {
        sensor.resumeProfile();
        return;
    }

    if (cmd.startsWith("depth ")) {
        float d = cmd.substring(6).toFloat();
        sensor.setManualDepth(d);
        return;
    }

    if (cmd.startsWith("gf ")) {
        int firstSpace = cmd.indexOf(' ');
        int secondSpace = cmd.indexOf(' ', firstSpace + 1);

        if (secondSpace > 0) {
            int low = cmd.substring(firstSpace + 1, secondSpace).toInt();
            int high = cmd.substring(secondSpace + 1).toInt();

            if (low >= GF_LOW_MIN && low <= GF_LOW_MAX &&
                high >= GF_HIGH_MIN && high <= GF_HIGH_MAX &&
                low < high) {

                gfLow_ = low;
                gfHigh_ = high;
                gfUpdated_ = true;

                Serial.printf("[MOCK_BLE] GF update requested %u/%u\n",
                            gfLow_, gfHigh_);
            } else {
                Serial.printf("[MOCK_BLE] Invalid GF. Allowed LOW %d-%d, HIGH %d-%d, LOW < HIGH\n",
                            GF_LOW_MIN,
                            GF_LOW_MAX,
                            GF_HIGH_MIN,
                            GF_HIGH_MAX);
            }
        }

        return;
    }


    if (cmd == "charge on") {
        charging_ = true;
        Serial.println("[MOCK_PWR] charging ON");
        return;
    }

    if (cmd == "charge off") {
        charging_ = false;
        Serial.println("[MOCK_PWR] charging OFF");
        return;
    }
    
    if (cmd == "ble ready") {
        bleConnected_ = false;
        bleAdvertising_ = true;
        Serial.println("[MOCK_BLE] advertising / ready");
        return;
    }

    if (cmd == "ble on") {
        bleConnected_ = true;
        bleAdvertising_ = false;
        Serial.println("[MOCK_BLE] connected");
        return;
    }

    if (cmd == "ble off") {
        bleConnected_ = false;
        bleAdvertising_ = false;
        Serial.println("[MOCK_BLE] disconnected");
        return;
    }

    if (cmd == "gps search") {
        gpsValid_ = false;
        gpsSearching_ = true;
        gpsSearchStartedMs_ = millis();
        Serial.println("[MOCK_GPS] searching");
        return;
    }

    if (cmd == "gps ok") {
        gpsValid_ = true;
        gpsSearching_ = false;
        gpsSearchStartedMs_ = 0;
        Serial.println("[MOCK_GPS] valid");
        return;
    }

    if (cmd == "gps fail") {
        gpsValid_ = false;
        gpsSearching_ = false;
        gpsSearchStartedMs_ = 0;
        Serial.println("[MOCK_GPS] invalid");
        return;
    }

    if (cmd.startsWith("battery ")) {
        int b = cmd.substring(8).toInt();
        if (b < 0) b = 0;
        if (b > 100) b = 100;
        batteryPct_ = (uint8_t)b;
        Serial.printf("[MOCK_PWR] battery %u%%\n", batteryPct_);
        return;
    }

    if (cmd == "beep test") {
        Serial.println("[MOCK_BUZZER] beep test");

        tone(PIN_BUZZER, 1000);
        delay(BUZZER_MEDIUM_BEEP_MS);
        noTone(PIN_BUZZER);

        delay(BUZZER_PATTERN_GAP_MS);

        tone(PIN_BUZZER, 1500);
        delay(BUZZER_MEDIUM_BEEP_MS);
        noTone(PIN_BUZZER);

        delay(BUZZER_PATTERN_GAP_MS);

        tone(PIN_BUZZER, 2000);
        delay(BUZZER_MEDIUM_BEEP_MS);
        noTone(PIN_BUZZER);

        return;
    }


        if (cmd == "log info") {
        DiveLogHeader header;

        if (!logStorage.loadLastDive(header)) {
            Serial.println("[LOG] no readable last dive log");
        }

        return;
    }

    if (cmd == "log clear") {
        logStorage.clearLastDive();
        return;
    }


    Serial.println("[CMD] Unknown command. Type help.");
}

void MockServices::loggerStartDive(uint16_t diveId) {
    Serial.printf("[MOCK_LOG] start dive #%u\n", diveId);
}

void MockServices::loggerSample(uint16_t sampleCount,
                                float depthM,
                                float tempC,
                                uint16_t ndlOrTts) {
    Serial.printf("[MOCK_LOG] sample %u depth=%.1f temp=%.1f ndl/tts=%u\r\n",
                  sampleCount, depthM, tempC, ndlOrTts);
}

void MockServices::loggerEndDive(uint16_t diveId,
                                 uint32_t durationSec,
                                 float maxDepthM) {
    Serial.printf("[MOCK_LOG] end dive #%u duration=%lus max=%.1fm\n",
                  diveId, durationSec, maxDepthM);
}
