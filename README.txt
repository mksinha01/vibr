# ESP8266 Delivery Robot - Arduino IDE Upload Instructions

## Required Libraries (Install via Arduino Library Manager)

1. **Adafruit MPU6050** (version 2.2.4 or higher)
   - Sketch → Include Library → Manage Libraries → Search "Adafruit MPU6050"
   
2. **Adafruit Unified Sensor**
   - Should auto-install with MPU6050
   
3. **Adafruit BusIO**
   - Should auto-install with MPU6050
   
4. **TinyGPSPlus** (version 1.0.3 or higher)
   - Search "TinyGPSPlus" in Library Manager
   
5. **ArduinoJson** (version 6.21.3 or higher)
   - Search "ArduinoJson" in Library Manager
   
6. **WebSockets** by Markus Sattler (version 2.4.1 or higher)
   - Search "WebSockets" in Library Manager
   
7. **EspSoftwareSerial**
   - Search "EspSoftwareSerial" in Library Manager

## Board Configuration

**Tools Menu Settings:**
- Board: "NodeMCU 1.0 (ESP-12E Module)"
- CPU Frequency: "80 MHz"
- Flash Size: "4MB (FS:1MB OTA:~1019KB)"
- Upload Speed: "115200" (slower = more reliable)
- Port: "COM7"

## Upload Instructions

1. Open the .ino file: `delivery_robot.ino`
2. Install all required libraries (see above)
3. Configure board settings (see above)
4. Click Upload button
5. **WHEN YOU SEE "Connecting..."** press and hold the FLASH button on your NodeMCU
6. Keep holding until upload starts (you'll see progress %)
7. Release button once upload is in progress

## After Upload

Open Serial Monitor (Ctrl+Shift+M) at 115200 baud to see the robot startup messages.

## Pin Configuration Notes

**2 Pins are Shared (acceptable tradeoffs):**
- D1 (GPIO5): I2C SCL + Motor Left PWM
- D7 (GPIO13): Sonar ECHO + Motor Right IN2

See config.h for full pin mapping.

## WiFi Configuration

Edit config.h to change WiFi credentials:
```cpp
#define WIFI_SSID "Your WiFi Name"
#define WIFI_PASSWORD "Your WiFi Password"
```
