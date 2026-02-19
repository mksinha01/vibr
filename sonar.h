#ifndef SONAR_H
#define SONAR_H

#include <Arduino.h>
#include "config.h"
#include "logger.h"

// ============================================================================
// SONAR SENSOR HAL - Ultrasonic Distance Sensor
// Industry-standard obstacle detection with filtering
// ============================================================================

struct SonarData {
    float distanceCm;
    bool obstacleDetected;
    bool criticalDistance;
    bool dataValid;
    unsigned long timestamp;
};

class Sonar {
private:
    // Hardware pins
    uint8_t triggerPin;
    uint8_t echoPin;
    
    // Current state
    SonarData currentData;
    float lastValidDistance;
    
    // Filtering - moving average
    static const int FILTER_SIZE = 5;
    float distanceBuffer[FILTER_SIZE];
    int bufferIndex;
    bool bufferFilled;
    
    // Health monitoring
    unsigned int errorCount;
    unsigned int timeoutCount;
    unsigned long lastSuccessfulRead;
    
    // Internal functions
    float readRaw();
    float applyFilter(float newDistance);
    bool validateDistance(float distance);
    
public:
    Sonar(uint8_t trig = SONAR_TRIGGER_PIN, uint8_t echo = SONAR_ECHO_PIN);
    
    // Initialization
    bool init();
    void test();
    
    // Data acquisition
    bool update();
    SonarData getData() const { return currentData; }
    float getDistance() const { return currentData.distanceCm; }
    
    // Obstacle detection
    bool isObstacleDetected() const { return currentData.obstacleDetected; }
    bool isCriticalDistance() const { return currentData.criticalDistance; }
    bool isClearPath() const;
    
    // Configuration
    void setThresholds(float obstacleThreshold, float criticalThreshold);
    
    // Health monitoring
    bool isHealthy() const;
    unsigned int getErrorCount() const { return errorCount; }
    
    // Diagnostics
    void printData();
    void selfTest();
    void clearBuffer();
};

#endif // SONAR_H
