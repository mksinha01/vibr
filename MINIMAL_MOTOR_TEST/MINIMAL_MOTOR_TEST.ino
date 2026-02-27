// FIXED L298N WIRING - SAFE ESP8266 PINS
// ENA-D1, IN1-D4, IN2-D5, ENB-D6, IN3-D7, IN4-D2

void setup() {
    Serial.begin(115200);
    delay(3000);  // Long delay for stability
    
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║   FIXED L298N MOTOR TEST               ║");  
    Serial.println("╚════════════════════════════════════════╝");
    
    Serial.print("Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    
    Serial.print("Chip ID: 0x");
    Serial.println(ESP.getChipId(), HEX);
    
    // FIXED L298N WIRING - SAFE ESP8266 PINS
    const int MOTOR_LEFT_PWM = 5;    // D1 (GPIO5) - ENA (PWM WORKS!)
    const int MOTOR_LEFT_IN1 = 2;    // D4 (GPIO2) - IN1 (KEEP)
    const int MOTOR_LEFT_IN2 = 14;   // D5 (GPIO14) - IN2 (KEEP)
    const int MOTOR_RIGHT_PWM = 12;  // D6 (GPIO12) - ENB (KEEP)
    const int MOTOR_RIGHT_IN1 = 13;  // D7 (GPIO13) - IN3 (KEEP)
    const int MOTOR_RIGHT_IN2 = 4;   // D2 (GPIO4) - IN4 (SAFE PIN)
    
    // Initialize motor pins as outputs
    pinMode(MOTOR_LEFT_PWM, OUTPUT);
    pinMode(MOTOR_LEFT_IN1, OUTPUT);
    pinMode(MOTOR_LEFT_IN2, OUTPUT);
    pinMode(MOTOR_RIGHT_PWM, OUTPUT);
    pinMode(MOTOR_RIGHT_IN1, OUTPUT);
    pinMode(MOTOR_RIGHT_IN2, OUTPUT);
    
    // Stop all motors initially
    analogWrite(MOTOR_LEFT_PWM, 0);
    analogWrite(MOTOR_RIGHT_PWM, 0);
    digitalWrite(MOTOR_LEFT_IN1, LOW);
    digitalWrite(MOTOR_LEFT_IN2, LOW);
    digitalWrite(MOTOR_RIGHT_IN1, LOW);
    digitalWrite(MOTOR_RIGHT_IN2, LOW);
    
    Serial.println("✅ Motor pins initialized successfully");
    Serial.println("System ready for motor commands");
    Serial.println("Type 'help' for available commands");
    Serial.println();
}

void loop() {
    // FIXED L298N WIRING - SAFE ESP8266 PINS
    const int MOTOR_LEFT_PWM = 5;    // D1 (GPIO5) - ENA (PWM WORKS!)
    const int MOTOR_LEFT_IN1 = 2;    // D4 (GPIO2) - IN1 (KEEP)
    const int MOTOR_LEFT_IN2 = 14;   // D5 (GPIO14) - IN2 (KEEP)
    const int MOTOR_RIGHT_PWM = 12;  // D6 (GPIO12) - ENB (KEEP)
    const int MOTOR_RIGHT_IN1 = 13;  // D7 (GPIO13) - IN3 (KEEP)
    const int MOTOR_RIGHT_IN2 = 4;   // D2 (GPIO4) - IN4 (SAFE PIN)
    
    // Simple serial command processing
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();
        
        if (command == "help") {
            Serial.println("Available commands:");
            Serial.println("  forward - Move forward 2 seconds");
            Serial.println("  backward - Move backward 2 seconds");
            Serial.println("  left - Turn left 1 second");
            Serial.println("  right - Turn right 1 second");
            Serial.println("  stop - Stop all motors");
            Serial.println("  status - Show system status");
            Serial.println("  testleft - Test left motor only");
            Serial.println("  testright - Test right motor only");
            Serial.println();
            
        } else if (command == "forward") {
            Serial.println("Moving forward...");
            // Both motors forward
            digitalWrite(MOTOR_LEFT_IN1, HIGH);
            digitalWrite(MOTOR_LEFT_IN2, LOW);
            digitalWrite(MOTOR_RIGHT_IN1, HIGH);
            digitalWrite(MOTOR_RIGHT_IN2, LOW);
            analogWrite(MOTOR_LEFT_PWM, 150);
            analogWrite(MOTOR_RIGHT_PWM, 150);
            delay(2000);
            // Stop
            analogWrite(MOTOR_LEFT_PWM, 0);
            analogWrite(MOTOR_RIGHT_PWM, 0);
            Serial.println("Forward complete");
            Serial.println();
            
        } else if (command == "backward") {
            Serial.println("Moving backward...");
            // Both motors backward
            digitalWrite(MOTOR_LEFT_IN1, LOW);
            digitalWrite(MOTOR_LEFT_IN2, HIGH);
            digitalWrite(MOTOR_RIGHT_IN1, LOW);
            digitalWrite(MOTOR_RIGHT_IN2, HIGH);
            analogWrite(MOTOR_LEFT_PWM, 150);
            analogWrite(MOTOR_RIGHT_PWM, 150);
            delay(2000);
            // Stop
            analogWrite(MOTOR_LEFT_PWM, 0);
            analogWrite(MOTOR_RIGHT_PWM, 0);
            Serial.println("Backward complete");
            Serial.println();
            
        } else if (command == "left") {
            Serial.println("Turning left...");
            // Left motor backward, right motor forward
            digitalWrite(MOTOR_LEFT_IN1, LOW);
            digitalWrite(MOTOR_LEFT_IN2, HIGH);
            digitalWrite(MOTOR_RIGHT_IN1, HIGH);
            digitalWrite(MOTOR_RIGHT_IN2, LOW);
            analogWrite(MOTOR_LEFT_PWM, 120);
            analogWrite(MOTOR_RIGHT_PWM, 120);
            delay(1000);
            // Stop
            analogWrite(MOTOR_LEFT_PWM, 0);
            analogWrite(MOTOR_RIGHT_PWM, 0);
            Serial.println("Left turn complete");
            Serial.println();
            
        } else if (command == "right") {
            Serial.println("Turning right...");
            // Left motor forward, right motor backward
            digitalWrite(MOTOR_LEFT_IN1, HIGH);
            digitalWrite(MOTOR_LEFT_IN2, LOW);
            digitalWrite(MOTOR_RIGHT_IN1, LOW);
            digitalWrite(MOTOR_RIGHT_IN2, HIGH);
            analogWrite(MOTOR_LEFT_PWM, 120);
            analogWrite(MOTOR_RIGHT_PWM, 120);
            delay(1000);
            // Stop
            analogWrite(MOTOR_LEFT_PWM, 0);
            analogWrite(MOTOR_RIGHT_PWM, 0);
            Serial.println("Right turn complete");
            Serial.println();
            
        } else if (command == "testleft") {
            Serial.println("Testing LEFT motor only...");
            // Only left motor forward
            digitalWrite(MOTOR_LEFT_IN1, HIGH);
            digitalWrite(MOTOR_LEFT_IN2, LOW);
            analogWrite(MOTOR_LEFT_PWM, 150);
            delay(2000);
            analogWrite(MOTOR_LEFT_PWM, 0);
            Serial.println("Left motor test complete");
            Serial.println();
            
        } else if (command == "testright") {
            Serial.println("Testing RIGHT motor only...");
            // Only right motor forward
            digitalWrite(MOTOR_RIGHT_IN1, HIGH);
            digitalWrite(MOTOR_RIGHT_IN2, LOW);
            analogWrite(MOTOR_RIGHT_PWM, 150);
            delay(2000);
            analogWrite(MOTOR_RIGHT_PWM, 0);
            Serial.println("Right motor test complete");
            Serial.println();
            
        } else if (command == "stop") {
            Serial.println("Emergency stop - all motors off");
            analogWrite(MOTOR_LEFT_PWM, 0);
            analogWrite(MOTOR_RIGHT_PWM, 0);
            digitalWrite(MOTOR_LEFT_IN1, LOW);
            digitalWrite(MOTOR_LEFT_IN2, LOW);
            digitalWrite(MOTOR_RIGHT_IN1, LOW);
            digitalWrite(MOTOR_RIGHT_IN2, LOW);
            Serial.println();
            
        } else if (command == "status") {
            Serial.println("System Status:");
            Serial.print("  Free Heap: ");
            Serial.print(ESP.getFreeHeap());
            Serial.println(" bytes");
            Serial.print("  Uptime: ");
            Serial.print(millis());
            Serial.println(" ms");
            Serial.println("  Motor pins (FIXED L298N WIRING):");
            Serial.println("    LEFT: D1(ENA), D4(IN1), D5(IN2)");
            Serial.println("    RIGHT: D6(ENB), D7(IN3), D2(IN4)");
            Serial.println();
            
        } else if (command.length() > 0) {
            Serial.print("Unknown command: ");
            Serial.println(command);
            Serial.println("Type 'help' for available commands");
            Serial.println();
        }
    }
    
    delay(10);  // Small delay for stability
}