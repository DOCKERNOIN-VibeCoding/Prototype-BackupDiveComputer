#include <Arduino.h>
#include "app.h"

void setup() {
    Serial.begin(115200);

    // Wokwi / ESP32-S3 USB Serial 환경에서 while(!Serial) 무한대기 방지
    delay(500);

    Serial.println();
    Serial.println("[BOOT] Serial started");

    app.begin();
}

void loop() {
    app.update();

    // 너무 바쁘게 돌지 않도록 약간의 여유
    delay(10);
}
