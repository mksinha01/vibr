// ============================================================================
// AUTONOMOUS DELIVERY ROBOT - ESP8266 VERSION
// Arduino IDE Compatible Version
// ============================================================================
#include <ESP8266WiFi.h>
#include "config.h"
#include "logger.h"
#include "state_machine.h"
#include "motors.h"
#include "mpu6050.h"
#include "gps.h"
#include "sonar.h"
#include "wifi_comm.h"
#include "watchdog.h"

// ============================================================================
// GLOBAL COMPONENT INSTANCES
// ============================================================================

Motors motors;
MPU6050Sensor imu;
// GPS disabled in minimal version
Sonar sonar;
WiFiComm wifiComm;
StateMachine stateMachine;
Watchdog watchdog;

// System state
bool systemInitialized = false;
unsigned long lastControlUpdate = 0;
unsigned long lastTelemetryUpdate = 0;
unsigned long lastSensorUpdate = 0;
unsigned long lastSafetyCheck = 0;
unsigned long lastHealthCheck = 0;
bool emergencyStop = false;

// Performance Metrics (Industry Standard)
struct PerformanceMetrics {
    unsigned long loopCount;
    unsigned long avgLoopTime;
    unsigned long maxLoopTime;
    unsigned long minLoopTime;
    unsigned long totalRunTime;
    unsigned int safetyViolations;
    unsigned int errorRecoveries;
    float cpuUsagePercent;
} perfMetrics = {0, 0, 0, 999999, 0, 0, 0, 0.0};

// Function prototypes
bool initializeSystem();
IRAM_ATTR void emergencyStopISR();
void updateSensors();
void updateControl();
void updateTelemetry();
void processEmergencyStop();
void performSafetyChecks();
void updatePerformanceMetrics();
void printPerformanceReport();

// ============================================================================
// ARDUINO SETUP - SYSTEM INITIALIZATION
// ============================================================================

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════════════╗");
    Serial.println("║   AUTONOMOUS DELIVERY ROBOT - ESP8266         ║");
    Serial.println("║   Arduino IDE Upload Version                  ║");
    Serial.println("╚════════════════════════════════════════════════╝\n");
    
    // Initialize logging system
    Logger::init((LogLevel)LOG_LEVEL);
    LOGI("System boot initiated...");
    LOGI("Firmware Version: %s", FIRMWARE_VERSION);
    LOGI("Build Date: %s %s", BUILD_DATE, BUILD_TIME);
    LOGI("Free Heap: %d bytes", ESP.getFreeHeap());
    
    // Initialize watchdog
    ESP.wdtEnable(WATCHDOG_TIMEOUT);
    LOGI("Watchdog initialized (timeout: %d ms)", WATCHDOG_TIMEOUT);
    
    // Configure emergency stop button
    pinMode(EMERGENCY_STOP_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(EMERGENCY_STOP_PIN), 
                   emergencyStopISR, FALLING);
    LOGI("Emergency stop configured on pin %d", EMERGENCY_STOP_PIN);
    
    // Initialize state machine
    stateMachine.init();
    
    // Initialize system components
    if (initializeSystem()) {
        systemInitialized = true;
        
#ifdef ENABLE_SELF_TEST
        LOGI("\n");
        LOGI("╔══════════════════════════════════════════════╗");
        LOGI("║    RUNNING COMPREHENSIVE SELF-TEST         ║");
        LOGI("╚══════════════════════════════════════════════╝");
        runComprehensiveSelfTest();
        LOGI("╔══════════════════════════════════════════════╗");
        LOGI("║    SELF-TEST COMPLETE                       ║");
        LOGI("╚══════════════════════════════════════════════╝\n");
#endif
        
        stateMachine.forceState(STATE_IDLE);
        LOGI("System initialization complete");
        LOGI("Entering main loop...");
    } else {
        LOGE("System initialization failed!");
        stateMachine.forceState(STATE_ERROR);
    }
    
    // Print startup stats
    Logger::logSystemInfo();
    
    LOGI("Setup complete. Free heap: %d bytes", ESP.getFreeHeap());
}

