
---
```markdown
# HARDWARE_PROTOTYPE.md

Backup Dive Computer v1.3 실제 프로토타입 하드웨어 사양 문서입니다.

이 문서는 Wokwi 시뮬레이션용 하드웨어와 실제 제작 예정 하드웨어의 차이를 명확히 정리하고, ESP32-S3 기준 핀맵, 주요 부품, 전원 구성, 배터리/충전 관련 주의사항을 기록하기 위한 문서입니다.

> 주의: 본 하드웨어는 개발 및 프로토타입 검증용입니다. 실제 다이빙 안전장비로 사용하기 전에는 방수, 압력, 배터리 안전, 센서 정확도, 펌웨어 안정성, 장시간 운용 테스트가 필요합니다.

---

## 1. Prototype Target

본 프로토타입은 다음 목적을 가집니다.

- ESP32-S3 기반 백업 다이브 컴퓨터 구현
- 수심/온도 측정
- Bühlmann 감압 계산 및 UI 표시
- 배터리 기반 저전력 운용
- 수면 GPS 위치 기록
- 무선충전 기반 밀폐형 하우징 운용
- 부저 또는 피에조 디스크를 이용한 경고음 출력
- 액션캠 다이빙 하우징을 활용한 실험적 방수 구조 검토

---

## 2. Main Components

| 구분 | 부품 | 상태 | 비고 |
|---|---|---|---|
| MCU | ESP32-S3 N16R8 Dev Board | 사용 예정 | 16MB Flash / 8MB PSRAM 계열 |
| Display | 128x64 SPI LCD, ST7567A | 사용 예정 | AliExpress item `1005004706328499` |
| Pressure Sensor | MS5837-30BA | 사용 예정 | I2C, 0~30 bar, 수심/온도 측정 |
| GPS | 드론용 GPS 모듈 | 사용 예정 | UART GPS, SCL/SDA는 Compass용으로 우선 미사용 |
| Buzzer | 12mm Piezo Disc | 사용 예정 | 액션캠 하우징 벽면 부착 예정 |
| Battery | Protected 16340 Li-ion | 검토/사용 예정 | 보호회로 내장 제품 권장 |
| Charging | Qi Wireless Charging Receiver + Li-ion Charger | 검토 중 | 회로 구성 확인 필요 |
| Enclosure | Action Camera Diving Housing | 검토/사용 예정 | 압력센서 포트 및 음향 전달 방식 검증 필요 |

---

## 3. Important Difference: Wokwi vs Real Prototype

현재 Wokwi 시뮬레이션에서는 SSD1306 I2C OLED를 사용할 수 있습니다.

하지만 실제 프로토타입에서는 다음 LCD를 사용할 예정입니다.

```text
12864 SPI LCD Module
128x64
ST7567A COG Graphic Display
SPI interface
```

따라서 실제 하드웨어에서는 OLED용 I2C 핀맵이 아니라 ST7567A SPI LCD 핀맵을 사용합니다.

### Wokwi Simulation Display

```text
SSD1306 I2C OLED
SDA = GPIO5
SCL = GPIO6
```

### Real Prototype Display

```text
ST7567A SPI LCD
SCLK = GPIO12
MOSI = GPIO11
CS   = GPIO10
DC   = GPIO8
RST  = GPIO9
BL   = GPIO7
```

---

## 4. Final Recommended ESP32-S3 Pin Map

실제 프로토타입 기준 권장 핀맵입니다.

| GPIO | 기능 | 연결 대상 | 비고 |
|---:|---|---|---|
| GPIO1 | Battery ADC | 배터리 전압 분압 입력 | 선택 기능 |
| GPIO2 | Qi Detect | 무선충전 5V 감지 분압 입력 | 선택 기능, 5V 직접 입력 금지 |
| GPIO4 | Buzzer | Piezo Disc | 100~330Ω 직렬저항 권장 |
| GPIO5 | I2C SDA | MS5837 SDA | 압력센서 |
| GPIO6 | I2C SCL | MS5837 SCL | 압력센서 |
| GPIO7 | LCD Backlight | LCD BL/LED | PWM 제어 가능 |
| GPIO8 | LCD DC | LCD DC/A0 | ST7567A SPI LCD |
| GPIO9 | LCD Reset | LCD RST/RES | ST7567A SPI LCD |
| GPIO10 | LCD CS | LCD CS | ST7567A SPI LCD |
| GPIO11 | LCD MOSI | LCD SDA/DIN | ST7567A SPI LCD |
| GPIO12 | LCD SCLK | LCD SCL/CLK | ST7567A SPI LCD |
| GPIO14 | GPS Power Enable | Load Switch EN | 선택 기능 |
| GPIO15 | GPS TX | GPS RX로 연결 | ESP32 TX |
| GPIO16 | GPS RX | GPS TX로 연결 | ESP32 RX |

---

## 5. Display: ST7567A 128x64 SPI LCD

사용 예정 스크린:

```text
12864 SPI LCD Module
128X64
ST7567A COG Graphic Display
AliExpress item: 1005004706328499
```

### LCD Wiring

| LCD Pin | Meaning | ESP32-S3 |
|---|---|---:|
| GND | Ground | GND |
| VCC | Power | 3V3 |
| SCL / CLK | SPI Clock | GPIO12 |
| SDA / DIN | SPI MOSI | GPIO11 |
| CS | Chip Select | GPIO10 |
| DC / A0 | Data/Command | GPIO8 |
| RES / RST | Reset | GPIO9 |
| BL / LED | Backlight | GPIO7 또는 3V3 |

권장 연결:

```text
LCD GND  → ESP32 GND
LCD VCC  → ESP32 3V3
LCD SCL  → ESP32 GPIO12
LCD SDA  → ESP32 GPIO11
LCD CS   → ESP32 GPIO10
LCD DC   → ESP32 GPIO8
LCD RES  → ESP32 GPIO9
LCD BL   → ESP32 GPIO7
```

### Backlight Policy

배터리 절약을 위해 Backlight는 GPIO로 제어하는 것을 권장합니다.

```text
GPIO7 HIGH → Backlight ON
GPIO7 LOW  → Backlight OFF
```

PWM 제어를 사용하면 밝기 조절도 가능합니다.

---

## 6. Pressure Sensor: MS5837-30BA

MS5837-30BA는 수심 측정용 핵심 센서입니다.

### Sensor Wiring

```text
MS5837 VCC → ESP32 3V3
MS5837 GND → ESP32 GND
MS5837 SDA → ESP32 GPIO5
MS5837 SCL → ESP32 GPIO6
```

### Notes

- I2C 방식 센서입니다.
- 3.3V 전원 사용을 권장합니다.
- 수압을 직접 받아야 하므로 센서 캡슐부 또는 압력 포트 설계가 중요합니다.
- 일반 breakout board 전체를 물에 노출하면 안 됩니다.
- 하우징 내부 회로는 방수되어야 하며, 센서부만 수압을 받을 수 있도록 별도 실링 구조가 필요합니다.
- I2C 주소는 실제 모듈 수령 후 I2C scanner로 확인해야 합니다.

---

## 7. I2C Bus Sharing Note

I2C는 여러 장치를 같은 SDA/SCL 라인에 병렬로 연결할 수 있는 버스 방식입니다.

예를 들어 Wokwi 또는 I2C OLED를 사용하는 경우 다음과 같이 연결할 수 있습니다.

```text
ESP32 GPIO5 SDA ─┬─ OLED SDA
                 └─ MS5837 SDA

