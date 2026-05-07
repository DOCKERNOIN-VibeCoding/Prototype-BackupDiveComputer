#include "ui.h"
#include "config.h"

#include <Wire.h>
#include <string.h>
#include <time.h>

#ifdef WOKWI_SIMULATION
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#else
#include <SPI.h>
U8G2_ST7567_OS12864_F_4W_HW_SPI u8g2(U8G2_R0, PIN_LCD_CS, PIN_LCD_DC, PIN_LCD_RST);
#endif


// ------------------------------------------------------------
// Fallback constants
// config.h에 이미 있으면 그 값을 사용하고,
// 없으면 아래 기본값을 사용합니다.
// ------------------------------------------------------------

#ifndef BATTERY_FULL_THRESHOLD_PCT
#define BATTERY_FULL_THRESHOLD_PCT 100
#endif

#ifndef CHARGING_BLINK_INTERVAL_MS
#define CHARGING_BLINK_INTERVAL_MS 500UL
#endif

#ifndef CHARGING_FULL_BLINK_INTERVAL_MS
#define CHARGING_FULL_BLINK_INTERVAL_MS 1000UL
#endif


// ------------------------------------------------------------
// 공통 유틸 함수
// ------------------------------------------------------------

static void drawBattery(int x, int y, uint8_t pct) {
    u8g2.drawFrame(x, y, 16, 8);
    u8g2.drawBox(x + 16, y + 2, 2, 4);

    int fill = map(pct, 0, 100, 0, 14);

    if (fill < 0) {
        fill = 0;
    }

    if (fill > 14) {
        fill = 14;
    }

    if (fill > 0) {
        u8g2.drawBox(x + 1, y + 1, fill, 6);
    }
}


static void formatDateTimeFromEpoch(uint32_t epochSec,
                                    int16_t tzOffsetMin,
                                    char* buf,
                                    size_t len,
                                    bool slashFormat) {
    if (epochSec == 0) {
        snprintf(buf, len, "--");
        return;
    }

    time_t localTime = (time_t)epochSec + (time_t)tzOffsetMin * 60;
    struct tm tmv;

#if defined(ESP32)
    gmtime_r(&localTime, &tmv);
#else
    struct tm* p = gmtime(&localTime);

    if (p) {
        tmv = *p;
    } else {
        snprintf(buf, len, "--");
        return;
    }
#endif

    if (slashFormat) {
        snprintf(buf,
                 len,
                 "%04d/%02d/%02d %02d:%02d",
                 tmv.tm_year + 1900,
                 tmv.tm_mon + 1,
                 tmv.tm_mday,
                 tmv.tm_hour,
                 tmv.tm_min);
    } else {
        snprintf(buf,
                 len,
                 "%04d-%02d-%02d %02d:%02d",
                 tmv.tm_year + 1900,
                 tmv.tm_mon + 1,
                 tmv.tm_mday,
                 tmv.tm_hour,
                 tmv.tm_min);
    }
}

static void formatDateOnlyFromEpoch(uint32_t epochSec,
                                    int16_t tzOffsetMin,
                                    char* buf,
                                    size_t len) {
    if (epochSec == 0) {
        snprintf(buf, len, "--");
        return;
    }

    time_t localTime = (time_t)epochSec + (time_t)tzOffsetMin * 60;
    struct tm tmv;

#if defined(ESP32)
    gmtime_r(&localTime, &tmv);
#else
    struct tm* p = gmtime(&localTime);

    if (p) {
        tmv = *p;
    } else {
        snprintf(buf, len, "--");
        return;
    }
#endif

    snprintf(buf,
             len,
             "%04d-%02d-%02d",
             tmv.tm_year + 1900,
             tmv.tm_mon + 1,
             tmv.tm_mday);
}



static void formatSurfaceDuration(uint32_t sec, char* buf, size_t len) {
    uint32_t hours = sec / 3600UL;
    uint32_t minutes = (sec % 3600UL) / 60UL;

    if (hours < 24UL) {
        snprintf(buf,
                 len,
                 "%02lu:%02lu",
                 (unsigned long)hours,
                 (unsigned long)minutes);
    } else {
        uint32_t days = hours / 24UL;
        uint32_t remHours = hours % 24UL;

        snprintf(buf,
                 len,
                 "%lu days %02lu H",
                 (unsigned long)days,
                 (unsigned long)remHours);
    }
}


