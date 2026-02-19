# 🔌 WIRING DIAGRAM - Autonomous Delivery Robot V2
## ESP8266 NodeMCU Based System (PIN CONFLICTS SOLVED)

**Board:** ESP8266 NodeMCU (ESP-12E Module)  
**Version:** V2 - GPS Disabled, Conflict-Free Pin Assignment  
**Date:** February 19, 2026  
**Voltage:** 3.3V Logic, 5V-12V Motor Power

---

## ⚠️ IMPORTANT SAFETY NOTES

1. **ESP8266 is 3.3V logic** - Do NOT connect 5V directly to GPIO pins!
2. **Motors require separate power** - Use external 5-12V battery for motors
3. **Common ground** - Connect all ground pins together
4. **✅ V2: NO PIN CONFLICTS** - All functions have dedicated GPIO pins
5. **Power supply** - Use adequate power supply (2A+ for motors)
6. **⚠️ Sonar ECHO voltage divider required** - See wiring details below

---

## 📊 COMPLETE PIN ASSIGNMENT TABLE (V2 - CONFLICT-FREE)

| Component | Function | ESP8266 Pin | GPIO | Notes |
|-----------|----------|-------------|------|-------|
| **MOTOR DRIVER L298D** |
| Left Motor PWM | Speed Control | D0 | GPIO16 | ✅ Dedicated PWM |
| Left Motor IN1 | Direction | D4 | GPIO2 | Shares with built-in LED |
| Left Motor IN2 | Direction | D5 | GPIO14 | ✅ Dedicated |
| Right Motor PWM | Speed Control | D6 | GPIO12 | ✅ Dedicated PWM |
| Right Motor IN1 | Direction | D8 | GPIO15 | ✅ Dedicated |
| Right Motor IN2 | Direction | D7 | GPIO13 | ✅ Dedicated (no conflict!) |
| **ULTRASONIC SENSOR (HC-SR04)** |
| Trigger | Pulse Output | D2 | GPIO4 | ✅ Dedicated OUTPUT |
| Echo | Pulse Input | D3 | GPIO0 | ✅ Dedicated INPUT (needs divider) |
| **MPU6050 IMU (I2C)** |
| SDA | I2C Data | D1 | GPIO5 | ✅ Dedicated I2C SDA |
| SCL | I2C Clock | TX | GPIO1 | ✅ Dedicated I2C SCL (no PWM!) |
| **GPS MODULE** |
| - | - | - | - | ❌ DISABLED in V2 (pins reallocated) |
| **STATUS LED** |
| LED | Status Indicator | D4 | GPIO2 | Built-in LED (shares with Motor IN1) |

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

### 2. L298D Motor Driver Connections (V2 - UPDATED PINS)
```
ESP8266 NodeMCU          L298D Motor Driver
┌─────────────────┐      ┌──────────────────────┐
│                 │      │                      │
│  D0 (GPIO16) ───┼──────┼─→ ENA (Left PWM)    │ ✅ CHANGED from D1
│  D4 (GPIO2)  ───┼──────┼─→ IN1               │ ✅ CHANGED from D3
│  D5 (GPIO14) ───┼──────┼─→ IN2               │ ✅ CHANGED from D4
│                 │      │                      │
│  D6 (GPIO12) ───┼──────┼─→ ENB (Right PWM)   │ Same
│  D8 (GPIO15) ───┼──────┼─→ IN3               │ ✅ CHANGED from D6
│  D7 (GPIO13) ───┼──────┼─→ IN4               │ Same
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

### 3. HC-SR04 Ultrasonic Sensor (V2 - UPDATED PINS + VOLTAGE DIVIDER)
```
ESP8266 NodeMCU          HC-SR04 Sonar
┌─────────────────┐      ┌──────────────┐
│                 │      │              │
│  D2 (GPIO4)  ───┼──────┼─→ TRIG      │ ✅ CHANGED from D8
│                 │      │              │
│  D3 (GPIO0)  ───┼──┐   │  ECHO ───────┤ ✅ CHANGED from D7
│                 │  │   └──────────────┘
│                 │  │
│                 │  └─── 1kΩ ─── ECHO (5V)
│                 │         │
│  GND ───────────┼─── 2kΩ ─┘    Creates 3.3V safe signal
│                 │
│  5V or 3.3V  ───┼──────┼─→ VCC (HC-SR04)
│  GND ────────────┼──────┼─→ GND
└─────────────────┘      

