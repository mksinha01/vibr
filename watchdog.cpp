#include "watchdog.h"

static const char* TAG = "Watchdog";

Watchdog::Watchdog()
    : watchdogEnabled(false)
    , lastFeedTime(0)
    , lastMotorHeartbeat(0)
    , lastIMUHeartbeat(0)
    , lastGPSHeartbeat(0)
    , lastSonarHeartbeat(0)
    , lastWiFiHeartbeat(0)
    , consecutiveErrors(0)
    , bootTime(0)
    , resetCount(0)
    , panicCount(0) {
    
    memset(&health, 0, sizeof(HealthStatus));
    health.overall = HEALTH_GOOD;
}

bool Watchdog::init() {
    LOGI("Initializing watchdog system...");
    
    bootTime = millis();
    
    // Load reset count from preferences/EEPROM (simplified here)
    resetCount++;
    
#ifndef DISABLE_WATCHDOG
    // Configure ESP8266 hardware watchdog
    ESP.wdtEnable(WATCHDOG_TIMEOUT);
    
    watchdogEnabled = true;
    LOGI("Hardware watchdog enabled (timeout: %d ms)", WATCHDOG_TIMEOUT);
#else
    LOGW("Watchdog DISABLED for debugging");
#endif
    
    lastFeedTime = millis();
    LOGI("Watchdog system initialized");
    return true;
}

void Watchdog::enable() {
    watchdogEnabled = true;
    LOGI("Watchdog enabled");
}

void Watchdog::disable() {
    watchdogEnabled = false;
    LOGW("Watchdog disabled");
}

void Watchdog::feed() {
    if (watchdogEnabled) {
#ifndef DISABLE_WATCHDOG
        ESP.wdtFeed(); // ESP8266 watchdog feed
#endif
        lastFeedTime = millis();
    }
}

void Watchdog::update() {
    unsigned long currentTime = millis();
    
    // Feed watchdog if system is healthy
    if (currentTime - lastFeedTime >= 1000) {  // Feed every second
        checkComponentHealth();
        
        if (isSystemHealthy()) {
            feed();
        } else {
            LOGW("System health degraded - watchdog not fed");
            
            if (health.overall == HEALTH_CRITICAL) {
                LOGE("System health CRITICAL - attempting recovery");
                attemptRecovery();
            }
        }
    }
    
    // Update metrics
    health.uptime = getUptime();
}

void Watchdog::heartbeatMotor() {
    lastMotorHeartbeat = millis();
}

void Watchdog::heartbeatIMU() {
    lastIMUHeartbeat = millis();
}

void Watchdog::heartbeatGPS() {
    lastGPSHeartbeat = millis();
}

void Watchdog::heartbeatSonar() {
    lastSonarHeartbeat = millis();
}

void Watchdog::heartbeatWiFi() {
    lastWiFiHeartbeat = millis();
}

HealthStatus Watchdog::getHealth() {
    checkComponentHealth();
    return health;
}

bool Watchdog::isSystemHealthy() {
    checkComponentHealth();
    return health.overall == HEALTH_GOOD || health.overall == HEALTH_WARNING;
}

void Watchdog::updateBatteryVoltage(float voltage) {
    health.batteryVoltage = voltage;
    
    if (isBatteryCritical()) {
        LOGE("CRITICAL BATTERY: %.2fV", voltage);
    } else if (isBatteryLow()) {
        LOGW("Low battery: %.2fV", voltage);
    }
}

bool Watchdog::isBatteryLow() {
    return health.batteryVoltage < LOW_BATTERY_VOLTAGE && 
           health.batteryVoltage > 0;
}

bool Watchdog::isBatteryCritical() {
    return health.batteryVoltage < CRITICAL_BATTERY && 
           health.batteryVoltage > 0;
}

void Watchdog::reportError(const char* component, const char* error) {
    LOGE("Error reported by %s: %s", component, error);
    health.totalErrors++;
    consecutiveErrors++;
    
    if (consecutiveErrors > 10) {
        LOGE("Too many consecutive errors - system health degraded");
        health.overall = HEALTH_CRITICAL;
    }
}

void Watchdog::clearErrors() {
    consecutiveErrors = 0;
    LOGI("Error counter cleared");
}

unsigned long Watchdog::getUptime() {
    return (millis() - bootTime) / 1000;  // Seconds
}

void Watchdog::printStatus() {
    const char* healthStr[] = {"GOOD", "WARNING", "CRITICAL", "FAILED"};
    
    LOGI("=== SYSTEM HEALTH STATUS ===");
    LOGI("Overall Health: %s", healthStr[health.overall]);
    LOGI("Components:");
    LOGI("  Motors: %s", health.motorHealthy ? "OK" : "FAIL");
    LOGI("  IMU:    %s", health.imuHealthy ? "OK" : "FAIL");
    LOGI("  GPS:    %s", health.gpsHealthy ? "OK" : "FAIL");
    LOGI("  Sonar:  %s", health.sonarHealthy ? "OK" : "FAIL");
    LOGI("  WiFi:   %s", health.wifiHealthy ? "OK" : "FAIL");
    LOGI("Battery: %.2fV", health.batteryVoltage);
    LOGI("Uptime: %lu seconds", health.uptime);
    LOGI("Total Errors: %d", health.totalErrors);
    LOGI("==========================");
}