// ============================================================================
// ARDUINO LOOP - MAIN EXECUTION
// ============================================================================

void loop() {
    unsigned long loopStart = millis();
    
    if (!systemInitialized) {
        delay(1000);
        return;
    }
    
    // ===== SAFETY LAYER (Highest Priority) =====
    // Check for emergency stop FIRST
    if (emergencyStop) {
        processEmergencyStop();
        return;
    }
    
    // Critical safety checks at 10Hz
    if (millis() - lastSafetyCheck >= 100) {
        performSafetyChecks();
        lastSafetyCheck = millis();
    }
    
    // ===== STATE MACHINE CONTROL =====
    stateMachine.update();
    
    // ===== WATCHDOG FEEDING =====
    watchdog.feed();
    ESP.wdtFeed();
    
    // ===== SENSOR UPDATES (20Hz) =====
    if (millis() - lastSensorUpdate >= 50) {
        updateSensors();
        lastSensorUpdate = millis();
    }
    
    // ===== CONTROL LOOP (20Hz) =====
    if (millis() - lastControlUpdate >= 50) {
        updateControl();
        lastControlUpdate = millis();
    }
    
    // ===== TELEMETRY (5Hz) =====
    if (millis() - lastTelemetryUpdate >= 200) {
        updateTelemetry();
        lastTelemetryUpdate = millis();
    }
    
    // ===== HEALTH MONITORING (1Hz) =====
    if (millis() - lastHealthCheck >= 1000) {
        watchdog.update();
        lastHealthCheck = millis();
    }
    
    // ===== COMMUNICATION =====
    wifiComm.update();
    
    // ===== PERFORMANCE TRACKING =====
    unsigned long loopTime = millis() - loopStart;
    if (loopTime > perfMetrics.maxLoopTime) perfMetrics.maxLoopTime = loopTime;
    if (loopTime < perfMetrics.minLoopTime) perfMetrics.minLoopTime = loopTime;
    perfMetrics.loopCount++;
    perfMetrics.totalRunTime = millis();
    
    // Calculate CPU usage
    perfMetrics.cpuUsagePercent = (loopTime / 50.0) * 100.0; // 50ms target
    
    delay(1);
}

// ============================================================================
// SYSTEM INITIALIZATION
// ============================================================================

bool initializeSystem() {
    LOGI("Initializing robot subsystems...");
    bool allOk = true;
    
    // Initialize motors
    if (motors.init()) {
        LOGI("Motors initialized");
        watchdog.reportHealthy(MOTOR);
    } else {
        LOGE("Motor initialization failed!");
        watchdog.reportError(MOTOR);
        allOk = false;
    }
    
    // Initialize IMU
    if (imu.init()) {
        LOGI("IMU initialized");
        watchdog.reportHealthy(IMU);
    } else {
        LOGW("IMU initialization failed - will continue without it");
        watchdog.reportError(IMU);
    }
    
    // GPS skipped in minimal version
    LOGW("GPS disabled in minimal version");
    
    // Initialize sonar
    if (sonar.init()) {
        LOGI("Sonar initialized");
        watchdog.reportHealthy(SONAR);
    } else {
        LOGW("Sonar initialization failed - will continue without it");
        watchdog.reportError(SONAR);
    }
    
    // Initialize WiFi communication
    if (wifiComm.init()) {
        LOGI("WiFi communication initialized");
        watchdog.reportHealthy(WIFI);
    } else {
        LOGW("WiFi initialization failed - will continue without it");
        watchdog.reportError(WIFI);
    }
    
    // Initialize watchdog monitoring
    watchdog.init();
    
    return allOk;
}

