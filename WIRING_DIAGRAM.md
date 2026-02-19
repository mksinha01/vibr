# 🔌 WIRING DIAGRAM - Autonomous Delivery Robot
## ESP8266 NodeMCU Based System

**Board:** ESP8266 NodeMCU (ESP-12E Module)  
**Date:** February 19, 2026  
**Voltage:** 3.3V Logic, 5V-12V Motor Power

---

## ⚠️ IMPORTANT SAFETY NOTES

1. **ESP8266 is 3.3V logic** - Do NOT connect 5V directly to GPIO pins!
2. **Motors require separate power** - Use external 5-12V battery for motors
3. **Common ground** - Connect all ground pins together
4. **Pin sharing** - Some pins are shared (see notes below)
5. **Power supply** - Use adequate power supply (2A+ for motors)

---

## 📊 COMPLETE PIN ASSIGNMENT TABLE

| Component | Function | ESP8266 Pin | GPIO | Notes |
|-----------|----------|-------------|------|-------|
| **MOTOR DRIVER L298D** |
| Left Motor PWM | Speed Control | D1 | GPIO5 | ⚠️ Shared with I2C SCL |
| Left Motor IN1 | Direction | D3 | GPIO0 | |
| Left Motor IN2 | Direction | D4 | GPIO2 | |
| Right Motor PWM | Speed Control | D5 | GPIO14 | |
| Right Motor IN1 | Direction | D6 | GPIO12 | |
| Right Motor IN2 | Direction | D7 | GPIO13 | ⚠️ Shared with Sonar Echo |
| **ULTRASONIC SENSOR (HC-SR04)** |
| Trigger | Pulse Output | D8 | GPIO15 | |
| Echo | Pulse Input | D7 | GPIO13 | ⚠️ Shared with Motor Right IN2 |
| **MPU6050 IMU (I2C)** |
| SDA | I2C Data | D2 | GPIO4 | Standard I2C SDA |
| SCL | I2C Clock | D1 | GPIO5 | ⚠️ Shared with Motor Left PWM |
| **GPS MODULE** |
| RX | Serial Receive | RX | GPIO3 | ⚠️ Serial debugging disabled when GPS active |
| TX | Serial Transmit | TX | GPIO1 | ⚠️ Serial debugging disabled when GPS active |
| **EMERGENCY STOP** |
| Button | E-Stop Input | D0 | GPIO16 | Pull-up, active LOW |
| **STATUS LED** |
| LED | Status Indicator | D4 | GPIO2 | Built-in LED |

---

## 🔧 DETAILED COMPONENT WIRING

### 1. ESP8266 NodeMCU Power
```
ESP8266 NodeMCU:
┌─────────────────┐
│  VIN  ───────── 5V (USB or External 5V)
│  GND  ───────── Ground (Common Ground)
│  3V3  ───────── (Output 3.3V - max 300mA)
└─────────────────┘

Note: Can power via USB (5V) or VIN pin (5-12V with onboard regulator)
```

### 2. L298D Motor Driver Connections
```
ESP8266 NodeMCU          L298D Motor Driver
┌─────────────────┐      ┌──────────────────────┐
│                 │      │                      │
│  D1 (GPIO5)  ───┼──────┼─→ ENA (Left PWM)    │
│  D3 (GPIO0)  ───┼──────┼─→ IN1               │
│  D4 (GPIO2)  ───┼──────┼─→ IN2               │
│                 │      │                      │
│  D5 (GPIO14) ───┼──────┼─→ ENB (Right PWM)   │
│  D6 (GPIO12) ───┼──────┼─→ IN3               │
│  D7 (GPIO13) ───┼──────┼─→ IN4               │
│                 │      │                      │
│  GND ───────────┼──────┼─→ GND               │
└─────────────────┘      │                      │
                         │  VCC ←─── 5V Logic   │
                         │  12V ←─── Motor Power (5-12V) │
                         │  GND ←─── Motor Ground│
                         │                      │
                         │  OUT1 ←─── Left Motor + │
                         │  OUT2 ←─── Left Motor - │
                         │  OUT3 ←─── Right Motor + │
                         │  OUT4 ←─── Right Motor - │
                         └──────────────────────┘

Motor Power Supply:
    Battery (+) ──→ L298D 12V
    Battery (-) ──→ Common GND

⚠️ CRITICAL: 
- L298D logic pins (IN1-IN4, ENA, ENB) connect to ESP8266 (3.3V safe)
- Motor power (12V/VCC) comes from SEPARATE battery
- ALL grounds must be connected together
```