ESP32 GPIO6 SCL ─┬─ OLED SCL
                 └─ MS5837 SCL
```

이것은 정상적인 I2C 연결 방식입니다.

단, 조건이 있습니다.

1. 각 장치의 I2C 주소가 달라야 합니다.
2. 모든 장치의 I2C 전압 레벨이 3.3V여야 합니다.
3. 풀업 저항이 과도하게 중복되지 않아야 합니다.
4. 배선 길이가 너무 길지 않아야 합니다.

다만 실제 프로토타입에서는 ST7567A SPI LCD를 사용하므로, I2C 버스는 우선 MS5837 압력센서 전용으로 사용합니다.

---

## 8. GPS Module

드론용 GPS 모듈을 사용할 예정입니다.

GPS 모듈 배선 예:

```text
VCC: 5V
GND: GND
TX: GPS TX → ESP32 RX
RX: GPS RX ← ESP32 TX
SCL: Compass I2C SCL
SDA: Compass I2C SDA
```

### GPS Wiring

| GPS Pin | ESP32-S3 | 비고 |
|---|---:|---|
| VCC | Switched 5V 권장 | GPS 전원 |
| GND | GND | 공통 접지 |
| TX | GPIO16 | GPS TX → ESP32 RX |
| RX | GPIO15 | GPS RX ← ESP32 TX |
| SCL | 미연결 | Compass 사용 시에만 연결 |
| SDA | 미연결 | Compass 사용 시에만 연결 |

권장 연결:

```text
GPS VCC → GPS 전원 스위치 출력, 5V 권장
GPS GND → ESP32 GND
GPS TX  → ESP32 GPIO16
GPS RX  → ESP32 GPIO15
GPS SCL → 미연결
GPS SDA → 미연결
```

### TX/RX Rule

UART는 교차 연결합니다.

```text
GPS TX → ESP32 RX
GPS RX → ESP32 TX
```

따라서 본 프로젝트에서는 다음과 같습니다.

```text
GPS TX → GPIO16
GPS RX → GPIO15
```

### Compass Lines

GPS 모듈의 SCL/SDA는 일반적으로 GPS 위치 데이터용이 아니라 내장 Compass용 I2C 라인입니다.

현재 백업 다이브컴퓨터 기능에서는 Compass가 필수 기능이 아니므로 우선 연결하지 않습니다.

나중에 Compass를 사용한다면 다음처럼 연결할 수 있습니다.

```text
GPS SCL → GPIO6
GPS SDA → GPIO5
```

다만 이 경우 MS5837과 같은 I2C 버스를 공유하게 되므로, I2C 주소 충돌과 풀업 전압을 확인해야 합니다.

---

## 9. GPS Backup Battery / Coin Cell Policy

GPS 모듈에 코인배터리 또는 VBAT 입력이 있는 경우가 있습니다.

본 프로토타입에서는 다음 방침을 적용합니다.

- GPS 코인배터리는 방전되어도 치명적인 문제는 없다고 판단합니다.
- 코인배터리가 없으면 GPS는 cold start로 동작할 수 있습니다.
- Cold start는 fix 시간이 길어질 수 있지만, sealed dive computer 구조에서는 허용 가능한 설계로 봅니다.
- GPS VBAT에 ESP32 3.3V를 상시 연결하는 방식은 사용하지 않습니다.
- 이유: 대기전력 증가 가능성이 있기 때문입니다.

권장 정책:

```text
GPS는 수면에서 위치 기록이 필요할 때만 전원을 켠다.
다이빙 중 또는 슬립 모드에서는 GPS 전원을 차단한다.
```

---

## 10. GPS Power Control

GPS 대기전력을 줄이기 위해 Load Switch 또는 MOSFET을 사용한 전원 제어를 권장합니다.

### Suggested Wiring

```text
Battery / 5V Rail
    → Load Switch
    → GPS VCC