static void formatNoFly(uint32_t remainSec, char* buf, size_t len) {
    if (remainSec == 0) {
        snprintf(buf, len, "SAFE TO FLY");
        return;
    }

    uint32_t hours = remainSec / 3600UL;
    uint32_t minutes = (remainSec % 3600UL) / 60UL;

    snprintf(buf,
             len,
             "%02lu:%02lu",
             (unsigned long)hours,
             (unsigned long)minutes);
}


// 기존 다이빙 화면용 단순 시각 표시
// Dive/Safety/Deco 화면은 현재 함수 인자로 epoch를 받지 않으므로,
// 기존처럼 millis() 기반의 임시 시간을 표시합니다.
static void makeDateTimeText(char* buf, size_t len) {
    uint32_t totalMin = millis() / 60000UL;

    uint16_t year = 2026;
    uint8_t month = 4;
    uint8_t day = 30;
    uint8_t hour = 9;
    uint8_t minute = 0;

    minute += totalMin % 60;
    hour += totalMin / 60;

    while (minute >= 60) {
        minute -= 60;
        hour++;
    }

    while (hour >= 24) {
        hour -= 24;
        day++;
    }

    snprintf(buf,
             len,
             "%04u/%02u/%02u %02u:%02u",
             year,
             month,
             day,
             hour,
             minute);
}


// ------------------------------------------------------------
// Top Bar
// ------------------------------------------------------------

// Dive / Safety / Deco / PostDive 등 기존 화면용 Top Bar
static void drawTopBar(uint8_t batteryPct) {
    char timeText[24];
    makeDateTimeText(timeText, sizeof(timeText));

    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(1, 7, timeText);

    drawBattery(106, 0, batteryPct);

    u8g2.drawHLine(0, 9, 128);
}


// Surface 전용 Top Bar
// 현재 날짜/시간 + GPS 상태 + 배터리/충전 상태 표시
static void drawTopBarSurface(uint32_t currentEpochSec,
                              int16_t tzOffsetMin,
                              uint8_t batteryPct,
                              bool gpsValid,
                              bool charging,
                              bool chargeFull) {
    char timeText[24];

    formatDateTimeFromEpoch(currentEpochSec,
                            tzOffsetMin,
                            timeText,
                            sizeof(timeText),
                            true);

    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(1, 7, timeText);

    // GPS 표시
    // G : GPS Fix 성공
    // - : GPS 미수신
    u8g2.drawStr(88, 7, gpsValid ? "G" : "-");

    const int batX = 106;
    const int batY = 0;

    if (!charging) {
        // 일반 Surface 상태
        drawBattery(batX, batY, batteryPct);
    } else {
        if (chargeFull) {
            // 충전기 위 + 완충:
            // FULL과 배터리 아이콘을 번갈아 표시
            bool showFull =
                ((millis() / CHARGING_FULL_BLINK_INTERVAL_MS) % 2UL) == 0;

            if (showFull) {
                u8g2.setFont(u8g2_font_5x7_tr);
                u8g2.drawStr(104, 7, "FULL");
            } else {
                drawBattery(batX, batY, batteryPct);
            }
        } else {
            // 충전기 위 + 충전 중:
            // 배터리 아이콘 깜빡임
            bool showBattery =
                ((millis() / CHARGING_BLINK_INTERVAL_MS) % 2UL) == 0;

            if (showBattery) {
                drawBattery(batX, batY, batteryPct);
            }
        }
    }

    u8g2.drawHLine(0, 9, 128);
}


// ------------------------------------------------------------
// Dive Layout Helper
// ------------------------------------------------------------

static void drawDiveLayout(uint8_t batteryPct) {
    drawTopBar(batteryPct);

    // 좌측 / 우측 구분선
    u8g2.drawVLine(65, 10, 54);

    // 좌측 하단 구분선
    u8g2.drawHLine(0, 39, 65);
}