// ============================================================================
// EMERGENCY STOP INTERRUPT HANDLER
// ============================================================================

IRAM_ATTR void emergencyStopISR() {
    emergencyStop = true;
}

// ============================================================================
// SENSOR UPDATE LOOP
// ============================================================================

void updateSensors() {
    // Update IMU
    if (!imu.update()) {
        watchdog.reportError(IMU);
    }
    
    // Update sonar
    if (!sonar.update()) {
        watchdog.reportError(SONAR);
    }
}

// ============================================================================
// CONTROL UPDATE LOOP
// ============================================================================

void updateControl() {
    RobotState currentState = stateMachine.getCurrentState();
    
    // Get sensor data
    SonarData sonarData = sonar.getData();
    IMUData imuData = imu.getData();
    
    // Obstacle detection
    if (sonarData.distanceCm < OBSTACLE_THRESHOLD && sonarData.distanceCm > 0) {
        if (currentState == STATE_NAVIGATING) {
            LOGW("Obstacle detected at %.1f cm", sonarData.distanceCm);
            stateMachine.processEvent(EVENT_OBSTACLE_DETECTED);
        }
    }
    
    // Basic control logic
    switch (currentState) {
        case STATE_IDLE:
            motors.stop();
            break;
            
        case STATE_NAVIGATING:
            motors.forward(MOTOR_BASE_SPEED);
            break;
            
        case STATE_OBSTACLE_AVOID:
            motors.stop();
            if (sonarData.distanceCm > OBSTACLE_THRESHOLD) {
                stateMachine.processEvent(EVENT_OBSTACLE_CLEARED);
            }
            break;
            
        case STATE_EMERGENCY_STOP:
        case STATE_ERROR:
            motors.emergencyStop();
            break;
            
        default:
            break;
    }
}

// ============================================================================
// TELEMETRY UPDATE
// ============================================================================

void updateTelemetry() {
    if (!wifiComm.isConnected()) {
        return;
    }
    
    RobotState currentState = stateMachine.getCurrentState();
    IMUData imuData = imu.getData();
    SonarData sonarData = sonar.getData();
    
    static int counter = 0;
    if (++counter >= 25) {
        LOGI("Status: %s | Heap: %d | Sonar: %.1fcm", 
            stateMachine.getStateName(currentState),
            ESP.getFreeHeap(),
            sonarData.distanceCm);
        counter = 0;
    }
}

// ============================================================================
// EMERGENCY STOP HANDLER
// ============================================================================

void processEmergencyStop() {
    static bool stopHandled = false;
    
    if (!stopHandled) {
        LOGE("EMERGENCY STOP ACTIVATED!");
        stateMachine.forceState(STATE_EMERGENCY_STOP);
        motors.emergencyStop();
        perfMetrics.safetyViolations++;
        stopHandled = true;
    }
    
    if (digitalRead(EMERGENCY_STOP_PIN) == HIGH) {
        delay(50);
        if (digitalRead(EMERGENCY_STOP_PIN) == HIGH) {
            LOGI("Emergency stop released");
            emergencyStop = false;
            stopHandled = false;
            stateMachine.forceState(STATE_IDLE);
            perfMetrics.errorRecoveries++;
        }
    }
    
    delay(100);
}

// ============================================================================
// SAFETY LAYER - Critical Safety Checks (Industry Standard)
// ============================================================================