### 3. HC-SR04 Ultrasonic Sensor
```
ESP8266 NodeMCU          HC-SR04 Sonar
┌─────────────────┐      ┌──────────────┐
│                 │      │              │
│  D8 (GPIO15) ───┼──────┼─→ TRIG      │
│  D7 (GPIO13) ───┼──────┼─→ ECHO      │
│  5V or 3.3V  ───┼──────┼─→ VCC       │
│  GND ────────────┼──────┼─→ GND       │
└─────────────────┘      └──────────────┘

⚠️ NOTE: 
- Most HC-SR04 work with 3.3V, some need 5V
- If using 5V VCC, ECHO output might be 5V!
- Use voltage divider for ECHO if 5V:
  
  ECHO ──┬── 1kΩ ──→ ESP8266 D7
         └── 2kΩ ──→ GND
  
  This creates ~3.3V safe voltage
```

### 4. MPU6050 IMU (I2C)
```
ESP8266 NodeMCU          MPU6050
┌─────────────────┐      ┌──────────────┐
│                 │      │              │
│  D2 (GPIO4)  ───┼──────┼─→ SDA       │
│  D1 (GPIO5)  ───┼──────┼─→ SCL       │
│  3.3V ───────────┼──────┼─→ VCC       │
│  GND ────────────┼──────┼─→ GND       │
│                 │      │  AD0 ──→ GND │ (I2C address 0x68)
└─────────────────┘      └──────────────┘

⚠️ IMPORTANT:
- MPU6050 is 3.3V device
- AD0 pin to GND selects address 0x68
- If you have XDA/XCL pins, leave unconnected
```

### 5. GPS Module (NEO-6M or similar)
```
ESP8266 NodeMCU          GPS Module
┌─────────────────┐      ┌──────────────┐
│                 │      │              │
│  RX (GPIO3)  ───┼──────┼─→ TX        │
│  TX (GPIO1)  ───┼──────┼─→ RX        │
│  3.3V or 5V  ───┼──────┼─→ VCC       │
│  GND ────────────┼──────┼─→ GND       │
└─────────────────┘      └──────────────┘

⚠️ WARNING:
- Using RX/TX disables Serial debugging (Serial.print won't work)
- GPS TX → ESP RX (cross connection)
- GPS RX → ESP TX (cross connection)
- Check your GPS module voltage (3.3V or 5V)
- For 5V GPS: Use voltage divider on GPS TX → ESP RX
```

### 6. Emergency Stop Button
```
ESP8266 NodeMCU          Emergency Button
┌─────────────────┐      
│                 │      
│  D0 (GPIO16) ───┼───┬─── Button ─── GND
│                 │   │
│                 │   └─── 10kΩ ─── 3.3V (pull-up)
└─────────────────┘      

Button Wiring (Active LOW):
    
    3.3V ──┬── 10kΩ ──┬── D0 (GPIO16)
           │          │
           │          └── Button ── GND
           
When pressed: D0 reads LOW (0V)
When released: D0 reads HIGH (3.3V via pull-up)

Note: Internal pull-up can be used, external is more reliable
```

### 7. Status LED (Built-in)
```
ESP8266 NodeMCU has built-in LED on GPIO2 (D4)
No external wiring needed!

If adding external LED:
    D4 (GPIO2) ──→ LED (+) ──→ 220Ω ──→ GND
```

---

## 🔌 COMPLETE BREADBOARD/PCB LAYOUT