static void drawDepthBox(float depthM) {
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(1, 17, "DEPTH");

    char depthBuf[12];
    snprintf(depthBuf, sizeof(depthBuf), "%.1f", depthM);

    u8g2.setFont(u8g2_font_logisoso18_tn);
    int depthW = u8g2.getStrWidth(depthBuf);

    u8g2.setFont(u8g2_font_5x7_tr);
    int mW = u8g2.getStrWidth("m");

    int x = 63 - depthW - mW - 2;

    if (x < 1) {
        x = 1;
    }

    u8g2.setFont(u8g2_font_logisoso18_tn);
    u8g2.drawStr(x, 37, depthBuf);

    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(x + depthW + 2, 37, "m");
}


static void drawRightNormalInfo(uint32_t diveTimeSec,
                                float maxDepthM,
                                float tempC,
                                uint16_t ttsMin,
                                bool paused) {
    u8g2.setFont(u8g2_font_5x7_tr);

    char buf[24];

    snprintf(buf,
             sizeof(buf),
             "TIME %lu:%02lu",
             (unsigned long)(diveTimeSec / 60UL),
             (unsigned long)(diveTimeSec % 60UL));
    u8g2.drawStr(68, 19, buf);

    snprintf(buf, sizeof(buf), "MAX  %.1fm", maxDepthM);
    u8g2.drawStr(68, 29, buf);

    if (!paused) {
        snprintf(buf, sizeof(buf), "TMP  %.0fC", tempC);
        u8g2.drawStr(68, 39, buf);

        snprintf(buf, sizeof(buf), "TTS  %umin", ttsMin);
        u8g2.drawStr(68, 49, buf);
    } else {
        snprintf(buf, sizeof(buf), "TTS  %umin", ttsMin);
        u8g2.drawStr(68, 39, buf);

        u8g2.drawStr(68, 52, "S.STOP");
        u8g2.drawStr(68, 62, "PAUSED!");
    }
}


static void drawNDLBox(uint16_t ndlMin) {
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(1, 49, "NDL");

    char ndlBuf[8];
    snprintf(ndlBuf, sizeof(ndlBuf), "%u", ndlMin);

    u8g2.setFont(u8g2_font_7x14B_tr);
    int ndlW = u8g2.getStrWidth(ndlBuf);

    u8g2.setFont(u8g2_font_5x7_tr);
    int minW = u8g2.getStrWidth("min");

    int x = 63 - ndlW - minW - 3;

    if (x < 1) {
        x = 1;
    }

    u8g2.setFont(u8g2_font_7x14B_tr);
    u8g2.drawStr(x, 62, ndlBuf);

    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(x + ndlW + 3, 62, "min");
}


static void drawSafetyBox(uint32_t remainSec) {
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(1, 48, "S.STOP");

    uint32_t mm = remainSec / 60UL;
    uint32_t ss = remainSec % 60UL;

    char timeBuf[12];
    snprintf(timeBuf,
             sizeof(timeBuf),
             "%lu:%02lu",
             (unsigned long)mm,
             (unsigned long)ss);

    u8g2.setFont(u8g2_font_7x14B_tr);

    int w = u8g2.getStrWidth(timeBuf);
    int x = 63 - w;

    if (x < 1) {
        x = 1;
    }

    u8g2.drawStr(x, 63, timeBuf);
}


static void drawSkippedBox() {
    u8g2.setFont(u8g2_font_5x7_tr);

    if ((millis() / 500UL) % 2UL) {
        u8g2.drawStr(1, 49, "S.STOP");
        u8g2.drawStr(1, 61, "SKIPPED!");
    } else {
        u8g2.drawStr(1, 49, "S.STOP");
    }
}


// ------------------------------------------------------------
// 상승속도 막대그래프
// 위험영역 Dot Pattern은 항상 표시됩니다.
// ------------------------------------------------------------

