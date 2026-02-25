// V2 HARDWARE TEST - Upload this to diagnose wiring issues
// This tests each component individually to find wiring problems

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║   V2 HARDWARE DIAGNOSTICS TEST        ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // V2 Pin Definitions
  #define MOTOR_LEFT_PWM      16   // D0
  #define MOTOR_LEFT_IN1      2    // D4
  #define MOTOR_LEFT_IN2      14   // D5
  #define MOTOR_RIGHT_PWM     12   // D6
  #define MOTOR_RIGHT_IN1     15   // D8
  #define MOTOR_RIGHT_IN2     13   // D7
  
  #define SONAR_TRIGGER_PIN   4    // D2
  #define SONAR_ECHO_PIN      0    // D3
  
  #define I2C_SDA             3    // RX (GPS disabled, conflict-free)
  #define I2C_SCL             1    // TX (GPS disabled, conflict-free)
  
  // Initialize motor pins
  pinMode(MOTOR_LEFT_PWM, OUTPUT);
  pinMode(MOTOR_LEFT_IN1, OUTPUT);
  pinMode(MOTOR_LEFT_IN2, OUTPUT);
  pinMode(MOTOR_RIGHT_PWM, OUTPUT);
  pinMode(MOTOR_RIGHT_IN1, OUTPUT);
  pinMode(MOTOR_RIGHT_IN2, OUTPUT);
  
  // Initialize sonar pins
  pinMode(SONAR_TRIGGER_PIN, OUTPUT);
  pinMode(SONAR_ECHO_PIN, INPUT);
  
  Serial.println("All pins initialized\n");
  
  // Test 1: Left Motor
  Serial.println("═══════════════════════════════════════");
  Serial.println("TEST 1: LEFT MOTOR (should spin for 2 seconds)");
  Serial.println("═══════════════════════════════════════");
  Serial.println("Pins: D0(PWM), D4(IN1), D5(IN2)");
  Serial.println("Action: Forward rotation...");
  
  digitalWrite(MOTOR_LEFT_IN1, HIGH);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  analogWrite(MOTOR_LEFT_PWM, 600);  // Medium speed
  delay(2000);
  analogWrite(MOTOR_LEFT_PWM, 0);
  
  Serial.println("Did LEFT motor spin? (Y/N)\n");
  delay(3000);
  
  // Test 2: Right Motor
  Serial.println("═══════════════════════════════════════");
  Serial.println("TEST 2: RIGHT MOTOR (should spin for 2 seconds)");
  Serial.println("═══════════════════════════════════════");
  Serial.println("Pins: D6(PWM), D8(IN1), D7(IN2)");
  Serial.println("Action: Forward rotation...");
  
  digitalWrite(MOTOR_RIGHT_IN1, HIGH);
  digitalWrite(MOTOR_RIGHT_IN2, LOW);
  analogWrite(MOTOR_RIGHT_PWM, 600);  // Medium speed
  delay(2000);
  analogWrite(MOTOR_RIGHT_PWM, 0);
  
  Serial.println("Did RIGHT motor spin? (Y/N)\n");
  delay(3000);
  
  // Test 3: Both Motors
  Serial.println("═══════════════════════════════════════");
  Serial.println("TEST 3: BOTH MOTORS (should spin for 2 seconds)");
  Serial.println("═══════════════════════════════════════");
  Serial.println("Action: Both motors forward...");
  
  digitalWrite(MOTOR_LEFT_IN1, HIGH);
  digitalWrite(MOTOR_LEFT_IN2, LOW);
  digitalWrite(MOTOR_RIGHT_IN1, HIGH);
  digitalWrite(MOTOR_RIGHT_IN2, LOW);
  analogWrite(MOTOR_LEFT_PWM, 600);
  analogWrite(MOTOR_RIGHT_PWM, 600);
  delay(2000);
  analogWrite(MOTOR_LEFT_PWM, 0);
  analogWrite(MOTOR_RIGHT_PWM, 0);
  
  Serial.println("Did BOTH motors spin? (Y/N)\n");
  delay(3000);
  
  // Test 4: Sonar
  Serial.println("═══════════════════════════════════════");
  Serial.println("TEST 4: SONAR SENSOR (10 readings)");
  Serial.println("═══════════════════════════════════════");
  Serial.println("Pins: D2(TRIG), D3(ECHO)");
  Serial.println("Put your hand in front of sensor (20-50cm)\n");
  
  delay(2000);
  
  for (int i = 0; i < 10; i++) {
    // Trigger pulse
    digitalWrite(SONAR_TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(SONAR_TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(SONAR_TRIGGER_PIN, LOW);
    
    // Read echo
    long duration = pulseIn(SONAR_ECHO_PIN, HIGH, 30000);
    float distance = duration * 0.034 / 2;
    
    Serial.print("Reading ");
    Serial.print(i + 1);
    Serial.print(": ");
    if (distance > 0 && distance < 400) {
      Serial.print(distance);
      Serial.println(" cm ✓");
    } else {
      Serial.println("-1.0 cm ✗ (timeout - check ECHO wire and voltage divider!)");
    }
    
    delay(500);
  }
  
  Serial.println("\n═══════════════════════════════════════");
  Serial.println("TEST 5: I2C SCANNER (MPU6050 should be 0x68)");
  Serial.println("═══════════════════════════════════════");
  Serial.println("Pins: RX(SDA), TX(SCL)");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);
  
  Serial.println("\nScanning I2C bus...");
  byte count = 0;
  
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      if (address == 0x68) {
        Serial.println(" ✓ (MPU6050 IMU - CORRECT!)");
      } else {
        Serial.println(" (unknown device)");
      }
      count++;
    }
  }
  
  if (count == 0) {
    Serial.println("No I2C devices found ✗");
    Serial.println("Check: RX→SDA, TX→SCL, 3.3V power, GND");
  }
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   DIAGNOSTICS COMPLETE                ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println("\nREPORT YOUR RESULTS:");
  Serial.println("1. Which motor(s) spun?");
  Serial.println("2. Did sonar show distances or -1.0?");
  Serial.println("3. Was MPU6050 found at 0x68?");
}

void loop() {
  // Empty - run diagnostics once on startup
}