### Full System Wiring Diagram
```
                    ┌─────────────────────────────────────────┐
                    │     EXTERNAL MOTOR BATTERY (6-12V)      │
                    │         (+)              (-)            │
                    └──────────┬────────────────┬─────────────┘
                               │                │
                               │                └───────────────┐
                               │                                │
                    ┌──────────▼─────────────────────────┐     │
                    │       L298D Motor Driver           │     │
                    │                                    │     │
                    │  12V   VCC   GND   ENA  IN1  IN2  │     │
                    │   │     │     │     │    │    │   │     │
                    └───┼─────┼─────┼─────┼────┼────┼───┘     │
                        │     │     │     │    │    │         │
                        │     │     └─────┼────┼────┼─────────┤ Common GND
                        │     │           │    │    │         │
                        │   ┌─▼───────────▼────▼────▼─────┐   │
                        │   │      ESP8266 NodeMCU        │   │
    5V USB/Power ───────┼───┤ VIN  GND  D1  D3  D4       │   │
                        │   │                              │   │
                        │   │  3.3V D2  D5  D6  D7  D8    │   │
                        │   │   │    │   │   │   │   │    │   │
                        │   └───┼────┼───┼───┼───┼───┼────┘   │
                        │       │    │   │   │   │   │        │
                        │       │    │   │   │   │   │        │
                    ┌───▼───────▼────┘   │   │   │   │        │
                    │  MPU6050 IMU       │   │   │   │        │
                    │  VCC SDA SCL GND   │   │   │   │        │
                    └────────────────────┘   │   │   │        │
                                             │   │   │        │
                    ┌────────────────────────┼───┼───┼────┐   │
                    │  L298D (continued)     │   │   │    │   │
                    │  ENB IN3 IN4          │   │   │    │   │
                    └────────────────────────┘   │   │    │   │
                                                 │   │    │   │
                    ┌────────────────────────────┼───┼────┘   │
                    │  HC-SR04 Sonar             │   │        │
                    │  VCC TRIG ECHO GND        │   │        │
                    └────────────────────────────┘   │        │
                                                     │        │
                    ┌────────────────────────────────┘        │
                    │  GPS Module (Optional)                  │
                    │  VCC TX RX GND                          │
                    └─────────────────────────────────────────┘
                                                               │
    Ground Bus ────────────────────────────────────────────────┘
    (All GND connected together)
```

---

## ⚡ POWER SUPPLY REQUIREMENTS

### Component Power Consumption:
| Component | Voltage | Current | Notes |
|-----------|---------|---------|-------|
| ESP8266 NodeMCU | 3.3V | 80-170mA | Via onboard regulator from 5V |
| MPU6050 IMU | 3.3V | 3.5mA | Very low power |
| HC-SR04 Sonar | 5V or 3.3V | 15mA | Peak 30mA during ping |
| GPS Module | 3.3-5V | 40-60mA | Depends on model |
| L298D Logic | 5V | 10mA | Logic circuits only |
| DC Motors (2x) | 6-12V | 500mA-2A each | **Depends on motor size!** |
| **TOTAL** | - | **2-5A** | **Motors are the main draw** |

### Recommended Power Setup:

#### Option 1: USB + External Motor Power (RECOMMENDED FOR TESTING)
```
USB 5V (1A) ──→ ESP8266 VIN (powers ESP8266, sensors)
Battery 7.4V (2S LiPo, 2A+) ──→ L298D 12V pin (powers motors only)
```

#### Option 2: Single Battery Power (RECOMMENDED FOR DEPLOYMENT)
```
Battery 7.4-11.1V (2S-3S LiPo, 3A+) ──┬──→ Voltage Regulator 5V ──→ ESP8266 VIN
                                      └──→ L298D 12V pin
```

#### Option 3: Dual Battery System (SAFEST)
```
Battery 1: 5V power bank ──→ ESP8266 via USB (sensors, logic)
Battery 2: 7.4V LiPo ──→ L298D 12V pin (motors only)
```

### Battery Recommendations:
- **2S LiPo (7.4V, 2200mAh)** - Good for medium motors
- **3S LiPo (11.1V, 2200mAh)** - Better for larger motors
- **6x AA batteries (9V)** - Simple, but limited current
- **Power bank (5V, 2A+)** - For ESP8266 only, separate motor power needed

---

## ⚠️ PIN SHARING STRATEGY & IMPACT

### Shared Pin #1: D1 (GPIO5)
**Functions:** Motor Left PWM + I2C SCL

**How it works:**
- I2C communication happens in bursts (reading IMU data)
- Motor PWM is continuous when moving
- **Impact:** Minor - I2C reads should happen between motor updates
- **Solution:** Main loop already sequences operations properly

**Timing:**
```
Loop cycle (50ms):
1. Read I2C (IMU data) - 2-5ms
2. Update motors - remaining time
No conflict because they don't happen simultaneously
```

### Shared Pin #2: D7 (GPIO13)
**Functions:** Motor Right IN2 + Sonar Echo

**How it works:**
- Sonar Echo receives pulses for distance measurement
- Motor IN2 controls direction (HIGH/LOW, not PWM)
- **Impact:** Minor - Sonar readings affected when changing motor direction
- **Solution:** Sonar primarily used when motors are stable/moving forward