void uiDrawAscBar(float ascentRateMpm) {
    const int BAR_X = 120;
    const int BAR_Y = 10;
    const int BAR_W = 8;
    const int BAR_H = 54;

    const float MIN_RATE = ASC_GRAPH_MIN_RATE_MPM;   // 0m/min
    const float MAX_RATE = ASC_GRAPH_MAX_RATE_MPM;   // 18m/min
    const float DOT_RATE = ASC_DOT_START_RATE_MPM;   // 10m/min

    u8g2.setDrawColor(1);

    // 외곽
    u8g2.drawFrame(BAR_X, BAR_Y, BAR_W, BAR_H);

    const int innerX = BAR_X + 1;
    const int innerY = BAR_Y + 1;
    const int innerW = BAR_W - 2;
    const int innerH = BAR_H - 2;

    const int bottomY = innerY + innerH - 1;

    // 10m/min 위치 계산
    int dotStartFillH = (int)((DOT_RATE / MAX_RATE) * innerH + 0.5f);

    if (dotStartFillH < 1) {
        dotStartFillH = 1;
    }

    if (dotStartFillH > innerH) {
        dotStartFillH = innerH;
    }

    int dotStartY = bottomY - dotStartFillH + 1;

    // 1. 위험영역 Dot Pattern 항상 표시
    // 하강 중이거나 상승속도 0이어도 표시됩니다.
    for (int y = innerY; y <= dotStartY; y++) {
        for (int x = innerX; x < innerX + innerW; x++) {
            if (((x + y) & 1) == 0) {
                u8g2.drawPixel(x, y);
            }
        }
    }

    // 2. 10m/min 경계선
    u8g2.drawHLine(innerX, dotStartY, innerW);

    // 3. 현재 상승속도
    float rate = ascentRateMpm;

    if (rate < MIN_RATE) {
        rate = 0.0f;
    }

    if (rate > MAX_RATE) {
        rate = MAX_RATE;
    }

    // 상승하지 않는 경우:
    // 현재 막대는 표시하지 않지만, 위험영역 Dot Pattern은 유지
    if (rate <= 0.0f) {
        return;
    }

    int fillH = (int)((rate / MAX_RATE) * innerH + 0.5f);

    if (fillH < 1) {
        fillH = 1;
    }

    if (fillH > innerH) {
        fillH = innerH;
    }

    int fillTopY = bottomY - fillH + 1;

    if (rate < DOT_RATE) {
        // 0~10m/min 미만:
        // 안전영역 안에서 일반 막대 표시
        u8g2.drawBox(innerX, fillTopY, innerW, fillH);
    } else {
        // 10m/min 이상:
        // 안전영역은 꽉 채우고,
        // 위험영역 안에서는 Dot Pattern이 보이도록 중앙 마커만 표시

        int safeTopY = dotStartY + 1;
        int safeH = bottomY - safeTopY + 1;

        if (safeH > 0) {
            u8g2.drawBox(innerX, safeTopY, innerW, safeH);
        }

        const int markerW = 2;
        const int markerX = innerX + (innerW - markerW) / 2;

        int dangerTopY = fillTopY;
        int dangerBottomY = dotStartY;

        if (dangerTopY < innerY) {
            dangerTopY = innerY;
        }

        if (dangerBottomY >= dangerTopY) {
            u8g2.drawBox(markerX,
                         dangerTopY,
                         markerW,
                         dangerBottomY - dangerTopY + 1);
        }

        // 현재 속도 위치 표시선
        u8g2.drawHLine(innerX, fillTopY, innerW);
    }
}


// ------------------------------------------------------------
// 초기화 / Splash
// ------------------------------------------------------------

void uiBegin() {
#ifdef WOKWI_SIMULATION
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    u8g2.begin();
#else
    SPI.begin(PIN_LCD_SCLK, -1, PIN_LCD_MOSI, PIN_LCD_CS);
    u8g2.begin();

    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);
#endif

    u8g2.setContrast(180);
}


void uiSplash() {
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_helvR10_tr);

    const char* t1 = "BACKUP";
    const char* t2 = "DIVE COMPUTER";

    u8g2.drawStr((128 - u8g2.getStrWidth(t1)) / 2, 18, t1);
    u8g2.drawStr((128 - u8g2.getStrWidth(t2)) / 2, 36, t2);

    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(26, 56, FW_VERSION);

    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.sendBuffer();
}


// ------------------------------------------------------------
// Surface 화면
// ------------------------------------------------------------

