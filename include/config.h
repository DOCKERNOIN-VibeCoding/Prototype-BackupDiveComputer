#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
// Build
// ============================================================

#define FW_NAME     "BackupDiveComputer"
#define FW_VERSION  "v1.3-dev"

// ============================================================
// Pins
// ============================================================

// Wokwi SSD1306 I2C
#define PIN_I2C_SDA      5
#define PIN_I2C_SCL      6

// Buzzer
#define PIN_BUZZER       4

// Real hardware placeholders
#define PIN_LCD_SCLK    12
#define PIN_LCD_MOSI    11
#define PIN_LCD_CS      10
#define PIN_LCD_DC       8
#define PIN_LCD_RST      9
#define PIN_LCD_BL       7

#define PIN_GPS_RX      16
#define PIN_GPS_TX      15
#define GPS_BAUD    115200

#define PIN_QI_DETECT    2

// ============================================================
// Display
// ============================================================

#define LCD_WIDTH   128
#define LCD_HEIGHT   64

// ============================================================
// Dive thresholds
// ============================================================

#define SURFACE_PRESSURE_BAR       1.01325f

#define DIVE_START_DEPTH_M         1.2f
#define DIVE_END_DEPTH_M           0.5f
#define DIVE_END_SURFACE_SEC       60
#define DIVE_START_CONFIRM_COUNT   3

#define SENSOR_SAMPLE_INTERVAL_MS  250
#define UI_UPDATE_INTERVAL_MS      500
#define LOG_RECORD_INTERVAL_S      4

// ============================================================
// Safety stop
// ============================================================

#define SAFETY_STOP_TRIGGER_DEPTH_M  10.0f
#define SAFETY_STOP_MIN_DEPTH_M       3.0f
#define SAFETY_STOP_MAX_DEPTH_M       6.0f
#define SAFETY_STOP_DURATION_S      180
#define SAFETY_STOP_PAUSE_MAX_S      60
#define SAFETY_STOP_SKIP_DEPTH_M      8.0f
#define SAFETY_STOP_REEVAL_SEC       30
#define SAFETY_STOP_SKIPPED_DISPLAY_SEC 30UL

// ============================================================
// Ascent rate / ASC bar graph
// ============================================================

// 상승속도 측정 주기
// 250ms = 1초에 4회 측정
#define ASC_RATE_SAMPLE_INTERVAL_MS     250

// 상승속도 그래프 표시 범위
#define ASC_GRAPH_MIN_RATE_MPM          0.0f
#define ASC_GRAPH_MAX_RATE_MPM          18.0f

// Dot Pattern 시작 속도
// 반드시 10m/min부터 위험영역 Dot Pattern에 진입
#define ASC_DOT_START_RATE_MPM          10.0f

// 알람 기준
#define ASC_WARNING_RATE_MPM            10.0f
#define ASC_DANGER_RATE_MPM             18.0f

// 기존 코드 호환용
#define ASC_BAR_MAX_RATE_MPM            ASC_GRAPH_MAX_RATE_MPM


// ------------------------------------------------------------
// Gradient Factor 기본 설정
// 향후 BLE에서도 이 값을 기준으로 GF 변경값을 검증
// ------------------------------------------------------------
#define DEFAULT_GF_LOW   40
#define DEFAULT_GF_HIGH  85

#define GF_LOW_MIN       10
#define GF_LOW_MAX       90
#define GF_HIGH_MIN      50
#define GF_HIGH_MAX      100

// GF Low는 GF High보다 작아야 함
#define GF_REQUIRE_LOW_LESS_THAN_HIGH 1


// ------------------------------------------------------------
// Battery 기본 설정
// ------------------------------------------------------------
#define DEFAULT_BATTERY_PCT 95

// 배터리 저전압 경고 기준
#define BATTERY_LOW_THRESHOLD_PCT 10

// BATTERY LOW 팝업 표시 주기: 2분
#define BATTERY_LOW_POPUP_INTERVAL_MS 120000UL

// BATTERY LOW 팝업 표시 시간: 2초
#define BATTERY_LOW_POPUP_DURATION_MS 2000UL

// ------------------------------------------------------------
// Charging display
// ------------------------------------------------------------
#define BATTERY_FULL_THRESHOLD_PCT 100

// 충전 중 배터리 아이콘 깜빡임 주기
#define CHARGING_BLINK_INTERVAL_MS 500UL

// 완충 시 배터리 아이콘 / FULL 교대 표시 주기
#define CHARGING_FULL_BLINK_INTERVAL_MS 1000UL



// ============================================================
// Post dive / no fly
// ============================================================

#define POST_DIVE_DISPLAY_MS       180000UL
#define POST_DIVE_INFO_DURATION_S  1800UL

// ============================================================
// No-Fly calculation
// ============================================================

#define NO_FLY_CABIN_PRESSURE_BAR       0.757f
#define NO_FLY_MIN_HOURS                12UL
#define NO_FLY_DECO_VIOLATION_HOURS     48UL
#define NO_FLY_MAX_HOURS                72UL

// ============================================================
// Simulation
// ============================================================

#define SIM_SINGLE_DIVE_DURATION_S 1040UL
#define SIM_SURFACE_INTERVAL_1_S    300UL
#define SIM_SURFACE_INTERVAL_2_S    180UL

// ============================================================
// Alarm frequencies
// ============================================================

#define ALARM_FREQ_NDL_WARN        1000
#define ALARM_FREQ_DECO_ENTER      1500
#define ALARM_FREQ_CEILING_VIOL    3000
#define ALARM_FREQ_ASCENT_WARN     2000
#define ALARM_FREQ_SAFETY_DONE     1500

#endif