**Mitigation:**
```
When obstacle detected:
1. Motors stop (IN2 becomes LOW and stays stable)
2. Sonar can read clearly
3. Decision made
4. Motors resume
```

### Shared Pin #3: RX/TX (GPIO3/GPIO1)
**Functions:** Serial Debug + GPS Communication

**How it works:**
- Cannot use Serial.print() when GPS is active
- **Impact:** No serial debugging when GPS connected
- **Solution:** Remove GPS for debugging, or use WiFi logging

---

## 🔧 ASSEMBLY INSTRUCTIONS

### Step 1: Prepare Components
- [ ] ESP8266 NodeMCU
- [ ] L298D Motor Driver module
- [ ] 2x DC Motors (6-12V)
- [ ] MPU6050 IMU module
- [ ] HC-SR04 Ultrasonic sensor
- [ ] GPS module (optional)
- [ ] Emergency stop button
- [ ] Breadboard or custom PCB
- [ ] Jumper wires
- [ ] Battery/Power supply

### Step 2: ESP8266 to L298D (Motors)
```
ESP D1  → L298D ENA
ESP D3  → L298D IN1
ESP D4  → L298D IN2
ESP D5  → L298D ENB
ESP D6  → L298D IN3
ESP D7  → L298D IN4
ESP GND → L298D GND
```

### Step 3: ESP8266 to MPU6050 (IMU)
```
ESP 3.3V → MPU6050 VCC
ESP D2   → MPU6050 SDA
ESP D1   → MPU6050 SCL (shared with motor PWM)
ESP GND  → MPU6050 GND
MPU AD0  → GND (sets I2C address to 0x68)
```

### Step 4: ESP8266 to HC-SR04 (Sonar)
```
ESP 3.3V or 5V → HC-SR04 VCC
ESP D8         → HC-SR04 TRIG
ESP D7         → HC-SR04 ECHO (shared with motor IN2)
ESP GND        → HC-SR04 GND

If using 5V VCC, add voltage divider on ECHO:
ECHO → 1kΩ → ESP D7
       └─ 2kΩ → GND
```

### Step 5: ESP8266 to GPS (Optional)
```
ESP 3.3V → GPS VCC
ESP RX   → GPS TX
ESP TX   → GPS RX
ESP GND  → GPS GND
```

### Step 6: Emergency Stop Button
```
ESP 3.3V ──┬── 10kΩ ──┬── ESP D0
           │          │
           │          └── Button ── GND
```

### Step 7: Power Connections
```
Option A (Testing with USB):
  USB 5V → ESP8266 VIN
  Battery → L298D 12V pin
  All GND → Common ground

Option B (Battery operation):
  Battery → Voltage regulator → ESP8266 VIN
  Battery → L298D 12V pin
  All GND → Common ground
```

### Step 8: Motor Connections
```
L298D OUT1 → Left Motor +
L298D OUT2 → Left Motor -
L298D OUT3 → Right Motor +
L298D OUT4 → Right Motor -
```

---

## 🧪 TESTING CHECKLIST

### Before Powering On:
- [ ] All GND pins connected to common ground
- [ ] No short circuits between VCC and GND
- [ ] Motor power separate from logic power
- [ ] Double-check all pin assignments
- [ ] Emergency stop button accessible

### Initial Power-On Test (NO MOTORS):
- [ ] Power ESP8266 only (USB)
- [ ] Check 3.3V output (should be ~3.3V)
- [ ] Upload test sketch
- [ ] Verify serial output at 115200 baud

### Component Testing (One at a time):
- [ ] MPU6050: Check I2C communication (should detect 0x68)
- [ ] Sonar: Test distance readings
- [ ] GPS: Check NMEA sentences (if connected)
- [ ] Emergency stop: Test button press

### Motor Testing (WITH MOTORS):
- [ ] Power motor battery separately
- [ ] Run motor test sequence
- [ ] Check motor directions
- [ ] Verify emergency stop works
- [ ] Test obstacle avoidance

---

## 🔍 TROUBLESHOOTING

### Problem: ESP8266 won't boot
**Check:**
- GPIO15 must be LOW (D8 has pull-down)
- GPIO0 can't be LOW during boot (D3)
- GPIO2 must be HIGH during boot (D4)
- Remove all connections and test ESP alone

### Problem: Motors don't work
**Check:**
- Separate motor power connected
- L298D GND connected to ESP GND
- Motor power voltage (6-12V)
- Enable pins (ENA, ENB) getting PWM signal
- Motor connections to OUT1-OUT4