void uiDrawSurface(uint32_t currentEpochSec,
                   int16_t tzOffsetMin,
                   uint8_t batteryPct,
                   bool gpsValid,
                   bool charging,
                   bool chargeFull,
                   uint32_t lastDiveStartEpochSec,
                   float lastMaxDepthM,
                   float lastMinTempC,
                   uint32_t surfaceIntervalSec,
                   uint32_t noFlyRemainSec) {

    u8g2.clearBuffer();

    drawTopBarSurface(currentEpochSec,
                      tzOffsetMin,
                      batteryPct,
                      gpsValid,
                      charging,
                      chargeFull);

    char lastDiveText[24];
    char surfaceText[24];
    char noFlyText[24];
    char value[24];

    formatDateOnlyFromEpoch(lastDiveStartEpochSec,
                            tzOffsetMin,
                            lastDiveText,
                            sizeof(lastDiveText));


    formatSurfaceDuration(surfaceIntervalSec,
                          surfaceText,
                          sizeof(surfaceText));

    formatNoFly(noFlyRemainSec,
                noFlyText,
                sizeof(noFlyText));

    // ------------------------------------------------------------
    // Surface body
    // 요청사항 반영:
    // - 폰트 크기 확대
    // - 줄간격 11px
    //
    // u8g2에는 정확한 7x9 이름의 기본 폰트가 없을 수 있으므로,
    // 실제 가독성과 화면 폭을 고려해 6x10 폰트를 사용합니다.
    // ------------------------------------------------------------
    u8g2.setFont(u8g2_font_6x10_tf);

    const int labelX = 1;
    const int valueX = 61;

    // 줄간격 11px
    const int y1 = 20;
    const int y2 = 31;
    const int y3 = 42;
    const int y4 = 53;
    const int y5 = 64;

    // ------------------------------------------------------------
    // 1. LAST DIVE
    // 128px 폭에서 "LAST DIVE 2025-01-01 09:00" 전체는
    // 큰 폰트로는 한 줄에 들어가지 않으므로,
    // 표시 공간이 부족하면 "--" 또는 짧은 시간만 표시하는 방식이 필요합니다.
    // 우선 요청 형식을 유지하되, 오른쪽이 잘릴 수 있습니다.
    // ------------------------------------------------------------
    u8g2.drawStr(labelX, y1, "LAST");


    if (lastDiveStartEpochSec == 0) {
      u8g2.drawStr(valueX, y1, "--");
    } else {    
       u8g2.drawStr(valueX, y1, lastDiveText);
    }


    // ------------------------------------------------------------
    // 2. LAST MAX
    // ------------------------------------------------------------
    u8g2.drawStr(labelX, y2, "MAX");

    snprintf(value, sizeof(value), "%.1fm", lastMaxDepthM);
    u8g2.drawStr(valueX, y2, value);

    // ------------------------------------------------------------
    // 3. LAST TMP
    // ------------------------------------------------------------
    u8g2.drawStr(labelX, y3, "TMP");

    if (lastMinTempC <= 0.0f) {
        snprintf(value, sizeof(value), "--C");
    } else {
        snprintf(value, sizeof(value), "%.0fC", lastMinTempC);
    }

    u8g2.drawStr(valueX, y3, value);

    // ------------------------------------------------------------
    // 4. SURFACE
    // ------------------------------------------------------------
    u8g2.drawStr(labelX, y4, "SURFACE");
    u8g2.drawStr(valueX, y4, surfaceText);

    // ------------------------------------------------------------
    // 5. N-FLY
    // ------------------------------------------------------------
    u8g2.drawStr(labelX, y5, "N-FLY");
    u8g2.drawStr(valueX, y5, noFlyText);

    u8g2.sendBuffer();
}


// ------------------------------------------------------------
// Dive Normal
// ------------------------------------------------------------

void uiDrawDiveNormal(float depthM,
                      uint16_t ndlMin,
                      uint16_t ttsMin,
                      uint32_t diveTimeSec,
                      float maxDepthM,
                      float tempC,
                      float ascentRateMpm,
                      uint8_t batteryPct) {
    u8g2.clearBuffer();

    drawDiveLayout(batteryPct);

    drawDepthBox(depthM);
    drawNDLBox(ndlMin);

    drawRightNormalInfo(diveTimeSec,
                        maxDepthM,
                        tempC,
                        ttsMin,
                        false);

    uiDrawAscBar(ascentRateMpm);

    u8g2.sendBuffer();
}


// ------------------------------------------------------------
// Safety Stop
// ------------------------------------------------------------

