#include "motors.h"

static const char* TAG = "Motors";

Motors::Motors()
    : currentLeftSpeed(0)
    , currentRightSpeed(0)
    , leftDirection(MOTOR_STOP)
    , rightDirection(MOTOR_STOP)
    , emergencyStopActive(false)
    , lastCommandTime(0)
    , commandStartTime(0)
    , commandDuration(0)
    , totalDistance(0)
    , leftRotations(0)
    , rightRotations(0) {
}

bool Motors::init() {
    LOGI("Initializing motor control system...");
    
    // Configure motor pins
    pinMode(MOTOR_LEFT_IN1, OUTPUT);
    pinMode(MOTOR_LEFT_IN2, OUTPUT);
    pinMode(MOTOR_RIGHT_IN1, OUTPUT);
    pinMode(MOTOR_RIGHT_IN2, OUTPUT);
    
    // Configure PWM pins (ESP8266 uses analogWrite 0-1023)
    pinMode(MOTOR_LEFT_PWM, OUTPUT);
    pinMode(MOTOR_RIGHT_PWM, OUTPUT);
    
    // Set PWM frequency (ESP8266)
    analogWriteFreq(MOTOR_PWM_FREQUENCY);
    analogWriteRange(1023); // ESP8266 PWM range
    
    // Initialize to stopped state
    stop();
    
    LOGI("Motor system initialized successfully");
    return true;
}

void Motors::test() {
    LOGI("Starting motor test sequence...");
    
    // Test left motor forward
    LOGI("Testing LEFT motor forward");
    setLeftMotor(100);
    delay(1000);
    stop();
    delay(500);
    
    // Test left motor backward
    LOGI("Testing LEFT motor backward");
    setLeftMotor(-100);
    delay(1000);
    stop();
    delay(500);
    
    // Test right motor forward
    LOGI("Testing RIGHT motor forward");
    setRightMotor(100);
    delay(1000);
    stop();
    delay(500);
    
    // Test right motor backward
    LOGI("Testing RIGHT motor backward");
    setRightMotor(-100);
    delay(1000);
    stop();
    delay(500);
    
    // Test both motors forward
    LOGI("Testing BOTH motors forward");
    forward(100);
    delay(1000);
    stop();
    
    LOGI("Motor test complete");
}

void Motors::move(int leftSpeed, int rightSpeed) {
    if (emergencyStopActive) {
        LOGW("Cannot move - emergency stop active");
        return;
    }
    
    applySpeedLimits(leftSpeed);
    applySpeedLimits(rightSpeed);
    
    // Smooth acceleration
    smoothAcceleration(leftSpeed, rightSpeed);
    
    setLeftMotor(leftSpeed);
    setRightMotor(rightSpeed);
    
    lastCommandTime = millis();
}

void Motors::stop() {
    setLeftMotor(0);
    setRightMotor(0);
    currentLeftSpeed = 0;
    currentRightSpeed = 0;
    leftDirection = MOTOR_STOP;
    rightDirection = MOTOR_STOP;
    LOGD("Motors stopped");
}

void Motors::brake() {
    // Active braking by reversing polarity briefly
    int leftBrake = currentLeftSpeed > 0 ? -50 : 50;
    int rightBrake = currentRightSpeed > 0 ? -50 : 50;
    
    setLeftMotor(leftBrake);
    setRightMotor(rightBrake);
    delay(50);
    stop();
    
    LOGI("Emergency brake applied");
}

void Motors::forward(int speed) {
    move(speed, speed);
    LOGD("Moving forward at speed %d", speed);
}

void Motors::backward(int speed) {
    move(-speed, -speed);
    LOGD("Moving backward at speed %d", speed);
}

void Motors::turnLeft(int speed) {
    move(speed / 2, speed);
    LOGD("Turning left");
}

void Motors::turnRight(int speed) {
    move(speed, speed / 2);
    LOGD("Turning right");
}

void Motors::pivot(int direction, int speed) {
    if (direction > 0) {
        // Pivot right (left backward, right forward)
        move(-speed, speed);
    } else {
        // Pivot left (left forward, right backward)
        move(speed, -speed);
    }
    LOGD("Pivoting %s", direction > 0 ? "right" : "left");
}

void Motors::moveDistance(int distanceMm, int speed) {
    // Calculate time needed based on calibration
    unsigned long timeMs = (distanceMm * 1000) / (MM_PER_PWM_SECOND * speed);
    
    LOGI("Moving %d mm (estimated time: %lu ms)", distanceMm, timeMs);
    
    MotorCommand cmd = {speed, speed, timeMs};
    setCommand(cmd);
}

void Motors::turnAngle(int degrees, bool clockwise) {
    // Calculate time needed based on turn rate calibration
    unsigned long timeMs = (abs(degrees) * 1000) / TURN_RATE_DEG_PER_SEC;
    
    LOGI("Turning %d degrees %s (estimated time: %lu ms)", 
         degrees, clockwise ? "CW" : "CCW", timeMs);
    
    int direction = clockwise ? 1 : -1;
    pivot(direction, MOTOR_TURN_SPEED);
    
    commandStartTime = millis();
    commandDuration = timeMs;
}

void Motors::setCommand(MotorCommand cmd) {
    commandStartTime = millis();
    commandDuration = cmd.duration;
    move(cmd.leftSpeed, cmd.rightSpeed);
}

void Motors::updateTimedCommand() {
    if (commandDuration > 0) {
        unsigned long elapsed = millis() - commandStartTime;
        if (elapsed >= commandDuration) {
            stop();
            commandDuration = 0;
            LOGD("Timed command complete");
        }
    }
}

void Motors::emergencyStop() {
    emergencyStopActive = true;
    brake();
    LOGE("EMERGENCY STOP ACTIVATED!");
}