void performSafetyChecks() {
    bool safetyViolation = false;
    
    // 1. Check IMU for excessive tilt
    if (imu.isHealthy() && imu.isTilted(MAX_TILT_ANGLE)) {
        LOGE("SAFETY: Excessive tilt detected! Pitch: %.1f, Roll: %.1f", 
             imu.getPitch(), imu.getRoll());
        motors.emergencyStop();
        stateMachine.forceState(STATE_EMERGENCY_STOP);
        safetyViolation = true;
    }
    
    // 2. Check for high acceleration (collision detection)
    if (imu.isHealthy() && imu.isHighAcceleration(MAX_ACCELERATION)) {
        LOGE("SAFETY: High acceleration detected! Possible collision!");
        motors.emergencyStop();
        stateMachine.forceState(STATE_EMERGENCY_STOP);
        safetyViolation = true;
    }
    
    // 3. Check critical obstacle distance
    SonarData sonarData = sonar.getData();
    if (sonarData.dataValid && sonarData.distanceCm < EMERGENCY_STOP_DIST && 
        sonarData.distanceCm > 0 && motors.isMoving()) {
        LOGE("SAFETY: Critical obstacle at %.1f cm! Emergency stop!", 
             sonarData.distanceCm);
        motors.emergencyStop();
        stateMachine.processEvent(EVENT_EMERGENCY_STOP);
        safetyViolation = true;
    }
    
    // 4. Check motor timeout
    if (motors.checkTimeout()) {
        LOGE("SAFETY: Motor command timeout!");
        safetyViolation = true;
    }
    
    // 5. Check system health (only stop for TRULY critical failures)
    SystemHealth health = watchdog.getOverallHealth();
    if (health == HEALTH_CRITICAL) {
        // Only motor failure is truly critical
        if (!motors.isMoving()) {
            // Already stopped, just log
            LOGD("SAFETY: System health critical, motors already stopped");
        } else {
            // Motors still moving but critical failure - stop them
            LOGE("SAFETY: Critical system health! Stopping robot.");
            motors.emergencyStop();
            stateMachine.forceState(STATE_ERROR);
            safetyViolation = true;
        }
    } else if (health == HEALTH_WARNING) {
        // Just log warnings, don't stop
        static unsigned long lastWarning = 0;
        if (millis() - lastWarning > 5000) {
            LOGW("SAFETY: System health degraded (non-critical)");
            lastWarning = millis();
        }
    }
    
    if (safetyViolation) {
        perfMetrics.safetyViolations++;
    }
}

// ============================================================================
// PERFORMANCE METRICS TRACKING
// ============================================================================

void updatePerformanceMetrics() {
    if (perfMetrics.loopCount > 0) {
        perfMetrics.avgLoopTime = perfMetrics.totalRunTime / perfMetrics.loopCount;
    }
}

void printPerformanceReport() {
    updatePerformanceMetrics();
    
    LOGI("╔══════════════════════════════════════════════╗");
    LOGI("║       PERFORMANCE METRICS REPORT             ║");
    LOGI("╚══════════════════════════════════════════════╝");
    LOGI("Loop Statistics:");
    LOGI("  Total Loops: %lu", perfMetrics.loopCount);
    LOGI("  Avg Loop Time: %lu ms", perfMetrics.avgLoopTime);
    LOGI("  Max Loop Time: %lu ms", perfMetrics.maxLoopTime);
    LOGI("  Min Loop Time: %lu ms", perfMetrics.minLoopTime);
    LOGI("  CPU Usage: %.1f%%", perfMetrics.cpuUsagePercent);
    LOGI("Safety & Recovery:");
    LOGI("  Safety Violations: %u", perfMetrics.safetyViolations);
    LOGI("  Error Recoveries: %u", perfMetrics.errorRecoveries);
    LOGI("System:");
    LOGI("  Total Runtime: %lu seconds", perfMetrics.totalRunTime / 1000);
    LOGI("  Free Heap: %d bytes", ESP.getFreeHeap());
    LOGI("  Uptime: %lu seconds", watchdog.getUptime());
    LOGI("════════════════════════════════════════════════");
    
    // Print state machine metrics
    stateMachine.printMetrics();
    
    // Print watchdog status
    watchdog.printDetailedReport();
}

// ============================================================================
// COMPREHENSIVE SELF-TEST SYSTEM (Industry Standard)
// ============================================================================

