# 🔧 TROUBLESHOOTING GUIDE - Delivery Robot Runtime Errors

## 🚨 CURRENT PROBLEM ANALYSIS

Based on your serial monitor output, your robot is experiencing these critical errors:

### Error Pattern (repeating every ~5 seconds):
```
[ERROR] WiFi connection failed!
[WARN ] SAFETY: System health degraded
[ERROR] SAFETY: Critical system health! Stopping robot.
[ERROR] EMERGENCY STOP ACTIVATED!
[WARN ] Suspicious distance jump: 400.0 -> 4.6 cm
[WARN ] Sonar component error reported
```

### Root Causes:

1. **WiFi Connection Failure**
   - Network "Mk's S24 FE" not available or wrong password
   - Watchdog marks WiFi component as unhealthy
   
2. **Sonar Sensor Errors**
   - Reading 400cm (timeout/max) then jumping to small values (4-50cm)
   - Indicates wiring issue or pin sharing conflict on D7 (GPIO13)
   
3. **System Health Critical**
   - 2+ components failing triggers safety system
   - Emergency stop activates (working correctly!)
   - Robot stuck in ERROR state loop

---

## ⚡ QUICK FIX - Get Running in 5 Minutes

### Option 1: Use Test Mode Configuration (RECOMMENDED)

This will let your robot run without WiFi and ignore sensor errors:

**Steps:**
1. Backup current config:
   ```
   In Arduino project folder, rename:
   config.h → config_production.h
   ```

2. Use test mode config:
   ```
   Rename:
   config_test_mode.h → config.h
   ```

3. Re-upload to ESP8266

