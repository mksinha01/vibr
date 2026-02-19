#ifndef MPU6050_SENSOR_H
#define MPU6050_SENSOR_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "config.h"
#include "logger.h"

// ============================================================================
// MPU6050 IMU HAL - Inertial Measurement Unit
// Provides orientation, rotation, and acceleration data
// ============================================================================

struct IMUData {
    // Acceleration (m/s²)
    float accelX, accelY, accelZ;
    
    // Gyroscope (rad/s)
    float gyroX, gyroY, gyroZ;
    
    // Calculated orientation (degrees)
    float pitch, roll, yaw;
    
    // Temperature (°C)
    float temperature;
    
    // Quality indicators
    bool dataValid;
    unsigned long timestamp;
};

class MPU6050Sensor {
private:
    Adafruit_MPU6050 mpu;
    
    // Calibration offsets
    float accelOffsetX, accelOffsetY, accelOffsetZ;
    float gyroOffsetX, gyroOffsetY, gyroOffsetZ;
    
    // Current state
    IMUData currentData;
    float currentYaw;
    unsigned long lastUpdateTime;
    
    // Filtering
    const float ALPHA = 0.98;  // Complementary filter coefficient
    float filteredPitch, filteredRoll;
    
    // Health monitoring
    bool initialized;
    unsigned int errorCount;
    unsigned long lastSuccessfulRead;
    
    // Internal functions
    void applyCalibration(sensors_event_t& a, sensors_event_t& g);
    void calculateOrientation(const sensors_event_t& a, const sensors_event_t& g);
    void complementaryFilter(float accelPitch, float accelRoll, float dt);
    
public:
    MPU6050Sensor();
    
    // Initialization
    bool init();
    bool calibrate(int samples = 1000);
    void loadCalibration(float ax, float ay, float az, float gx, float gy, float gz);
    
    // Data acquisition
    bool update();
    IMUData getData() const { return currentData; }
    
    // Orientation queries
    float getYaw() const { return currentYaw; }
    float getPitch() const { return currentData.pitch; }
    float getRoll() const { return currentData.roll; }
    
    // Safety checks
    bool isTilted(float maxAngle = MAX_TILT_ANGLE) const;
    bool isHighAcceleration(float maxG = MAX_ACCELERATION) const;
    bool isHealthy() const;
    
    // Yaw control for navigation
    void resetYaw();
    float getRelativeYaw(float targetYaw) const;
    
    // Diagnostics
    void printData();
    void selfTest();
    unsigned int getErrorCount() const { return errorCount; }
    
    // Calibration helpers
    void startCalibration();
    bool isCalibrated() const;
};

#endif // MPU6050_SENSOR_H
