# 🔌 V2 WIRING QUICK REFERENCE CARD

Print this page and keep it handy while rewiring!

---

## 📍 PIN CHANGES AT A GLANCE

### L298D Motor Driver
| Connection | V1 Pin | → | V2 Pin | ESP GPIO |
|------------|--------|---|--------|----------|
| ENA (Left PWM) | **D1** | → | **D0** | GPIO16 |
| IN1 (Left Dir) | **D3** | → | **D4** | GPIO2 |
| IN2 (Left Dir) | **D4** | → | **D5** | GPIO14 |
| ENB (Right PWM) | **D5** | → | **D6** | GPIO12 |
| IN3 (Right Dir) | **D6** | → | **D8** | GPIO15 |
| IN4 (Right Dir) | D7 | = | D7 | GPIO13 ✅ |

### HC-SR04 Sonar
| Connection | V1 Pin | → | V2 Pin | ESP GPIO |
|------------|--------|---|--------|----------|
| TRIG | **D8** | → | **D2** | GPIO4 |
| ECHO | **D7** | → | **D3** + divider | GPIO0 |
| VCC | 5V | = | 5V or 3.3V | - |
| GND | GND | = | GND | - |

### MPU6050 IMU
| Connection | V1 Pin | → | V2 Pin | ESP GPIO |
|------------|--------|---|--------|----------|
| SDA | **D2** | → | **D1** | GPIO5 |
| SCL | **D1** | → | **TX** | GPIO1 |
| VCC | 3.3V | = | 3.3V | - |
| GND | GND | = | GND | - |

### GPS (Neo-6M)
| Connection | V1 Pin | → | V2 Pin |
|------------|--------|---|--------|
| RX | RX | → | ❌ Not used |
| TX | TX | → | ❌ Not used |

---

## 🔧 VOLTAGE DIVIDER FOR SONAR

**If using 5V power for HC-SR04:**

```
HC-SR04         Resistors       ESP8266
┌──────┐                        ┌────────┐
│ ECHO ├──→ 1kΩ ──→ Junction ──→ D3     │
└──────┘            │           └────────┘
                    ↓
                   2kΩ
                    │
                   GND
```

**Components needed:**
- 1x 1kΩ resistor (brown-black-red)
- 1x 2kΩ resistor (red-black-red)

**Alternative (simpler):**
- Power HC-SR04 from 3.3V instead
- Direct connection: ECHO → D3
- No resistors needed!

---

## 📋 REWIRING CHECKLIST

### Step 1: Disconnect Old Wiring
- [ ] D1 (2 wires: Motor PWM + I2C SCL)
- [ ] D2 wire (I2C SDA)
- [ ] D3 wire (Motor IN1)
- [ ] D4 wire (Motor IN2)
- [ ] D5 wire (Motor PWM)
- [ ] D6 wire (Motor IN3)
- [ ] D7 (2 wires: Motor IN4 + Sonar ECHO)
- [ ] D8 wire (Sonar TRIG)
- [ ] GPS module (all 4 wires)

### Step 2: Connect V2 Wiring

**Motors (6 wires to L298D):**
- [ ] D0 → L298D ENA
- [ ] D4 → L298D IN1
- [ ] D5 → L298D IN2
- [ ] D6 → L298D ENB
- [ ] D7 → L298D IN4
- [ ] D8 → L298D IN3
- [ ] GND → L298D GND

**Sonar (4 connections):**
- [ ] D2 → HC-SR04 TRIG
- [ ] D3 → HC-SR04 ECHO (via voltage divider!)
- [ ] 5V or 3.3V → HC-SR04 VCC
- [ ] GND → HC-SR04 GND

**IMU (4 wires to MPU6050):**
- [ ] D1 → MPU6050 SDA
- [ ] TX → MPU6050 SCL
- [ ] 3.3V → MPU6050 VCC
- [ ] GND → MPU6050 GND
- [ ] MPU6050 AD0 → GND

### Step 3: Final Checks
- [ ] No wires on old positions (D2, D3, old D1, old D8)
- [ ] All grounds connected together
- [ ] Voltage divider correct (if using 5V sonar)
- [ ] No loose wires or shorts
- [ ] Motor power connected (6-12V battery)

---

## 🎯 EXPECTED RESULTS

**Startup Serial Output:**
```
✓ Motors initialized
✓ IMU initialized
✓ Sonar initialized, distance: XX.X cm
✓ WiFi connected
✓ ALL TESTS PASSED
State: IDLE
```

**If you see:**
- ❌ `IMU test FAILED` → Check D1 (SDA) and TX (SCL)
- ❌ `Sonar test FAILED` → Check D2 (TRIG) and D3 (ECHO)
- ❌ `Sonar distance: -1.0 cm` → ECHO not connected or no divider
- ❌ `Motors test FAILED` → Check all 6 motor pins

---

## 📐 BREADBOARD LAYOUT HELPER

```
ESP8266 NodeMCU PIN LABELS (Top View)

    [D0]  [D1]  [D2]  [D3]  [D4]  [D5]  [D6]  [D7]  [D8]
     │     │     │     │     │     │     │     │     │
     ↓     ↓     ↓     ↓     ↓     ↓     ↓     ↓     ↓
   Motor  I2C  Sonar Sonar Motor Motor Motor Motor Motor
    L     SDA  TRIG  ECHO   L     L     R     R     R
   PWM                      IN1   IN2   PWM   IN4   IN3


    [TX]  [RX]  [GND] [3V3] [VIN]
     │     │     │     │     │
     ↓     ↓     ↓     ↓     ↓
    I2C   Free  All   IMU   5V
    SCL        GNDs  Power Power
```

---

## 🔍 TROUBLESHOOTING QUICK TIPS

**Sonar always -1.0 cm:**
→ D3 (ECHO) not connected or voltage divider missing

**IMU not found:**
→ Swap D1 and TX connections (SDA/SCL reversed)

**Motors don't move:**
→ Check D0 (not D1!) for PWM signal

**Random emergency stops:**
→ All tests should pass first before deploying

---

**🎉 V2: Zero Pin Conflicts, 100% Reliability!**

Keep this card for reference while wiring! 📌