void uiDrawDiveSafety(float depthM,
                      uint32_t remainSec,
                      bool paused,
                      uint16_t ttsMin,
                      uint32_t diveTimeSec,
                      float maxDepthM,
                      float tempC,
                      float ascentRateMpm,
                      uint8_t batteryPct) {
    u8g2.clearBuffer();

    drawDiveLayout(batteryPct);

    drawDepthBox(depthM);
    drawSafetyBox(remainSec);

    drawRightNormalInfo(diveTimeSec,
                        maxDepthM,
                        tempC,
                        ttsMin,
                        paused);

    uiDrawAscBar(ascentRateMpm);

    u8g2.sendBuffer();
}


// ------------------------------------------------------------
// Safety Stop Skipped
// ------------------------------------------------------------

void uiDrawDiveSkipped(float depthM,
                       uint16_t ttsMin,
                       uint32_t diveTimeSec,
                       float maxDepthM,
                       float tempC,
                       float ascentRateMpm,
                       uint8_t batteryPct) {
    u8g2.clearBuffer();

    drawDiveLayout(batteryPct);

    drawDepthBox(depthM);
    drawSkippedBox();

    drawRightNormalInfo(diveTimeSec,
                        maxDepthM,
                        tempC,
                        ttsMin,
                        false);

    uiDrawAscBar(ascentRateMpm);

    u8g2.sendBuffer();
}


// ------------------------------------------------------------
// Deco
// ------------------------------------------------------------

void uiDrawDiveDeco(float depthM,
                    uint8_t stopDepthM,
                    uint32_t stopRemainSec,
                    uint16_t ttsMin,
                    uint32_t diveTimeSec,
                    float maxDepthM,
                    float tempC,
                    float ascentRateMpm,
                    uint8_t batteryPct,
                    bool ceilingGtMaxStop,
                    float ceilingDepthM) {

    u8g2.clearBuffer();

    drawDiveLayout(batteryPct);

    drawDepthBox(depthM);

        if (ceilingGtMaxStop) {
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.drawStr(1, 49, "CEIL >18m");

        u8g2.setFont(u8g2_font_5x7_tr);

        char ceilBuf[16];
        snprintf(ceilBuf, sizeof(ceilBuf), "CEIL %.1fm", ceilingDepthM);
        u8g2.drawStr(68, 49, ceilBuf);

        bool blinkOn = ((millis() / 500UL) % 2UL) == 0;

        if (blinkOn) {
            u8g2.drawStr(68, 62, "HOLD DEPTH");
        }

        uiDrawAscBar(ascentRateMpm);
        u8g2.sendBuffer();
        return;
    }


    // 좌측 하단: DECO.STOP 6m / mm:ss
    u8g2.setFont(u8g2_font_5x7_tr);

    char stopLabel[18];
    snprintf(stopLabel, sizeof(stopLabel), "DECO.STOP %um", stopDepthM);
    u8g2.drawStr(1, 48, stopLabel);

    uint32_t mm = stopRemainSec / 60UL;
    uint32_t ss = stopRemainSec % 60UL;

    char stopTime[12];
    snprintf(stopTime,
             sizeof(stopTime),
             "%lu:%02lu",
             (unsigned long)mm,
             (unsigned long)ss);

    u8g2.setFont(u8g2_font_7x14B_tr);

    int w = u8g2.getStrWidth(stopTime);
    int x = 63 - w;

    if (x < 1) {
        x = 1;
    }

    u8g2.drawStr(x, 63, stopTime);

    // 우측 보조정보
    u8g2.setFont(u8g2_font_5x7_tr);

    char buf[24];

    snprintf(buf,
             sizeof(buf),
             "TIME %lu:%02lu",
             (unsigned long)(diveTimeSec / 60UL),
             (unsigned long)(diveTimeSec % 60UL));
    u8g2.drawStr(68, 19, buf);

    snprintf(buf, sizeof(buf), "MAX  %.1fm", maxDepthM);
    u8g2.drawStr(68, 29, buf);

    snprintf(buf, sizeof(buf), "TMP  %.0fC", tempC);
    u8g2.drawStr(68, 39, buf);

    snprintf(buf, sizeof(buf), "TTS  %umin", ttsMin);
    u8g2.drawStr(68, 49, buf);

    // 행동 지시
    const char* actionText = "HOLD";
    int8_t actionArrow = 0; // 1 = up, -1 = down, 0 = none

    if (stopDepthM > 0) {
        if (depthM > (float)stopDepthM + DECO_STOP_WINDOW_M) {
            actionText = "ASCEND";
            actionArrow = 1;
        } else if (depthM < (float)stopDepthM - DECO_STOP_WINDOW_M) {
            actionText = "DESCEND";
            actionArrow = -1;
        } else {
            actionText = "HOLD";
            actionArrow = 0;
        }
    }

    bool blinkOn = ((millis() / 500UL) % 2UL) == 0;

    if (blinkOn) {
        u8g2.setFont(u8g2_font_5x7_tr);
        u8g2.drawStr(68, 62, actionText);

        // Draw arrow icon manually.
        // This avoids font/Unicode compatibility issues.
        if (actionArrow > 0) {
            // Up triangle ▲
            u8g2.drawTriangle(114, 55, 109, 62, 119, 62);
        } else if (actionArrow < 0) {
            // Down triangle ▼
            u8g2.drawTriangle(109, 55, 119, 55, 114, 62);
        }
    }

    uiDrawAscBar(ascentRateMpm);

    u8g2.sendBuffer();
}