⚠️ CRITICAL VOLTAGE DIVIDER REQUIRED:
- HC-SR04 ECHO outputs 5V when powered from 5V
- ESP8266 GPIO pins are 3.3V maximum!
- MUST use voltage divider:
  
  ECHO (5V) ──→ 1kΩ resistor ──→ D3 (GPIO0)
                               └──→ 2kΩ resistor ──→ GND
  
  Formula: 5V × (2kΩ / 3kΩ) = 3.33V ✓ SAFE

ALTERNATIVE (Simpler but reduced range):
- Power HC-SR04 from 3.3V instead of 5V
- Direct connection: ECHO → D3 (no resistors needed)
- Range reduced to ~2-3m (still usable for robot)
```

### 4. MPU6050 IMU (I2C) - V2 DEDICATED PINS (NO CONFLICTS!)
```
ESP8266 NodeMCU          MPU6050
┌─────────────────┐      ┌──────────────┐
│                 │      │              │
│  D1 (GPIO5)  ───┼──────┼─→ SDA       │ ✅ CHANGED from D2 - DEDICATED!
│  TX (GPIO1)  ───┼──────┼─→ SCL       │ ✅ CHANGED from D1 - DEDICATED!
│  3.3V ───────────┼──────┼─→ VCC       │
│  GND ────────────┼──────┼─→ GND       │
│                 │      │  AD0 ──→ GND │ (I2C address 0x68)
└─────────────────┘      └──────────────┘

✅ V2 BENEFITS:
- SDA now on D1 (GPIO5) - no motor PWM interference!
- SCL now on TX (GPIO1) - no motor PWM interference!
- I2C communication is clean and reliable
- Serial debugging still available (RX pin free)

⚠️ NOTE:
- TX pin used for I2C means Serial.print() works on RX
- MPU6050 is 3.3V device only
- AD0 pin to GND selects address 0x68
```

### 5. GPS Module - DISABLED IN V2
```
❌ GPS NOT USED IN VERSION 2

Reason: Pin conflicts solved by reallocating GPS pins
- TX (GPIO1) now used for I2C SCL (MPU6050)
- RX (GPIO3) available but not needed

Benefits:
✅ I2C gets dedicated pins (reliable IMU communication)
✅ Serial debugging available (Serial.print works)
✅ Simplified wiring
✅ All sensors work without conflicts

Future: GPS can be added via software serial on other pins if needed
```

### 6. Emergency Stop Button - REMOVED IN V2
```
❌ EMERGENCY STOP PIN REALLOCATED

Reason: D0 (GPIO16) now used for Motor Left PWM

Alternatives:
1. Software emergency stop via WiFi command
2. Add physical button on available pin (GPIO3 RX if not debugging)
3. Use state machine ERROR state triggered by watchdog

Current safety features:
✅ Watchdog system monitoring
✅ Software emergency stop (motors.emergencyStop())
✅ Obstacle detection (sonar)
✅ Tilt detection (IMU)
✅ WiFi command: {"action": "emergency_stop"}
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

## ✅ V2 PIN ALLOCATION - NO CONFLICTS!

### Why V2 is Better:
```
✅ ALL FUNCTIONS HAVE DEDICATED GPIO PINS
✅ No PWM interference with I2C communication
✅ No motor/sonar conflicts
✅ Reliable sensor readings
✅ Serial debugging available (TX used for I2C but RX free)
✅ Professional-grade pin allocation
```

