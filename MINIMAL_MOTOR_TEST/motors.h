#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include "config.h"
#include "logger.h"

// ============================================================================
// MOTOR CONTROL HAL - Hardware Abstraction Layer
// Industry standard motor control with safety features
// ============================================================================

enum MotorDirection {
    MOTOR_STOP,
    MOTOR_FORWARD,
    MOTOR_BACKWARD
};

struct MotorCommand {
    int leftSpeed;      // -255 to 255
    int rightSpeed;     // -255 to 255
    unsigned long duration;  // ms (0 = continuous)
};

class Motors {
private:
    // Current motor state
    int currentLeftSpeed;
    int currentRightSpeed;
    MotorDirection leftDirection;
    MotorDirection rightDirection;
    
    // Safety
    bool emergencyStopActive;
    unsigned long lastCommandTime;
    unsigned long commandStartTime;
    unsigned long commandDuration;
    
    // Metrics
    unsigned long totalDistance;  // mm
    unsigned long leftRotations;
    unsigned long rightRotations;
    
    // Internal control functions
    void setLeftMotor(int speed);
    void setRightMotor(int speed);
    void applySpeedLimits(int& speed);
    
    // Acceleration control
    void smoothAcceleration(int targetLeft, int targetRight);
    
public:
    Motors();
    
    // Initialization
    bool init();
    void test();
    
    // Basic control
    void move(int leftSpeed, int rightSpeed);
    void stop();
    void brake();  // Immediate stop with braking
    
    // High-level commands
    void forward(int speed = MOTOR_BASE_SPEED);
    void backward(int speed = MOTOR_BASE_SPEED);
    void turnLeft(int speed = MOTOR_TURN_SPEED);
    void turnRight(int speed = MOTOR_TURN_SPEED);
    void pivot(int direction, int speed = MOTOR_TURN_SPEED);  // Turn in place
    
    // Distance-based movement (calibrated)
    void moveDistance(int distanceMm, int speed = MOTOR_BASE_SPEED);
    void turnAngle(int degrees, bool clockwise = true);
    
    // Timed commands
    void setCommand(MotorCommand cmd);
    void updateTimedCommand();  // Call in loop
    
    // Safety
    void emergencyStop();
    void releaseEmergencyStop();
    bool isEmergencyStopped() const { return emergencyStopActive; }
    
    // Watchdog integration
    void heartbeat();  // Call regularly to prevent timeout
    bool checkTimeout();  // Returns true if timeout occurred
    
    // Getters
    int getLeftSpeed() const { return currentLeftSpeed; }
    int getRightSpeed() const { return currentRightSpeed; }
    bool isMoving() const;
    
    // Metrics
    unsigned long getTotalDistance() const { return totalDistance; }
    void resetMetrics();
    void printStatus();
    
    // Calibration helpers
    void testCalibration();
};

#endif // MOTORS_H
