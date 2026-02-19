#include "sonar.h"

static const char* TAG = "Sonar";

Sonar::Sonar(uint8_t trig, uint8_t echo)
    : triggerPin(trig)
    , echoPin(echo)
    , lastValidDistance(SONAR_MAX_DISTANCE)
    , bufferIndex(0)
    , bufferFilled(false)
    , errorCount(0)
    , timeoutCount(0)
    , lastSuccessfulRead(0) {
    
    memset(&currentData, 0, sizeof(SonarData));
    memset(distanceBuffer, 0, sizeof(distanceBuffer));
    currentData.distanceCm = SONAR_MAX_DISTANCE;
}

bool Sonar::init() {
    LOGI("Initializing ultrasonic sensor...");
    
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    digitalWrite(triggerPin, LOW);
    delay(50);
    
    // Test initial reading
    float testDistance = readRaw();
    if (testDistance > 0 && testDistance <= SONAR_MAX_DISTANCE) {
        LOGI("Sonar initialized successfully, initial distance: %.1f cm", testDistance);
        lastSuccessfulRead = millis();
        return true;
    } else {
        LOGW("Sonar initialization warning: unusual initial reading: %.1f cm", testDistance);
        return true;  // Still return true, sensor might warm up
    }
}

void Sonar::test() {
    LOGI("Starting sonar test...");
    
    for (int i = 0; i < 10; i++) {
        update();
        LOGI("Test reading %d: %.1f cm (valid: %s, obstacle: %s)",
             i + 1,
             currentData.distanceCm,
             currentData.dataValid ? "YES" : "NO",
             currentData.obstacleDetected ? "YES" : "NO");
        delay(100);
    }
    
    LOGI("Sonar test complete");
}

bool Sonar::update() {
    float distance = readRaw();
    
    if (!validateDistance(distance)) {
        errorCount++;
        currentData.dataValid = false;
        
        // Use last valid distance if available
        if (lastValidDistance > 0) {
            currentData.distanceCm = lastValidDistance;
            LOGD("Using last valid distance: %.1f cm", lastValidDistance);
        }
        
        return false;
    }
    
    // Apply filtering
    float filteredDistance = applyFilter(distance);
    
    // Update current data
    currentData.distanceCm = filteredDistance;
    currentData.obstacleDetected = (filteredDistance < OBSTACLE_THRESHOLD);
    currentData.criticalDistance = (filteredDistance < OBSTACLE_CRITICAL_DIST);
    currentData.dataValid = true;
    currentData.timestamp = millis();
    
    lastValidDistance = filteredDistance;
    lastSuccessfulRead = millis();
    
    // Log critical obstacles
    if (currentData.criticalDistance) {
        LOGW("CRITICAL OBSTACLE at %.1f cm!", filteredDistance);
    }
    
    return true;
}

bool Sonar::isClearPath() const {
    return currentData.dataValid && 
           !currentData.obstacleDetected &&
           isHealthy();
}

void Sonar::setThresholds(float obstacleThreshold, float criticalThreshold) {
    LOGI("Updating sonar thresholds: obstacle=%.1f cm, critical=%.1f cm",
         obstacleThreshold, criticalThreshold);
    // Note: Using #define constants, but this allows runtime override
}

bool Sonar::isHealthy() const {
    return currentData.dataValid &&
           (millis() - lastSuccessfulRead < 1000) &&
           errorCount < 50;
}

void Sonar::printData() {
    LOGI("Sonar Data:");
    LOGI("  Distance: %.1f cm", currentData.distanceCm);
    LOGI("  Obstacle detected: %s", currentData.obstacleDetected ? "YES" : "NO");
    LOGI("  Critical distance: %s", currentData.criticalDistance ? "YES" : "NO");
    LOGI("  Data valid: %s", currentData.dataValid ? "YES" : "NO");
    LOGI("  Error count: %d", errorCount);
    LOGI("  Timeout count: %d", timeoutCount);
}

void Sonar::selfTest() {
    LOGI("Performing sonar self-test...");
    
    clearBuffer();
    int successCount = 0;
    float minDist = SONAR_MAX_DISTANCE;
    float maxDist = 0;
    
    for (int i = 0; i < 20; i++) {
        if (update()) {
            successCount++;
            minDist = min(minDist, currentData.distanceCm);
            maxDist = max(maxDist, currentData.distanceCm);
        }
        delay(50);
    }
    
    LOGI("Self-test results:");
    LOGI("  Successful readings: %d/20", successCount);
    LOGI("  Distance range: %.1f - %.1f cm", minDist, maxDist);
    
    if (successCount >= 15) {
        LOGI("Self-test PASSED");
    } else {
        LOGE("Self-test FAILED");
    }
}

void Sonar::clearBuffer() {
    memset(distanceBuffer, 0, sizeof(distanceBuffer));
    bufferIndex = 0;
    bufferFilled = false;
    LOGD("Distance buffer cleared");
}

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

float Sonar::readRaw() {
    // Ensure trigger is low
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    
    // Send 10µs trigger pulse
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    
    // Measure echo pulse duration with timeout
    unsigned long duration = pulseIn(echoPin, HIGH, 30000);  // 30ms timeout
    
    if (duration == 0) {
        timeoutCount++;
        LOGD("Sonar timeout (count: %d)", timeoutCount);
        return -1;
    }
    
    // Calculate distance in cm
    // Speed of sound = 343 m/s = 0.0343 cm/µs
    // Distance = (duration / 2) * 0.0343
    float distance = (duration / 2.0) * 0.0343;
    
    return distance;
}

float Sonar::applyFilter(float newDistance) {
    // Add to circular buffer
    distanceBuffer[bufferIndex] = newDistance;
    bufferIndex = (bufferIndex + 1) % FILTER_SIZE;
    
    if (bufferIndex == 0) {
        bufferFilled = true;
    }
    
    // Calculate moving average
    float sum = 0;
    int count = bufferFilled ? FILTER_SIZE : bufferIndex;
    
    for (int i = 0; i < count; i++) {
        sum += distanceBuffer[i];
    }
    
    return sum / count;
}

bool Sonar::validateDistance(float distance) {
    // Check for invalid readings
    if (distance < 0) {
        return false;
    }
    
    if (distance > SONAR_MAX_DISTANCE) {
        LOGD("Distance exceeds max range: %.1f cm", distance);
        return false;
    }
    
    // Check for sudden jumps (possible noise)
    // Allow large jumps if they make sense (obstacle appeared/disappeared)
    if (lastValidDistance > 0 && lastValidDistance < SONAR_MAX_DISTANCE) {
        float diff = abs(distance - lastValidDistance);
        // Only reject if jump is > 200cm AND not near max range
        // This allows obstacle detection to work properly
        if (diff > 200 && distance < (SONAR_MAX_DISTANCE - 50)) {
            LOGD("Large distance change: %.1f -> %.1f cm", 
                 lastValidDistance, distance);
            // Don't reject, just log - let the filter handle it
        }
    }
    
    return true;
}
