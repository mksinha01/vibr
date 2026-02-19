#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <Arduino.h>
#include "config.h"
#include "logger.h"

// ============================================================================
// WATCHDOG & HEALTH MONITORING SYSTEM
// Industry-standard safety and monitoring
// ============================================================================

enum SystemHealth {
    HEALTH_GOOD,
    HEALTH_WARNING,
    HEALTH_CRITICAL,
    HEALTH_FAILED
};

enum HealthComponent {
    MOTOR,
    IMU,
    GPS,
    SONAR,
    WIFI,
    BATTERY
};

struct HealthStatus {
    SystemHealth overall;
    bool motorHealthy;
    bool imuHealthy;
    bool gpsHealthy;
    bool sonarHealthy;
    bool wifiHealthy;
    float batteryVoltage;
    unsigned long uptime;
    unsigned int totalErrors;
};

class Watchdog {
private:
    // Watchdog timer
    bool watchdogEnabled;
    unsigned long lastFeedTime;
    
    // Component heartbeats
    unsigned long lastMotorHeartbeat;
    unsigned long lastIMUHeartbeat;
    unsigned long lastGPSHeartbeat;
    unsigned long lastSonarHeartbeat;
    unsigned long lastWiFiHeartbeat;
    
    // Health tracking
    HealthStatus health;
    unsigned int consecutiveErrors;
    
    // System metrics
    unsigned long bootTime;
    unsigned int resetCount;
    unsigned int panicCount;
    
    // Internal functions
    void checkComponentHealth();
    void attemptRecovery();
    void softReset();
    
public:
    Watchdog();
    
    // Initialization
    bool init();
    void enable();
    void disable();
    
    // Watchdog operations
    void feed();
    void update();
    
    // Component heartbeats
    void heartbeatMotor();
    void heartbeatIMU();
    void heartbeatGPS();
    void heartbeatSonar();
    void heartbeatWiFi();
    
    // Component health reporting
    void reportHealthy(HealthComponent component);
    void reportError(HealthComponent component);
    
    // Health monitoring
    HealthStatus getHealth();
    SystemHealth getOverallHealth() { return health.overall; }
    bool isSystemHealthy();
    
    // Battery monitoring
    void updateBatteryVoltage(float voltage);
    bool isBatteryLow();
    bool isBatteryCritical();
    
    // Error tracking
    void reportError(const char* component, const char* error);
    void clearErrors();
    
    // System info
    unsigned long getUptime();
    unsigned int getResetCount() { return resetCount; }
    void printStatus();
    void printDetailedReport();
    
    // Recovery
    void triggerRecovery();
};

#endif // WATCHDOG_H
