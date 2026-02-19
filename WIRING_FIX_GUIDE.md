# 🔌 WIRING FIX GUIDE - Before & After

## 🔴 CURRENT BROKEN WIRING

```
ESP8266 NodeMCU          L298D Motor Driver
┌─────────────────┐      ┌──────────────┐
│                 │      │              │
│  D1 (GPIO5)  ●──┼──────┼──● ENA       │  ❌ CONFLICT: PWM + I2C SCL
│  D3 (GPIO0)  ●──┼──────┼──● IN1       │
│  D4 (GPIO2)  ●──┼──────┼──● IN2       │  ← Will move to D5
│  D6 (GPIO12) ●──┼──────┼──● ENB       │
│  D8 (GPIO15) ●──┼──────┼──● IN3       │
│  D7 (GPIO13) ●──┼──────┼──● IN4       │  ❌ CONFLICT: Motor + Sonar Echo
│                 │      └──────────────┘
│                 │      
│                 │      HC-SR04 Sonar
│                 │      ┌──────────────┐
│  D8 (GPIO15) ●──┼──────┼──● TRIG      │  ← Will move to D2
│  D7 (GPIO13) ●──┼──────┼──● ECHO      │  ❌ CONFLICT: Shares with Motor IN4
│                 │      └──────────────┘
│                 │      
│                 │      MPU6050 IMU
│                 │      ┌──────────────┐
│  D2 (GPIO4)  ●──┼──────┼──● SDA       │  ← Will swap with SCL
│  D1 (GPIO5)  ●──┼──────┼──● SCL       │  ❌ CONFLICT: Shares with Motor PWM
└─────────────────┘      └──────────────┘
```

---

## ✅ FIXED WIRING (NO CONFLICTS)

```
ESP8266 NodeMCU          L298D Motor Driver
┌─────────────────┐      ┌──────────────┐
│                 │      │              │
│  D0 (GPIO16) ●──┼──────┼──● ENA       │  ✅ MOVED: Dedicated Motor PWM
│  D4 (GPIO2)  ●──┼──────┼──● IN1       │
│  D5 (GPIO14) ●──┼──────┼──● IN2       │  ✅ MOVED: No conflict
│  D6 (GPIO12) ●──┼──────┼──● ENB       │
│  D8 (GPIO15) ●──┼──────┼──● IN3       │
│  D7 (GPIO13) ●──┼──────┼──● IN4       │  ✅ KEPT: Dedicated motor control
│                 │      └──────────────┘
│                 │      
│                 │      HC-SR04 Sonar
│                 │      ┌──────────────┐────────┐
│  D2 (GPIO4)  ●──┼──────┼──● TRIG      │        │  ✅ MOVED: Dedicated trigger
│                 │      │              │        │
│  D3 (GPIO0)  ●──┼──────┼──● ECHO ──── 1kΩ ────┤  ✅ MOVED: Dedicated + voltage divider
│                 │      └──────────────┘   │    │
│                 │                          2kΩ  │
│                 │                          │    │
│  GND         ●──┼──────────────────────────┴────┘
│                 │      
│                 │      MPU6050 IMU
│                 │      ┌──────────────┐
│  D1 (GPIO5)  ●──┼──────┼──● SDA       │  ✅ SWAPPED: Dedicated I2C
│  TX (GPIO1)  ●──┼──────┼──● SCL       │  ✅ MOVED: Dedicated I2C clock
└─────────────────┘      └──────────────┘
```

---

## 🔧 DETAILED REWIRING STEPS

### 1️⃣ L298D Motor Driver Connections

#### Current → Fixed Changes

| Connection | Old Pin | New Pin | Wire Action |
|------------|---------|---------|-------------|
| ENA (Left Motor PWM) | D1 (GPIO5) | **D0 (GPIO16)** | **MOVE WIRE** |
| IN1 (Left Motor) | D3 (GPIO0) | D4 (GPIO2) | **MOVE WIRE** |
| IN2 (Left Motor) | D4 (GPIO2) | **D5 (GPIO14)** | **MOVE WIRE** |
| ENB (Right Motor PWM) | D6 (GPIO12) | D6 (GPIO12) | Keep same |
| IN3 (Right Motor) | D8 (GPIO15) | D8 (GPIO15) | Keep same |
| IN4 (Right Motor) | D7 (GPIO13) | D7 (GPIO13) | Keep same |