ESP32 GPIO14
    → Load Switch EN
```

### Firmware Policy

```text
GPIO14 HIGH → GPS Power ON
GPIO14 LOW  → GPS Power OFF
```

권장 동작:

1. 수면 상태 진입
2. GPS 전원 ON
3. 일정 시간 동안 fix 대기
4. 위치 기록
5. GPS 전원 OFF
6. 슬립 또는 다이빙 모드 진입

---

## 11. Piezo Disc / Buzzer

12mm piezo disc를 사용할 예정입니다.

### Wiring

```text
ESP32 GPIO4 → 100~330Ω resistor → Piezo Disc
Piezo Disc other side → GND
```

단순 연결:

```text
Piezo Signal → GPIO4
Piezo GND    → GND
```

### Mechanical Mounting

- Piezo disc는 액션캠 다이빙 하우징 벽면에 부착 예정입니다.
- 하우징 벽면을 공진판처럼 사용할 수 있습니다.
- 실제 수중/하우징 조립 상태에서 음량 테스트가 필요합니다.
- 저전압 구동 시 음량이 부족하면 트랜지스터 드라이버 또는 전용 부저 드라이버 검토가 필요합니다.

---

## 12. Battery

현재 검토 중인 배터리 방향은 보호회로 내장 16340 Li-ion입니다.

### Recommended Battery Type

```text
Protected 16340 Li-ion
Nominal voltage: 3.6V / 3.7V
Full charge: 4.2V
Capacity: 현실적으로 약 650~950mAh 수준 예상
```

### Advantages

- 파우치형 LiPo보다 기계적으로 다루기 쉬울 수 있음
- 보호회로 내장 제품 선택 가능
- 과충전, 과방전, 단락, 과전류 보호 가능
- 배터리 홀더 사용 가능

### Warnings

- 보호회로는 충전회로가 아닙니다.
- 반드시 Li-ion CC/CV 4.2V 충전회로가 필요합니다.
- 직접 납땜 금지.
- 보호회로 내장 16340은 일반 16340보다 길 수 있습니다.
- 배터리 홀더에 실제로 들어가는지 확인해야 합니다.
- 저가 제품의 과장 용량 표기에 주의해야 합니다.

구매 시 확인할 항목:

```text
Protected
PCB protected
Button top
Over-charge protection
Over-discharge protection
Short-circuit protection
Realistic capacity, 약 650~950mAh
```

---

## 13. Wireless Charging and Charging Circuit

무선충전 모듈은 Qi receiver 또는 유사한 5V 출력 모듈을 사용할 수 있습니다.

다만 중요한 점은 다음입니다.

> 무선충전 수신 보드가 단순 5V 출력인지, Li-ion 충전 기능까지 포함하는지 반드시 확인해야 합니다.

일부 4단자 모듈은 다음 구조일 수 있습니다.

```text
B+ / B-     → Battery
OUT+ / OUT- → Device
```

이 경우 충전 기능이 포함되어 있을 수 있습니다.

하지만 다음 항목을 실제로 확인해야 합니다.

1. Li-ion CC/CV 4.2V 충전인지
2. 과충전 방지 기능이 있는지
3. 충전 종료 전압이 정확한지
4. 보호회로 내장 배터리와 충돌하지 않는지
5. 배터리 없이 Qi 입력만으로 OUT 전원이 나오는지
6. 충전 중 시스템 전원이 안정적으로 유지되는지
7. Power-path 또는 load sharing 기능이 있는지

---

## 14. Proposed Power Architecture

### Option A: Qi Receiver + Dedicated Li-ion Charger

가장 명확한 구조입니다.

```text
Qi Wireless Receiver 5V OUT
        ↓