### Problem: I2C not working (MPU6050)
**Check:**
- SDA and SCL not swapped
- MPU6050 VCC at 3.3V
- AD0 pin connected to GND
- Run I2C scanner sketch
- Avoid motor PWM when reading I2C

### Problem: Sonar gives wrong readings
**Check:**
- TRIG and ECHO not swapped
- Voltage divider if using 5V VCC
- Pin sharing with motor direction
- Obstacles in front of sensor
- Take readings when motors stable

### Problem: GPS not getting fix
**Check:**
- Antenna has clear view of sky
- TX/RX not swapped
- Baud rate 9600
- Wait 30-60 seconds for initial fix
- Indoor GPS won't work - go outside!

---

## 📐 RECOMMENDED PCB LAYOUT

If you want to create a custom PCB:

### Component Placement:
```
┌─────────────────────────────────────┐
│                                     │
│  [GPS]        [MPU6050]             │
│                                     │
│        [ESP8266 NodeMCU]            │
│                                     │
│  [HC-SR04]    [L298D]    [E-STOP]   │
│                                     │
│  Motor Terminal        Power Input │
│  [M1] [M2]            [BAT] [USB]   │
└─────────────────────────────────────┘

Mounting Holes in corners
Keep high-current paths (motor) short and thick
Separate power and signal grounds (star ground)
```

### PCB Design Tips:
- Use thick traces (20mil+) for motor power
- Keep I2C traces short and parallel
- Add decoupling capacitors (100nF) near each VCC
- Add bulk capacitor (100-1000µF) on motor power
- Use ground plane for better noise immunity

---

## 📦 RECOMMENDED PURCHASE LIST

### Essential Components:
- [ ] ESP8266 NodeMCU (1x)
- [ ] L298D Motor Driver Module (1x)
- [ ] DC Motors 6-12V with wheels (2x)
- [ ] MPU6050 IMU Module (1x)
- [ ] HC-SR04 Ultrasonic Sensor (1x)
- [ ] Push button (emergency stop) (1x)
- [ ] 10kΩ resistors (3x)
- [ ] 1kΩ & 2kΩ resistors (voltage divider)
- [ ] Jumper wires (Male-Male, Male-Female)
- [ ] Breadboard or PCB
- [ ] Battery: 7.4V LiPo 2200mAh (or suitable)
- [ ] Battery connector & switch

### Optional Components:
- [ ] GPS Module NEO-6M (1x)
- [ ] Voltage regulator 5V (if using single battery)
- [ ] Capacitors: 100nF (5x), 100µF (2x)
- [ ] Status LEDs & 220Ω resistors
- [ ] Robot chassis/platform

### Tools Needed:
- [ ] Soldering iron & solder
- [ ] Wire strippers
- [ ] Multimeter
- [ ] USB cable (Micro USB for NodeMCU)
- [ ] Screwdrivers

---

## ⚡ QUICK REFERENCE PINOUT

### ESP8266 NodeMCU D-Pin to GPIO Mapping:
```
D0  = GPIO16  (E-Stop)
D1  = GPIO5   (I2C SCL + Motor Left PWM)  ⚠️ SHARED
D2  = GPIO4   (I2C SDA)
D3  = GPIO0   (Motor Left IN1)
D4  = GPIO2   (Motor Left IN2 + Built-in LED)
D5  = GPIO14  (Motor Right PWM)
D6  = GPIO12  (Motor Right IN1)
D7  = GPIO13  (Motor Right IN2 + Sonar ECHO)  ⚠️ SHARED
D8  = GPIO15  (Sonar TRIG)
RX  = GPIO3   (GPS TX)
TX  = GPIO1   (GPS RX)
```

### Color Code Suggestion:
- **Red** = Power (VCC, VIN, Battery +)
- **Black** = Ground (GND, Battery -)
- **Yellow** = Signal (GPIO pins)
- **Blue** = I2C (SDA/SCL)
- **Green** = Serial (RX/TX)

---

## ✅ FINAL VERIFICATION

Before deploying:
- [ ] All connections match this diagram
- [ ] No loose wires
- [ ] Motor power isolated from logic power
- [ ] Emergency stop tested and working
- [ ] Self-test passes (7/7 tests)
- [ ] Robot chassis secure
- [ ] Batteries charged
- [ ] Ready to roll! 🤖

---

**Your robot is wired and ready for testing!**

For software setup, see [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md)

Good luck! 🚀