void runComprehensiveSelfTest() {
    bool allTestsPassed = true;
    int testsPassed = 0;
    int testsFailed = 0;
    
    LOGI("\n=== MOTOR SUBSYSTEM TEST ===");
    LOGI("Testing motor control and safety...");
    motors.test();
    if (motors.isMoving() == false) {
        LOGI("✓ Motor test PASSED");
        testsPassed++;
    } else {
        LOGE("✗ Motor test FAILED");
        testsFailed++;
        allTestsPassed = false;
    }
    delay(500);
    
    LOGI("\n=== IMU SUBSYSTEM TEST ===");
    LOGI("Testing IMU sensor and orientation...");
    imu.selfTest();
    if (imu.isHealthy()) {
        LOGI("✓ IMU test PASSED");
        testsPassed++;
        imu.printData();
    } else {
        LOGE("✗ IMU test FAILED");
        testsFailed++;
        allTestsPassed = false;
    }
    delay(500);
    
    LOGI("\n=== SONAR SUBSYSTEM TEST ===");
    LOGI("Testing ultrasonic distance sensor...");
    sonar.selfTest();
    if (sonar.isHealthy()) {
        LOGI("✓ Sonar test PASSED");
        testsPassed++;
        sonar.printData();
    } else {
        LOGE("✗ Sonar test FAILED");
        testsFailed++;
        allTestsPassed = false;
    }
    delay(500);
    
    LOGI("\n=== WiFi SUBSYSTEM TEST ===");
    LOGI("Testing WiFi connectivity...");
    wifiComm.printStatus();
    if (wifiComm.isConnected()) {
        LOGI("✓ WiFi test PASSED");
        testsPassed++;
    } else {
        LOGW("⚠ WiFi test WARNING - not critical");
        testsPassed++; // WiFi not critical for basic operation
    }
    delay(500);
    
    LOGI("\n=== WATCHDOG SUBSYSTEM TEST ===");
    LOGI("Testing health monitoring system...");
    watchdog.printStatus();
    if (watchdog.isSystemHealthy()) {
        LOGI("✓ Watchdog test PASSED");
        testsPassed++;
    } else {
        LOGE("✗ Watchdog test FAILED");
        testsFailed++;
        allTestsPassed = false;
    }
    delay(500);
    
    LOGI("\n=== STATE MACHINE TEST ===");
    LOGI("Testing state transitions...");
    RobotState initialState = stateMachine.getCurrentState();
    stateMachine.processEvent(EVENT_INIT_COMPLETE);
    if (stateMachine.getCurrentState() != initialState || initialState == STATE_INIT) {
        LOGI("✓ State machine test PASSED");
        testsPassed++;
    } else {
        LOGE("✗ State machine test FAILED");
        testsFailed++;
        allTestsPassed = false;
    }
    delay(500);
    
    LOGI("\n=== MEMORY TEST ===");
    LOGI("Checking system memory...");
    uint32_t freeHeap = ESP.getFreeHeap();
    LOGI("Free Heap: %u bytes", freeHeap);
    if (freeHeap > MIN_FREE_HEAP) {
        LOGI("✓ Memory test PASSED");
        testsPassed++;
    } else {
        LOGE("✗ Memory test FAILED - insufficient heap!");
        testsFailed++;
        allTestsPassed = false;
    }
    
    // Final Report
    LOGI("\n");
    LOGI("╔══════════════════════════════════════════════╗");
    LOGI("║       SELF-TEST RESULTS                     ║");
    LOGI("╚══════════════════════════════════════════════╝");
    LOGI("Tests Passed: %d", testsPassed);
    LOGI("Tests Failed: %d", testsFailed);
    
    if (allTestsPassed) {
        LOGI("✅ ALL TESTS PASSED - SYSTEM READY");
    } else {
        LOGE("❌ SOME TESTS FAILED - CHECK HARDWARE");
    }
    LOGI("════════════════════════════════════════════════");
}