Li-ion Charger Module IN+ / IN-
        ↓
Battery B+ / B-
        ↓
System OUT+ / OUT-
        ↓
5V Boost or 3.3V Regulator
        ↓
ESP32-S3 System
```

배선 개념:

```text
Wireless Charger OUT+ → Charger IN+
Wireless Charger OUT- → Charger IN-

Battery + → Charger B+
Battery - → Charger B-

Charger OUT+ → System Power
Charger OUT- → System GND
```

### Option B: 4-Terminal Wireless Charging Board with Built-in Charger

제품이 다음 단자를 제공하는 경우:

```text
B+
B-
OUT+
OUT-
```

내장 충전 기능이 있을 수 있습니다.

이 경우에도 실제 측정이 필요합니다.

테스트 항목:

```text
Qi 입력 시 B+ / B- 전압
충전 종료 전압
OUT+ / OUT- 전압
배터리 없이 OUT 출력 여부
충전 중 ESP32 리셋 여부
배터리 보호회로 차단 시 OUT 유지 여부
```

---

## 15. ESP32 Power Input

ESP32-S3 Dev Board를 사용하는 프로토타입에서는 다음 두 가지 방식 중 하나를 선택할 수 있습니다.

### Method A: Battery → 5V Boost → ESP32 5V Pin

```text
Battery
  → 5V Boost Converter
  → ESP32 5V Pin
```

장점:

- Dev board에서 사용하기 쉽습니다.
- USB 5V 전원 구조와 유사합니다.

단점:

- 승압 손실이 있습니다.
- 저전력 최적화에는 불리할 수 있습니다.

### Method B: Battery → 3.3V Regulator → ESP32 3V3 Pin

```text
Battery
  → Efficient 3.3V Regulator
  → ESP32 3V3 Pin