### Pin Assignment Summary:
| Function | GPIO | Pin | Dedicated? |
|----------|------|-----|------------|
| Motor Left PWM | 16 | D0 | ✅ Yes |
| Motor Left IN1 | 2 | D4 | ✅ Yes (shares with LED only) |
| Motor Left IN2 | 14 | D5 | ✅ Yes |
| Motor Right PWM | 12 | D6 | ✅ Yes |
| Motor Right IN1 | 15 | D8 | ✅ Yes |
| Motor Right IN2 | 13 | D7 | ✅ Yes - NO CONFLICT! |
| Sonar TRIG | 4 | D2 | ✅ Yes |
| Sonar ECHO | 0 | D3 | ✅ Yes - NO CONFLICT! |
| I2C SDA | 5 | D1 | ✅ Yes - NO PWM! |
| I2C SCL | 1 | TX | ✅ Yes - NO PWM! |

**Result:** Stable, reliable operation without timing-dependent workarounds!

---
- **Impact:** Minor - Sonar readings affected when changing motor direction
- **Solution:** Sonar primarily used when motors are stable/moving forward

**Mitigation:**
```
## 🔧 ASSEMBLY INSTRUCTIONS - V2 (CONFLICT-FREE)

### Step 1: Prepare Components
- [ ] ESP8266 NodeMCU
- [ ] L298D Motor Driver module
- [ ] 2x DC Motors (6-12V) with wheels
- [ ] MPU6050 IMU module
- [ ] HC-SR04 Ultrasonic sensor
- [ ] 2x 1kΩ resistors (for sonar voltage divider)
- [ ] 1x 2kΩ resistor (for sonar voltage divider)
- [ ] Breadboard or custom PCB
- [ ] Jumper wires (Male-Male, Male-Female)
- [ ] Battery/Power supply (7.4V LiPo recommended)

### Step 2: ESP8266 to L298D (Motors) - V2 UPDATED PINS
```
ESP D0 (GPIO16) → L298D ENA  (Left Motor PWM)   ✅ CHANGED from D1
ESP D4 (GPIO2)  → L298D IN1  (Left Motor Dir)   ✅ CHANGED from D3
ESP D5 (GPIO14) → L298D IN2  (Left Motor Dir)   ✅ CHANGED from D4
ESP D6 (GPIO12) → L298D ENB  (Right Motor PWM)  Same
ESP D8 (GPIO15) → L298D IN3  (Right Motor Dir)  ✅ CHANGED from D6
ESP D7 (GPIO13) → L298D IN4  (Right Motor Dir)  Same
ESP GND         → L298D GND
```

### Step 3: ESP8266 to MPU6050 (IMU) - V2 DEDICATED PINS
```
ESP 3.3V        → MPU6050 VCC
ESP D1 (GPIO5)  → MPU6050 SDA  ✅ CHANGED from D2 - DEDICATED!
ESP TX (GPIO1)  → MPU6050 SCL  ✅ CHANGED from D1 - DEDICATED!
ESP GND         → MPU6050 GND
MPU AD0         → GND (sets I2C address to 0x68)
```

### Step 4: ESP8266 to HC-SR04 (Sonar) - V2 UPDATED + VOLTAGE DIVIDER
```
HC-SR04 VCC  → ESP 5V or 3.3V
HC-SR04 GND  → ESP GND
HC-SR04 TRIG → ESP D2 (GPIO4)   ✅ CHANGED from D8
HC-SR04 ECHO → Voltage Divider → ESP D3 (GPIO0)  ✅ CHANGED from D7

⚠️ CRITICAL: Voltage Divider for ECHO (if using 5V VCC):
  HC-SR04 ECHO ──→ 1kΩ resistor ──→ ESP D3 (GPIO0)
                                └──→ 2kΩ resistor ──→ GND