**Full L298D Wiring (After Fix):**
```
ESP8266          L298D          Motors
────────────────────────────────────────
D0 (GPIO16) ──→  ENA     OUT1 ──→ Left Motor +
D4 (GPIO2)  ──→  IN1     OUT2 ──→ Left Motor -
D5 (GPIO14) ──→  IN2
                           
D6 (GPIO12) ──→  ENB     OUT3 ──→ Right Motor +
D8 (GPIO15) ──→  IN3     OUT4 ──→ Right Motor -
D7 (GPIO13) ──→  IN4

GND ────────────→ GND
5V/VIN ─────────→ +12V (motor power supply)
```

---

### 2️⃣ HC-SR04 Sonar Sensor Connections

#### Current → Fixed Changes

| Connection | Old Pin | New Pin | Wire Action |
|------------|---------|---------|-------------|
| VCC | 5V | 5V | Keep same |
| TRIG | D8 (GPIO15) | **D2 (GPIO4)** | **MOVE WIRE** |
| ECHO | D7 (GPIO13) | **D3 (GPIO0) + divider** | **MOVE WIRE + ADD RESISTORS** |
| GND | GND | GND | Keep same |

**⚠️ CRITICAL: Voltage Divider Required for ECHO Pin**

The HC-SR04 outputs 5V on ECHO, but ESP8266 GPIOs are 3.3V tolerant only.

**Circuit:**
```
HC-SR04                ESP8266
┌────────┐             ┌────────┐
│  ECHO  ├─── 1kΩ ────┤ D3     │
└────────┘        │    └────────┘
                  │
                 2kΩ
                  │
                 GND

Formula: 5V × (2kΩ / (1kΩ + 2kΩ)) = 3.33V ✓
```

**Full HC-SR04 Wiring (After Fix):**
```
HC-SR04         Resistors       ESP8266
────────────────────────────────────────
VCC ─────────────────────────→ 5V (or 3.3V safer)
TRIG ────────────────────────→ D2 (GPIO4)
ECHO ──→ 1kΩ ──→ Junction ───→ D3 (GPIO0)
               └→ 2kΩ ──→ GND
GND ─────────────────────────→ GND
```

**Alternative (Safer):** Power HC-SR04 from 3.3V instead of 5V:
- Connect VCC to 3.3V pin instead of 5V
- Remove voltage divider (direct ECHO → D3)
- Range may be reduced slightly (still works for <3m)

---

### 3️⃣ MPU6050 IMU Sensor Connections

#### Current → Fixed Changes

| Connection | Old Pin | New Pin | Wire Action |
|------------|---------|---------|-------------|
| VCC | 3.3V | 3.3V | Keep same |
| SDA | D2 (GPIO4) | **D1 (GPIO5)** | **MOVE WIRE** |
| SCL | D1 (GPIO5) | **TX (GPIO1)** | **MOVE WIRE** |
| GND | GND | GND | Keep same |

**⚠️ NOTE:** Moving SCL to TX (GPIO1) means:
- **Serial debugging will NOT work** when IMU is connected
- You must rely on WiFi logging or remove IMU temporarily for debugging
- Alternative: Use software I2C library to use different pins (slower)

**Full MPU6050 Wiring (After Fix):**
```
MPU6050         ESP8266
────────────────────────
VCC ──────────→ 3.3V
SDA ──────────→ D1 (GPIO5)
SCL ──────────→ TX (GPIO1)  ⚠️ Serial debug disabled
GND ──────────→ GND
```

---

## 📋 COMPLETE WIRING TABLE (AFTER FIX)

### ESP8266 NodeMCU Pin Allocation

| ESP Pin | GPIO | Function | Connects To | Notes |
|---------|------|----------|-------------|-------|
| D0 | 16 | Motor L PWM | L298D ENA | PWM capable |
| TX | 1 | I2C SCL | MPU6050 SCL | ⚠️ No serial debug |
| D1 | 5 | I2C SDA | MPU6050 SDA | Dedicated I2C |
| D2 | 4 | Sonar TRIG | HC-SR04 TRIG | OUTPUT |
| D3 | 0 | Sonar ECHO | HC-SR04 ECHO (via divider) | INPUT |
| D4 | 2 | Motor L IN1 | L298D IN1 | Built-in LED |
| D5 | 14 | Motor L IN2 | L298D IN2 | |
| D6 | 12 | Motor R PWM | L298D ENB | PWM capable |
| D7 | 13 | Motor R IN2 | L298D IN4 | |
| D8 | 15 | Motor R IN1 | L298D IN3 | |
| 3V3 | - | Power | All sensors VCC | 3.3V supply |
| 5V | - | Power | HC-SR04 VCC (optional) | 5V supply |
| GND | - | Ground | All GND | Common ground |

---

## 🛠️ PHYSICAL REWIRING CHECKLIST