void Watchdog::printDetailedReport() {
    printStatus();
    
    LOGI("=== DETAILED COMPONENT STATUS ===");
    unsigned long now = millis();
    LOGI("Last Heartbeats (ms ago):");
    LOGI("  Motor: %lu", now - lastMotorHeartbeat);
    LOGI("  IMU:   %lu", now - lastIMUHeartbeat);
    LOGI("  GPS:   %lu", now - lastGPSHeartbeat);
    LOGI("  Sonar: %lu", now - lastSonarHeartbeat);
    LOGI("  WiFi:  %lu", now - lastWiFiHeartbeat);
    LOGI("System Info:");
    LOGI("  Boot count: %d", resetCount);
    LOGI("  Panic count: %d", panicCount);
    LOGI("  Free heap: %d bytes", ESP.getFreeHeap());
    LOGI("================================");
}

void Watchdog::attemptRecovery() {
    LOGI("Attempting system recovery...");
    
    clearErrors();
    
    // Try to recover WiFi
    if (!health.wifiHealthy) {
        LOGI("Attempting WiFi recovery...");
        // WiFi reconnection handled by WiFi module
    }
    
    // Reset sensor error counts
    // This would involve calling reset functions on each sensor
    
    LOGI("Recovery attempt complete");
}

void Watchdog::softReset() {
    LOGW("Performing soft reset...");
    delay(1000);
    ESP.restart();
}

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

void Watchdog::checkComponentHealth() {
    unsigned long now = millis();
    const unsigned long HEARTBEAT_TIMEOUT = 5000;  // 5 seconds
    
    // Check each component based on last heartbeat
    health.motorHealthy = (now - lastMotorHeartbeat) < HEARTBEAT_TIMEOUT;
    health.imuHealthy = (now - lastIMUHeartbeat) < HEARTBEAT_TIMEOUT;
    health.gpsHealthy = (now - lastGPSHeartbeat) < HEARTBEAT_TIMEOUT;
    health.sonarHealthy = (now - lastSonarHeartbeat) < HEARTBEAT_TIMEOUT;
    health.wifiHealthy = (now - lastWiFiHeartbeat) < HEARTBEAT_TIMEOUT;
    
    // Evaluate overall health
    int failedComponents = 0;
    int criticalComponentsFailed = 0;
    
    // CRITICAL components (robot cannot operate without these)
    if (!health.motorHealthy) {
        criticalComponentsFailed++;
        failedComponents++;
    }
    
    // WARNING components (robot can operate with degraded performance)
    if (!health.imuHealthy) failedComponents++;
    if (!health.sonarHealthy) failedComponents++;
    
    // OPTIONAL components (robot works fine without these)
    // WiFi and GPS failures don't affect component count
    
    // Set overall health
    if (isBatteryCritical() || criticalComponentsFailed > 0) {
        health.overall = HEALTH_CRITICAL;
    } else if (failedComponents >= 3 || isBatteryLow()) {
        health.overall = HEALTH_WARNING;
    } else if (failedComponents >= 1) {
        health.overall = HEALTH_WARNING;
    } else {
        health.overall = HEALTH_GOOD;
    }
}

// Component health reporting (overloaded for HealthComponent enum)
void Watchdog::reportHealthy(HealthComponent component) {
    unsigned long now = millis();
    switch(component) {
        case MOTOR:
            lastMotorHeartbeat = now;
            health.motorHealthy = true;
            break;
        case IMU:
            lastIMUHeartbeat = now;
            health.imuHealthy = true;
            break;
        case GPS:
            lastGPSHeartbeat = now;
            health.gpsHealthy = true;
            break;
        case SONAR:
            lastSonarHeartbeat = now;
            health.sonarHealthy = true;
            break;
        case WIFI:
            lastWiFiHeartbeat = now;
            health.wifiHealthy = true;
            break;
        default:
            break;
    }
}

void Watchdog::reportError(HealthComponent component) {
    switch(component) {
        case MOTOR:
            health.motorHealthy = false;
            LOGW("Motor component error reported");
            break;
        case IMU:
            health.imuHealthy = false;
            LOGW("IMU component error reported");
            break;
        case GPS:
            health.gpsHealthy = false;
            LOGW("GPS component error reported");
            break;
        case SONAR:
            health.sonarHealthy = false;
            LOGW("Sonar component error reported");
            break;
        case WIFI:
            health.wifiHealthy = false;
            LOGW("WiFi component error reported");
            break;
        default:
            break;
    }
    consecutiveErrors++;
    health.totalErrors++;
}