Alternative (simpler):
  Power HC-SR04 from 3.3V instead of 5V
  Direct connection: ECHO → ESP D3 (no resistors)
```

### Step 5: GPS Module - NOT USED IN V2
```
❌ SKIP THIS STEP - GPS disabled in V2
GPS pins reallocated to solve conflicts
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

## ⚡ QUICK REFERENCE PINOUT - V2 (CONFLICT-FREE)

### ESP8266 NodeMCU D-Pin to GPIO Mapping:
```
D0  = GPIO16  (Motor Left PWM)           ✅ CHANGED - Dedicated
D1  = GPIO5   (I2C SDA)                  ✅ CHANGED - Dedicated I2C
D2  = GPIO4   (Sonar TRIG)               ✅ CHANGED from I2C SDA
D3  = GPIO0   (Sonar ECHO)               ✅ CHANGED - Dedicated INPUT
D4  = GPIO2   (Motor Left IN1 + LED)     ✅ CHANGED - LED OK to share
D5  = GPIO14  (Motor Left IN2)           ✅ CHANGED - Dedicated
D6  = GPIO12  (Motor Right PWM)          Same - Dedicated  
D7  = GPIO13  (Motor Right IN2)          ✅ NO CONFLICT! Dedicated
D8  = GPIO15  (Motor Right IN1)          ✅ CHANGED - Dedicated
RX  = GPIO3   (Available - not used)     GPS removed
TX  = GPIO1   (I2C SCL)                  ✅ CHANGED - Dedicated I2C
```

### V2 Pin Changes Summary:
| Old (V1) | New (V2) | Component | Reason |
|----------|----------|-----------|--------|
| D1 | D0 | Motor L PWM | Free D1 for I2C |
| D3 | D4 | Motor L IN1 | Accommodate shifts |
| D4 | D5 | Motor L IN2 | Accommodate shifts |
| D6 | D8 | Motor R IN1 | Better grouping |
| D8 | D2 | Sonar TRIG | Free D7/D8 |
| D7 | D3 | Sonar ECHO | Eliminate motor conflict |
| D2 | D1 | I2C SDA | Swap with SCL |
| D1 | TX | I2C SCL | Eliminate PWM interference |

### Color Code Suggestion:
- **Red** = Power (VCC, VIN, Battery +)
- **Black** = Ground (GND, Battery -)
- **Yellow** = Motor signals (PWM, IN1-IN4)
- **Blue** = I2C (SDA/SCL)
- **Green** = Sonar (TRIG/ECHO)
- **Orange** = PWM signals

---

## ✅ FINAL VERIFICATION - V2 CHECKLIST

Before deploying:
- [ ] All connections match V2 diagram (not V1!)
- [ ] Motor pins: D0, D4, D5, D6, D7, D8 → L298D
- [ ] Sonar pins: D2 (TRIG), D3 (ECHO with voltage divider)
- [ ] IMU pins: D1 (SDA), TX (SCL)
- [ ] No loose wires or shorts
- [ ] Motor power separate from logic power
- [ ] All grounds connected to common GND
- [ ] Voltage divider installed on sonar ECHO (if using 5V)
- [ ] Self-test passes (5/5 tests - no GPS/E-Stop)
  - ✅ Motors test PASSED
  - ✅ IMU test PASSED
  - ✅ Sonar test PASSED (not -1.0 cm!)
  - ✅ WiFi test PASSED (or optional in test mode)
  - ✅ Watchdog test PASSED
- [ ] Robot chassis secure
- [ ] Batteries charged
- [ ] Ready to roll! 🤖

---

**Your V2 robot is wired conflict-free and ready for testing!**

Expected startup log:
```
✓ Motors initialized
✓ IMU initialized  
✓ Sonar initialized
✓ WiFi connected (or optional)
✓ ALL TESTS PASSED
State: IDLE
```

For software setup, see [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md)

Good luck! 🚀