```

장점:

- 효율이 더 좋을 수 있습니다.
- 최종 PCB 설계에 더 적합합니다.

단점:

- Dev board의 3V3 레일에 직접 전원을 넣는 방식이므로 주의가 필요합니다.
- USB 연결 시 역전류/전원 충돌 방지 설계가 필요합니다.

프로토타입 초기 단계에서는 Method A가 단순합니다.  
최종 저전력 설계에서는 Method B 또는 별도 전원관리 IC 사용을 검토합니다.

---

## 16. Qi Detect Input

무선충전 상태를 펌웨어에서 감지하려면 GPIO2를 사용할 수 있습니다.

주의:

```text
Qi 5V OUT을 GPIO에 직접 연결하면 안 됩니다.
ESP32 GPIO는 3.3V 레벨입니다.
```

### Suggested Divider

```text
Qi 5V OUT+
   → 100kΩ
   → GPIO2
   → 200kΩ
   → GND
```

또는 저전력 분압:

```text
Qi 5V OUT+
   → 330kΩ
   → GPIO2
   → 680kΩ
   → GND
```

---

## 17. Battery Voltage Measurement

배터리 전압 측정은 GPIO1 ADC를 권장합니다.

16340 Li-ion은 완충 시 약 4.2V이므로 직접 GPIO에 연결하면 안 됩니다.

### Suggested Divider

```text
Battery +
   → 200kΩ
   → GPIO1
   → 100kΩ
   → GND
```

이 경우 GPIO1에는 배터리 전압의 약 1/3이 입력됩니다.

예:

```text
Battery 4.2V → GPIO1 약 1.4V
Battery 3.7V → GPIO1 약 1.23V
Battery 3.0V → GPIO1 약 1.0V
```

저전력 최적화를 위해서는 배터리 전압 분압 회로도 항상 켜두지 않고, MOSFET 또는 GPIO 제어로 측정 시에만 활성화하는 방법을 검토합니다.

---

## 18. Pins to Avoid

ESP32-S3에서 다음 핀들은 가능하면 일반 부품 연결에 사용하지 않습니다.

| Pin | 이유 |
|---:|---|
| GPIO0 | BOOT / 다운로드 모드 관련 |
| GPIO3 | Strapping pin |
| GPIO45 | Strapping pin |
| GPIO46 | Strapping pin, 입력 전용/제약 가능성 |
| GPIO19 | USB D- |
| GPIO20 | USB D+ |
| GPIO43 | UART0 TX / 로그 및 업로드 충돌 가능 |
| GPIO44 | UART0 RX / 로그 및 업로드 충돌 가능 |
| GPIO47 | 보드에 따라 RGB LED 또는 특수 기능 |
| GPIO48 | 보드에 따라 RGB LED 또는 특수 기능 |

현재 권장 핀맵은 이 위험 핀들을 대부분 피하도록 구성되어 있습니다.

---

## 19. Firmware Pin Definitions

현재 하드웨어 기준 `include/config.h` 등에 반영할 수 있는 예시입니다.

```cpp
// Display: ST7567A 128x64 SPI LCD
#define PIN_LCD_SCLK    12
#define PIN_LCD_MOSI    11
#define PIN_LCD_CS      10
#define PIN_LCD_DC       8
#define PIN_LCD_RST      9
#define PIN_LCD_BL       7

// I2C: MS5837 pressure/temperature sensor
#define PIN_I2C_SDA      5
#define PIN_I2C_SCL      6

// Buzzer / Piezo
#define PIN_BUZZER       4

// GPS UART
#define PIN_GPS_RX      16  // ESP32 RX, connect to GPS TX
#define PIN_GPS_TX      15  // ESP32 TX, connect to GPS RX