4. Robot will now run with:
   - ✅ WiFi optional (won't cause errors)
   - ✅ Sonar optional (errors ignored)
   - ✅ Motors fully functional
   - ✅ IMU working
   - ✅ Slower speeds for safety

---

### Option 2: Quick Config Edits (Manual Fix)

If you want to keep your current config, add these lines at the top of `config.h`:

```cpp
// Add these after the #ifndef CONFIG_H line:

#define DISABLE_WIFI_REQUIREMENT         // Don't fail on WiFi errors
#define RELAX_SONAR_CHECKS              // Ignore sonar errors
#define DISABLE_WATCHDOG                // Disable watchdog for testing
```

Then re-upload.

---

## 🔍 DETAILED TROUBLESHOOTING

### Problem 1: WiFi Connection Failed

#### Symptoms:
```
[ERROR][wifi_comm.cpp] WiFi connection failed!
[WARN ][watchdog.cpp] WiFi component error reported
```

#### Diagnosis:
```powershell
# Check if WiFi network is visible to your computer
# On Windows PowerShell:
netsh wlan show networks

# Look for "Mk's S24 FE" in the list
```

#### Solutions (choose one):

**A. Fix WiFi Connection:**
1. Make sure your hotspot "Mk's S24 FE" is active
2. Verify password is exactly "00000000"
3. Check ESP8266 is within range
4. Try different WiFi channel (2.4GHz only for ESP8266!)

**B. Change WiFi Credentials in code:**
Edit [config.h](config.h#L99-L100):
```cpp
#define WIFI_SSID             "YourNetworkName"
#define WIFI_PASSWORD         "YourPassword"
```

**C. Disable WiFi (for testing):**
Edit [config.h](config.h#L183) and uncomment:
```cpp
#define DISABLE_WIFI_REQUIREMENT
```

**D. Use different network:**
- ESP8266 only works with 2.4GHz WiFi (NOT 5GHz!)
- Try your home WiFi instead of mobile hotspot

---

### Problem 2: Sonar Suspicious Distance Jumps

#### Symptoms:
```
[WARN ][sonar.cpp] Suspicious distance jump: 400.0 -> 4.6 cm
[WARN ][watchdog.cpp] Sonar component error reported
```

#### Root Cause:
Pin D7 (GPIO13) is **SHARED** between:
- Motor Right IN2 (direction control)
- Sonar ECHO (distance reading)

When motor changes direction, it interferes with sonar reading!

#### Diagnosis:

1. **Test Sonar Alone** (motors off):
   Add this in `setup()` after sonar initialization:
   ```cpp
   // Test sonar without motors
   for(int i = 0; i < 10; i++) {
       sonar.update();
       sonar.printData();
       delay(500);
   }
   ```

2. **Check Wiring:**
   ```
   HC-SR04 Connections:
   VCC  → 3.3V or 5V (check your module)
   TRIG → D8 (GPIO15)
   ECHO → D7 (GPIO13) ⚠️ SHARED PIN!
   GND  → GND
   
   ⚠️ If using 5V VCC, ECHO output is 5V!
   You MUST use voltage divider:
   ECHO → 1kΩ → D7
           └─ 2kΩ → GND
   ```

3. **Voltage Divider Check:**
   If your HC-SR04 is powered by 5V, its ECHO pin outputs 5V.
   ESP8266 pins are 3.3V max - **you will damage it without voltage divider!**

#### Solutions (choose one):

**A. Add Voltage Divider (if using 5V sonar):**
```
If your sonar VCC is connected to 5V:

         HC-SR04 ECHO
              │
              ├── 1kΩ resistor ──→ ESP8266 D7 (GPIO13)
              │
              └── 2kΩ resistor ──→ GND

This drops 5V to ~3.3V (safe for ESP8266)
```

**B. Use 3.3V Power for Sonar:**
```
HC-SR04 VCC → ESP8266 3V3 pin (instead of 5V)

Most HC-SR04 work with 3.3V, but range may be reduced.
```

**C. Disable Sonar Temporarily:**
In `config.h`, add:
```cpp
#define RELAX_SONAR_CHECKS    // Ignore sonar errors
```

Or in `delivery_robot.ino`, comment out sonar initialization:
```cpp
// sonar.init();    // Commented for testing
```

**D. Move Sonar to Different Pin (HARDWARE CHANGE):**
This requires rewiring:
```
Current: ECHO → D7 (conflicts with motor)
Better:  ECHO → A0 (analog pin, can read digital)

In config.h change:
#define SONAR_ECHO_PIN  A0  // Instead of 13
```

---

### Problem 3: System Health Critical → Emergency Stop Loop

#### Symptoms:
```
[WARN ] SAFETY: System health degraded
[ERROR] SAFETY: Critical system health! Stopping robot.
[ERROR] EMERGENCY STOP ACTIVATED!
```

#### Why This Happens (it's working correctly!):

The watchdog checks component health every second:
- WiFi: No heartbeat for >5 seconds → marked unhealthy ❌
- Sonar: Errors reported → marked unhealthy ❌
- IMU: OK ✅
- Motors: OK ✅

When 1+ non-critical components fail → Health = WARNING
- Safety system in `delivery_robot.ino` detects this
- Triggers emergency stop (protecting your robot!)

#### Solution:

**Configuration Change** - Make WiFi and Sonar optional:

Edit [watchdog.cpp](watchdog.cpp) around line 220 in `checkComponentHealth()`:

Change this:
```cpp
// CRITICAL components (robot cannot operate without these)
if (!health.motorHealthy) {
    criticalComponentsFailed++;
    failedComponents++;
}

// WARNING components (robot can operate with degraded performance)
if (!health.imuHealthy) failedComponents++;
if (!health.sonarHealthy) failedComponents++;
```

To this (for testing):
```cpp
// CRITICAL components (robot cannot operate without these)
if (!health.motorHealthy) {
    criticalComponentsFailed++;
    failedComponents++;
}

// WARNING components (robot can operate with degraded performance)
if (!health.imuHealthy) failedComponents++;

// TESTING MODE: Sonar and WiFi are OPTIONAL
// if (!health.sonarHealthy) failedComponents++;
// WiFi already not counted in failedComponents
```

Or better yet, use the test mode config which already does this!

---

## 🧪 SYSTEMATIC DEBUGGING PROCEDURE

Follow these steps in order:

### Step 1: Test Motors Only (1 minute)

**Goal:** Verify motors work without any sensors

1. Edit `delivery_robot.ino`, in `setup()` comment out everything except motors:
```cpp
void setup() {
    Serial.begin(115200);
    logger.init();
    
    motors.init();
    motors.test();  // This will make motors spin
    
    // Comment out everything else:
    // mpu.init();
    // sonar.init();
    // gps.init();
    // wifiComm.init();
    // etc...
    
    while(1) { delay(1000); }  // Stop here
}
```

2. Upload and check serial monitor
3. Motors should spin forward/backward for 5 seconds
4. **Expected:** Motors work ✅
5. **If motors don't work:** Check motor wiring and power supply

---

### Step 2: Add IMU (2 minutes)

**Goal:** Verify IMU works

Uncomment in `setup()`:
```cpp
mpu.init();
mpu.test();
```

**Expected Output:**
```
[INFO] MPU6050 initialized successfully
[INFO] IMU test complete
```

**If fails:** Check I2C wiring (SDA→D2, SCL→D1)

---

### Step 3: Test Sonar Independently (3 minutes)

**Goal:** See if sonar works when motors are NOT running

Uncomment:
```cpp
sonar.init();
sonar.test();
```

**Expected:** Distance readings between 2-400 cm

**If getting 400 → small jumps:**
- Check wiring
- Add voltage divider if using 5V
- Test with hand in front of sensor

---

### Step 4: Add WiFi (5 minutes)

**Goal:** Get WiFi connected

Uncomment:
```cpp
wifiComm.init();
```

**Fixes if doesn't connect:**
- Change WIFI_SSID and WIFI_PASSWORD in config.h
- Make sure 2.4GHz network (ESP8266 doesn't support 5GHz)
- Move ESP8266 closer to router

---

### Step 5: Run Full System (10 minutes)

Once each component works individually:
1. Uncomment all initialization code
2. Upload
3. System should now start without errors

---

## 🔌 HARDWARE VERIFICATION CHECKLIST

Print this and check each item:

### Power Supply:
- [ ] ESP8266 powered via USB (5V, 1A minimum)
- [ ] Motor battery connected to L298D 12V pin (6-12V, 2A minimum)
- [ ] **ALL grounds connected together** (ESP GND, L298D GND, Battery GND)
- [ ] L298D VCC (logic) connected to 5V
- [ ] Check voltage with multimeter: ESP 3.3V pin reads ~3.3V

### Motor Wiring:
- [ ] D1 → L298D ENA
- [ ] D3 → L298D IN1  
- [ ] D4 → L298D IN2
- [ ] D5 → L298D ENB
- [ ] D6 → L298D IN3
- [ ] D7 → L298D IN4
- [ ] Motors connected to OUT1-OUT4

### Sonar Wiring:
- [ ] D8 → TRIG
- [ ] D7 → ECHO (⚠️ shared with motor IN2)
- [ ] VCC → 3.3V or 5V (check which!)
- [ ] If 5V: Voltage divider on ECHO (1kΩ + 2kΩ)
- [ ] GND → Common ground

### IMU Wiring:
- [ ] D2 → SDA
- [ ] D1 → SCL (⚠️ shared with motor PWM)
- [ ] VCC → 3.3V
- [ ] GND → Common ground
- [ ] AD0 → GND (sets I2C address to 0x68)

---

## 📊 INTERPRETING ERROR MESSAGES

### Good Signs (Normal Operation):
```
[INFO] Motor controller initialized
[INFO] MPU6050 initialized successfully  
[INFO] Sonar initialized successfully
[INFO] State: IDLE
```

### Warning Signs (Can Continue):
```
[WARN] WiFi connection failed!           ← OK in test mode
[WARN] Low battery: 10.2V                ← Check battery
[WARN] Sonar timeout                     ← Temporary, OK
```

### Critical Signs (Must Fix):
```
[ERROR] Motor initialization failed!     ← Check wiring!
[ERROR] I2C device not found             ← IMU not connected
[ERROR] EMERGENCY STOP ACTIVATED!        ← Safety working
```

---

## 🚀 EXPECTED STARTUP SEQUENCE (Normal)

When working correctly, you should see:

```
=== ESP8266 DELIVERY ROBOT ===
Version: 1.0.0-esp8266

[    0.123][INFO] Initializing logger...
[    0.234][INFO] Initializing watchdog system...
[    0.345][INFO] Motor controller initialized
[    0.456][INFO] MPU6050 initialized successfully
[    0.567][INFO] Sonar initialized successfully
[    0.678][INFO] WiFi connecting to Mk's S24 FE...
[    3.123][INFO] WiFi connected! IP: 192.168.1.50
[    3.234][INFO] WebSocket connected
[    3.345][INFO] State machine initialized
[    3.456][INFO] === SYSTEM READY ===

[    5.000][INFO] State: IDLE
[    5.100][INFO] Sonar: 145.2 cm, clear path
```

**If you see the error loop instead, use test mode config!**

---

## 💡 COMMON MISTAKES

### 1. Connecting 5V to ESP8266 GPIO pins
**Problem:** ESP8266 is 3.3V logic - 5V will damage it!
**Solution:** Use voltage dividers for 5V sensors

### 2. Separate motor power not connected
**Problem:** Motors don't move or ESP8266 resets
**Solution:** Connect separate battery to L298D 12V pin

### 3. Grounds not connected
**Problem:** Erratic behavior, sensors don't work
**Solution:** Connect ALL grounds together

### 4. Using 5GHz WiFi
**Problem:** ESP8266 cannot see network
**Solution:** Use 2.4GHz WiFi only

### 5. Not enough motor power
**Problem:** Motors weak or don't move
**Solution:** Use 7-12V battery with 2A+ current

### 6. Pin sharing conflicts
**Problem:** Sonar errors when motors run
**Solution:** Use test mode or rewire sonar to different pin

---

## 📞 NEXT STEPS

### If Still Having Problems:

1. **Use Test Mode Config** (simplest solution)
   - Rename config_test_mode.h → config.h
   - Re-upload
   - Robot will work without WiFi/GPS

2. **Test Components One-by-One**
   - Follow Step 1-5 in Systematic Debugging
   - Identify which component is failing

3. **Check Hardware**
   - Verify all connections with multimeter
   - Check voltages (3.3V, 5V, motor voltage)
   - Look for loose wires

4. **Simplify Configuration**
   - Start with just motors
   - Add one sensor at a time
   - Verify each works before adding next

---

## 🎯 SUCCESS CRITERIA

Your robot is working when you see:

✅ No ERROR messages in serial monitor
✅ Motors can be controlled
✅ IMU provides tilt data
✅ Sonar reads distances correctly
✅ State machine transitions: INIT → IDLE → READY
✅ No emergency stops (unless you trigger them)

---

## 📋 QUICK REFERENCE - Error Solutions

| Error | Quick Fix |
|-------|-----------|
| WiFi connection failed | Use test mode config OR change WIFI_SSID/PASSWORD |
| Sonar suspicious jumps | Add voltage divider OR use 3.3V power |
| System health critical | Use test mode config OR make WiFi/Sonar optional |
| Motor init failed | Check motor wiring and power supply |
| I2C device not found | Check IMU wiring (SDA→D2, SCL→D1) |
| Emergency stop loop | Use test mode config |

---

**Remember:** Your safety system is working correctly! It's stopping the robot because WiFi and Sonar are failing. This is good - it means the robot won't run away if something is wrong.

**The solution:** Use test mode configuration to make these components optional during testing.

---

Good luck! 🤖🚀