### Before You Start:
- [ ] Power OFF ESP8266 and disconnect USB
- [ ] Take a photo of current wiring for reference
- [ ] Have tools ready: wire cutters, jumper wires, resistors (1kΩ, 2kΩ)

### Rewiring Steps:

#### **L298D Motor Driver:**
- [ ] Remove D1 wire from ENA → reconnect D0 to ENA
- [ ] Remove D3 wire from IN1 → reconnect D4 to IN1
- [ ] Remove D4 wire from IN2 → reconnect D5 to IN2
- [ ] Verify D6, D7, D8 connections unchanged

#### **HC-SR04 Sonar:**
- [ ] Remove D8 wire from TRIG → reconnect D2 to TRIG
- [ ] Remove D7 wire from ECHO
- [ ] Build voltage divider: ECHO → 1kΩ → junction → D3 + (junction → 2kΩ → GND)
  - OR use 3.3V power: HC-SR04 VCC to 3.3V instead of 5V
  - Then direct connection: ECHO → D3 (no resistors needed)

#### **MPU6050 IMU:**
- [ ] Remove D2 wire from SDA → reconnect D1 to SDA
- [ ] Remove D1 wire from SCL → reconnect TX to SCL

#### **Final Checks:**
- [ ] All connections tight and secure
- [ ] No wires touching or shorting
- [ ] Voltage divider resistor values correct (if used)
- [ ] All grounds connected to common GND

---

## ⚡ TESTING PROCEDURE

### Step 1: Visual Inspection
Before powering on:
- [ ] No loose wires
- [ ] No shorts between pins
- [ ] Voltage divider correctly assembled (if used)
- [ ] All sensors have power and ground

### Step 2: Upload Fixed Code
```powershell
# In project folder
Copy-Item config_fixed_pins.h config.h
```
- [ ] Arduino IDE: Verify code compiles
- [ ] Upload to ESP8266
- [ ] Open Serial Monitor (115200 baud)

### Step 3: Verify Startup Tests
Watch for these success messages:
```
✓ Motors initialized
✓ Motors test PASSED
✓ IMU initialized  
✓ IMU test PASSED
✓ Sonar initialized
✓ Sonar test PASSED
✓ Watchdog initialized
✓ Watchdog test PASSED
────────────────────────
✅ ALL TESTS PASSED
State: IDLE
```

### Step 4: Individual Component Tests

**Motors:**
```
Expected: Both motors respond to commands
Watch: No unusual noise, smooth operation
```

**IMU:**
```
Expected: Tilt sensor readings appear (pitch/roll)
Watch: Values change when robot tilted
```

**Sonar:**
```
Expected: Distance readings 0-400cm (not -1.0)
Watch: Values change when hand moved in front
```

---

## 🐛 TROUBLESHOOTING NEW WIRING

### ❌ "Motors test FAILED"
- Check D0, D4, D5, D6, D7, D8 connections to L298D
- Verify motor power supply connected (6-12V)
- Common ground between ESP8266 and L298D

### ❌ "IMU test FAILED" or "not initialized"
- Check D1 (SDA) and TX (SCL) connections to MPU6050
- Verify MPU6050 powered from 3.3V (not 5V)
- Try adding 4.7kΩ pull-up resistors on SDA and SCL lines

### ❌ "Sonar test FAILED" or still showing "-1.0 cm"
- Check D2 (TRIG) and D3 (ECHO) connections
- Verify voltage divider: 1kΩ between ECHO and D3, 2kΩ from junction to GND
- OR verify HC-SR04 VCC connected to 3.3V (if using direct connection)

### ✅ All tests pass but robot doesn't move:
- This means wiring is correct!
- Check WiFi connection or use test mode
- Verify WebSocket server running at 192.168.1.100:8000

---

## 📸 WIRING REFERENCE PHOTOS

*Take these photos after fixing wiring:*
1. **Full robot overview** - all connections visible
2. **L298D connections** - close-up of all 6 motor wires
3. **HC-SR04 with voltage divider** - close-up of resistors
4. **MPU6050 I2C connections** - close-up of SDA/SCL

These photos help troubleshooting later!

---

## ⏭️ NEXT STEPS AFTER SUCCESSFUL WIRING

1. **All tests pass:** Robot ready for operation!
   - Configure WiFi network or enable test mode
   - Deploy and test basic movement commands

2. **Some tests still fail:** Debug specific components
   - See TROUBLESHOOTING.md for detailed diagnostics
   - Test each component individually

3. **Everything works:** Switch to production config
   - Copy `config_fixed_pins.h` to `config_production.h`
   - Adjust speeds and thresholds for your environment
   - Enable full safety features

---

**🎯 GOAL:** Transform error-filled startup into clean test passes by fixing impossible pin conflicts!