// Optional power management
#define PIN_QI_DETECT    2
#define PIN_BATTERY_ADC  1
#define PIN_GPS_EN      14
```

---

## 20. Open Hardware Questions

아직 실제 부품 수령 후 확인해야 할 항목입니다.

### Display

- ST7567A 모듈이 3.3V에서 정상 동작하는가?
- Backlight 핀에 직렬저항이 내장되어 있는가?
- U8g2에서 어떤 ST7567 생성자가 정확히 맞는가?
- 화면 방향, 반전, contrast 설정이 필요한가?

### Pressure Sensor

- MS5837 모듈의 I2C 주소는 무엇인가?
- Breakout board의 풀업 저항은 3.3V에 연결되어 있는가?
- 압력 포트 및 O-ring 실링 구조는 어떻게 구성할 것인가?

### GPS

- GPS UART 로직 레벨은 3.3V인가?
- GPS 기본 baud rate는 9600인가, 115200인가?
- 코인배터리 없이 cold start가 정상 동작하는가?
- GPS 전원 차단 후 재시작 시간이 허용 가능한가?

### Charging / Power

- 무선충전 보드가 단순 5V receiver인지, 충전회로 포함형인지?
- 충전 종료 전압은 4.2V로 정확한가?
- 보호회로 내장 16340과 충전모듈이 충돌하지 않는가?
- 충전 중 ESP32가 리셋되지 않는가?
- 배터리 없이 Qi 입력만으로 시스템 전원이 유지되는가?
- Power-path 기능이 있는가?

### Battery

- 보호회로 내장 16340의 실제 길이가 배터리 홀더와 맞는가?
- 실제 용량은 충분한가?
- 슬립 모드 전류는 목표 범위 내인가?
- 1일 3회, 7일 사용 시나리오에서 충분한가?

### Enclosure

- 액션캠 하우징 내부에 모든 부품이 들어가는가?
- 압력센서가 외부 수압을 정확히 받을 수 있는가?
- 피에조 디스크가 하우징 벽면에서 충분한 음량을 내는가?
- 무선충전 코일 위치와 하우징 두께가 충전에 적합한가?
- 장시간 수중 압력 테스트를 통과하는가?

---

## 21. Current Hardware Decision Summary

현재 기준 결정 사항:

```text
MCU        = ESP32-S3 N16R8 Dev Board
Display    = ST7567A 128x64 SPI LCD
Pressure   = MS5837-30BA I2C
GPS        = UART GPS module, Compass lines unused
Buzzer     = 12mm Piezo Disc on GPIO4
Battery    = Protected 16340 Li-ion preferred
Charging   = Qi wireless charging + Li-ion charger/power module under verification
Enclosure  = Action camera diving housing under evaluation
```

최종 핵심 핀:

```text
GPIO4  = Piezo Buzzer
GPIO5  = MS5837 SDA
GPIO6  = MS5837 SCL
GPIO7  = LCD Backlight
GPIO8  = LCD DC
GPIO9  = LCD Reset
GPIO10 = LCD CS
GPIO11 = LCD MOSI
GPIO12 = LCD SCLK
GPIO15 = GPS TX
GPIO16 = GPS RX
```

선택 기능 핀:

```text
GPIO1  = Battery ADC
GPIO2  = Qi Detect
GPIO14 = GPS Power Enable
```

---

## 22. Next Steps

1. 실제 부품 수령
2. LCD ST7567A 단독 표시 테스트
3. MS5837 I2C scanner 및 수심 센서 테스트
4. GPS UART cold start 테스트
5. Piezo disc 하우징 부착 음량 테스트
6. Qi 충전보드 충전/전원 유지 테스트
7. 16340 보호회로 배터리 충전 안정성 테스트
8. 슬립 전류 측정
9. 방수 하우징 내부 배치 검토
10. 압력센서 포트 및 실링 구조 설계

---

## 23. Safety Notice

본 프로토타입은 실제 생명보호용 다이브 컴퓨터가 아닙니다.

실제 다이빙에 사용하기 전 다음 검증이 필요합니다.

- 장시간 방수 테스트
- 압력 테스트
- 센서 오차 검증
- 배터리 충전/방전 안전성 검증
- 저온 환경 테스트
- 수중 가독성 테스트
- 경고음 전달 테스트
- 펌웨어 예외상황 테스트
- 기존 인증 다이브컴퓨터와의 비교 테스트

본 장치는 반드시 보조 장비 또는 실험 장비로만 사용해야 합니다.
```