void Motors::releaseEmergencyStop() {
    emergencyStopActive = false;
    LOGI("Emergency stop released");
}

void Motors::heartbeat() {
    lastCommandTime = millis();
}

bool Motors::checkTimeout() {
    if (isMoving() && (millis() - lastCommandTime > TASK_TIMEOUT)) {
        LOGE("Motor command timeout! Stopping motors.");
        emergencyStop();
        return true;
    }
    return false;
}

bool Motors::isMoving() const {
    return currentLeftSpeed != 0 || currentRightSpeed != 0;
}

void Motors::resetMetrics() {
    totalDistance = 0;
    leftRotations = 0;
    rightRotations = 0;
    LOGI("Metrics reset");
}

void Motors::printStatus() {
    LOGI("Motor Status:");
    LOGI("  Left: %d, Right: %d", currentLeftSpeed, currentRightSpeed);
    LOGI("  Direction: L=%d R=%d", leftDirection, rightDirection);
    LOGI("  Emergency Stop: %s", emergencyStopActive ? "ACTIVE" : "Inactive");
    LOGI("  Total Distance: %lu mm", totalDistance);
}

void Motors::testCalibration() {
    LOGI("=== Motor Calibration Test ===");
    LOGI("Testing forward movement for 5 seconds at base speed");
    forward(MOTOR_BASE_SPEED);
    delay(5000);
    stop();
    LOGI("Measure actual distance traveled and update MM_PER_PWM_SECOND");
    delay(2000);
    
    LOGI("Testing 360 degree turn");
    turnAngle(360, true);
    while (commandDuration > 0) {
        updateTimedCommand();
        delay(10);
    }
    LOGI("Verify robot completed 360 degrees and update TURN_RATE_DEG_PER_SEC");
}

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

void Motors::setLeftMotor(int speed) {
    applySpeedLimits(speed);
    currentLeftSpeed = speed;
    
    if (speed > 0) {
        leftDirection = MOTOR_FORWARD;
        digitalWrite(MOTOR_LEFT_IN1, HIGH);
        digitalWrite(MOTOR_LEFT_IN2, LOW);
        analogWrite(MOTOR_LEFT_PWM, abs(speed)); // ESP8266 analogWrite
    } else if (speed < 0) {
        leftDirection = MOTOR_BACKWARD;
        digitalWrite(MOTOR_LEFT_IN1, LOW);
        digitalWrite(MOTOR_LEFT_IN2, HIGH);
        analogWrite(MOTOR_LEFT_PWM, abs(speed)); // ESP8266 analogWrite
    } else {
        leftDirection = MOTOR_STOP;
        digitalWrite(MOTOR_LEFT_IN1, LOW);
        digitalWrite(MOTOR_LEFT_IN2, LOW);
        analogWrite(MOTOR_LEFT_PWM, 0); // ESP8266 analogWrite
    }
}

void Motors::setRightMotor(int speed) {
    applySpeedLimits(speed);
    currentRightSpeed = speed;
    
    if (speed > 0) {
        rightDirection = MOTOR_FORWARD;
        digitalWrite(MOTOR_RIGHT_IN1, HIGH);
        digitalWrite(MOTOR_RIGHT_IN2, LOW);
        analogWrite(MOTOR_RIGHT_PWM, abs(speed)); // ESP8266 analogWrite
    } else if (speed < 0) {
        rightDirection = MOTOR_BACKWARD;
        digitalWrite(MOTOR_RIGHT_IN1, LOW);
        digitalWrite(MOTOR_RIGHT_IN2, HIGH);
        analogWrite(MOTOR_RIGHT_PWM, abs(speed)); // ESP8266 analogWrite
    } else {
        rightDirection = MOTOR_STOP;
        digitalWrite(MOTOR_RIGHT_IN1, LOW);
        digitalWrite(MOTOR_RIGHT_IN2, LOW);
        analogWrite(MOTOR_RIGHT_PWM, 0); // ESP8266 analogWrite
    }
}

void Motors::applySpeedLimits(int& speed) {
    if (abs(speed) < MOTOR_MIN_SPEED && speed != 0) {
        speed = speed > 0 ? MOTOR_MIN_SPEED : -MOTOR_MIN_SPEED;
    }
    if (abs(speed) > MOTOR_MAX_SPEED) {
        speed = speed > 0 ? MOTOR_MAX_SPEED : -MOTOR_MAX_SPEED;
    }
}

void Motors::smoothAcceleration(int targetLeft, int targetRight) {
    // Gradual acceleration to prevent wheel slip
    const int ACCEL_STEP = 10;
    const int DELAY_MS = 10;
    
    if (abs(targetLeft - currentLeftSpeed) > ACCEL_STEP * 3 ||
        abs(targetRight - currentRightSpeed) > ACCEL_STEP * 3) {
        
        while (currentLeftSpeed != targetLeft || currentRightSpeed != targetRight) {
            if (currentLeftSpeed < targetLeft) {
                currentLeftSpeed = min(currentLeftSpeed + ACCEL_STEP, targetLeft);
            } else if (currentLeftSpeed > targetLeft) {
                currentLeftSpeed = max(currentLeftSpeed - ACCEL_STEP, targetLeft);
            }
            
            if (currentRightSpeed < targetRight) {
                currentRightSpeed = min(currentRightSpeed + ACCEL_STEP, targetRight);
            } else if (currentRightSpeed > targetRight) {
                currentRightSpeed = max(currentRightSpeed - ACCEL_STEP, targetRight);
            }
            
            setLeftMotor(currentLeftSpeed);
            setRightMotor(currentRightSpeed);
            delay(DELAY_MS);
        }
    }
}