// ------------------------------------------------------------
// Post Dive
// ------------------------------------------------------------

void uiDrawPostDive(uint16_t diveCount,
                    uint32_t diveTimeSec,
                    float maxDepthM,
                    float minTempC,
                    uint32_t noFlyRemainSec,
                    bool gpsValid,
                    uint8_t batteryPct) {
    u8g2.clearBuffer();

    drawTopBar(batteryPct);

    u8g2.setFont(u8g2_font_5x7_tr);

    char buf[32];

    u8g2.drawStr(2, 20, "POST DIVE");

    snprintf(buf, sizeof(buf), "DIVE #%u COMPLETE", diveCount);
    u8g2.drawStr(2, 31, buf);

    snprintf(buf,
             sizeof(buf),
             "TIME %lumin",
             (unsigned long)(diveTimeSec / 60UL));
    u8g2.drawStr(2, 42, buf);

    snprintf(buf, sizeof(buf), "MAX %.1fm TMP %.0fC", maxDepthM, minTempC);
    u8g2.drawStr(2, 53, buf);

    snprintf(buf, sizeof(buf), "GPS %s", gpsValid ? "OK" : "--");
    u8g2.drawStr(92, 63, buf);

    u8g2.sendBuffer();
}


// ------------------------------------------------------------
// Charging
// 현재 app.cpp에서는 Qi 충전 중에도 uiDrawSurface()를 사용하도록
// 변경하는 것을 권장합니다.
// 단, 기존 호출 호환성을 위해 함수는 유지합니다.
// ------------------------------------------------------------

void uiDrawCharging(uint8_t batteryPct,
                    bool bleConnected,
                    uint8_t gfLow,
                    uint8_t gfHigh) {
    u8g2.clearBuffer();

    drawTopBar(batteryPct);

    u8g2.setFont(u8g2_font_5x7_tr);

    char buf[32];

    u8g2.drawStr(2, 20, "CHARGING");

    snprintf(buf, sizeof(buf), "BATTERY %u%%", batteryPct);
    u8g2.drawStr(2, 32, buf);

    snprintf(buf, sizeof(buf), "BLE %s", bleConnected ? "CONNECTED" : "READY");
    u8g2.drawStr(2, 44, buf);

    snprintf(buf, sizeof(buf), "GF %u/%u", gfLow, gfHigh);
    u8g2.drawStr(2, 56, buf);

    u8g2.sendBuffer();
}


// ------------------------------------------------------------
// Battery Low Popup
// ------------------------------------------------------------

void uiDrawBatteryLowPopup(uint8_t batteryPct) {
    const int x = 12;
    const int y = 22;
    const int w = 104;
    const int h = 24;

    // 흰색 팝업 박스
    u8g2.setDrawColor(1);
    u8g2.drawBox(x, y, w, h);

    // 검은색 테두리/글자
    u8g2.setDrawColor(0);
    u8g2.drawFrame(x + 1, y + 1, w - 2, h - 2);

    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(x + 18, y + 10, "BATTERY LOW");

    char buf[24];
    snprintf(buf, sizeof(buf), "BAT %u%%", batteryPct);
    u8g2.drawStr(x + 34, y + 21, buf);

    // 이후 그리기를 위해 색상 복구
    u8g2.setDrawColor(1);

    u8g2.sendBuffer();
}
