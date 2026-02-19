#include "mpu6050.h"

static const char* TAG = "MPU6050";

MPU6050Sensor::MPU6050Sensor()
    : accelOffsetX(0), accelOffsetY(0), accelOffsetZ(0)
    , gyroOffsetX(0), gyroOffsetY(0), gyroOffsetZ(0)
    , currentYaw(0)
    , lastUpdateTime(0)
    , filteredPitch(0), filteredRoll(0)
    , initialized(false)
    , errorCount(0)
    , lastSuccessfulRead(0) {
    memset(&currentData, 0, sizeof(IMUData));
}

bool MPU6050Sensor::init() {
    LOGI("Initializing MPU6050...");
    
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(400000);  // 400kHz I2C
    
    if (!mpu.begin()) {
        LOGE("Failed to find MPU6050 chip!");
        return false;
    }
    
    LOGI("MPU6050 Found!");
    
    // Configure MPU6050
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    delay(100);
    
    initialized = true;
    lastSuccessfulRead = millis();
    
    LOGI("MPU6050 initialized successfully");
    return true;
}

bool MPU6050Sensor::calibrate(int samples) {
    if (!initialized) {
        LOGE("Cannot calibrate - not initialized");
        return false;
    }
    
    LOGI("Starting calibration with %d samples...", samples);
    LOGI("Keep robot stationary and level!");
    
    float sumAx = 0, sumAy = 0, sumAz = 0;
    float sumGx = 0, sumGy = 0, sumGz = 0;
    
    for (int i = 0; i < samples; i++) {
        sensors_event_t a, g, temp;
        if (!mpu.getEvent(&a, &g, &temp)) {
            LOGW("Failed to read during calibration sample %d", i);
            continue;
        }
        
        sumAx += a.acceleration.x;
        sumAy += a.acceleration.y;
        sumAz += a.acceleration.z - 9.81;  // Remove gravity
        sumGx += g.gyro.x;
        sumGy += g.gyro.y;
        sumGz += g.gyro.z;
        
        if (i % 100 == 0) {
            LOGI("Calibration progress: %d%%", (i * 100) / samples);
        }
        
        delay(10);
    }
    
    accelOffsetX = sumAx / samples;
    accelOffsetY = sumAy / samples;
    accelOffsetZ = sumAz / samples;
    gyroOffsetX = sumGx / samples;
    gyroOffsetY = sumGy / samples;
    gyroOffsetZ = sumGz / samples;
    
    LOGI("Calibration complete!");
    LOGI("Accel offsets: X=%.3f, Y=%.3f, Z=%.3f", 
         accelOffsetX, accelOffsetY, accelOffsetZ);
    LOGI("Gyro offsets: X=%.3f, Y=%.3f, Z=%.3f",
         gyroOffsetX, gyroOffsetY, gyroOffsetZ);
    
    return true;
}

void MPU6050Sensor::loadCalibration(float ax, float ay, float az, 
                                     float gx, float gy, float gz) {
    accelOffsetX = ax;
    accelOffsetY = ay;
    accelOffsetZ = az;
    gyroOffsetX = gx;
    gyroOffsetY = gy;
    gyroOffsetZ = gz;
    
    LOGI("Calibration data loaded");
}

bool MPU6050Sensor::update() {
    if (!initialized) return false;
    
    sensors_event_t a, g, temp;
    
    if (!mpu.getEvent(&a, &g, &temp)) {
        errorCount++;
        LOGW("Failed to read MPU6050 data (errors: %d)", errorCount);
        currentData.dataValid = false;
        return false;
    }
    
    // Apply calibration
    applyCalibration(a, g);
    
    // Update raw data
    currentData.accelX = a.acceleration.x;
    currentData.accelY = a.acceleration.y;
    currentData.accelZ = a.acceleration.z;
    currentData.gyroX = g.gyro.x;
    currentData.gyroY = g.gyro.y;
    currentData.gyroZ = g.gyro.z;
    currentData.temperature = temp.temperature;
    currentData.timestamp = millis();
    currentData.dataValid = true;
    
    // Calculate orientation
    calculateOrientation(a, g);
    
    lastSuccessfulRead = millis();
    return true;
}

bool MPU6050Sensor::isTilted(float maxAngle) const {
    return abs(currentData.pitch) > maxAngle || 
           abs(currentData.roll) > maxAngle;
}

bool MPU6050Sensor::isHighAcceleration(float maxG) const {
    float totalAccel = sqrt(
        currentData.accelX * currentData.accelX +
        currentData.accelY * currentData.accelY +
        currentData.accelZ * currentData.accelZ
    ) / 9.81;
    
    return totalAccel > maxG;
}

bool MPU6050Sensor::isHealthy() const {
    return initialized && 
           currentData.dataValid &&
           (millis() - lastSuccessfulRead < 1000) &&
           errorCount < 100;
}

void MPU6050Sensor::resetYaw() {
    currentYaw = 0;
    LOGI("Yaw reset to 0");
}

float MPU6050Sensor::getRelativeYaw(float targetYaw) const {
    float diff = targetYaw - currentYaw;
    
    // Normalize to -180 to 180
    while (diff > 180) diff -= 360;
    while (diff < -180) diff += 360;
    
    return diff;
}

void MPU6050Sensor::printData() {
    LOGI("IMU Data:");
    LOGI("  Accel: X=%.2f Y=%.2f Z=%.2f m/s²", 
         currentData.accelX, currentData.accelY, currentData.accelZ);
    LOGI("  Gyro: X=%.2f Y=%.2f Z=%.2f rad/s",
         currentData.gyroX, currentData.gyroY, currentData.gyroZ);
    LOGI("  Orientation: Pitch=%.1f Roll=%.1f Yaw=%.1f deg",
         currentData.pitch, currentData.roll, currentYaw);
    LOGI("  Temperature: %.1f°C", currentData.temperature);
    LOGI("  Error count: %d", errorCount);
}

void MPU6050Sensor::selfTest() {
    LOGI("Performing MPU6050 self-test...");
    
    if (!initialized) {
        LOGE("Self-test failed: not initialized");
        return;
    }
    
    // Read multiple samples
    int successCount = 0;
    for (int i = 0; i < 10; i++) {
        if (update()) successCount++;
        delay(10);
    }
    
    LOGI("Self-test: %d/10 reads successful", successCount);
    
    if (successCount >= 8) {
        LOGI("Self-test PASSED");
    } else {
        LOGE("Self-test FAILED");
    }
}

bool MPU6050Sensor::isCalibrated() const {
    return (accelOffsetX != 0 || accelOffsetY != 0 || accelOffsetZ != 0 ||
            gyroOffsetX != 0 || gyroOffsetY != 0 || gyroOffsetZ != 0);
}

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

void MPU6050Sensor::applyCalibration(sensors_event_t& a, sensors_event_t& g) {
    a.acceleration.x -= accelOffsetX;
    a.acceleration.y -= accelOffsetY;
    a.acceleration.z -= accelOffsetZ;
    g.gyro.x -= gyroOffsetX;
    g.gyro.y -= gyroOffsetY;
    g.gyro.z -= gyroOffsetZ;
}

void MPU6050Sensor::calculateOrientation(const sensors_event_t& a, 
                                         const sensors_event_t& g) {
    // Calculate time delta
    unsigned long currentTime = millis();
    float dt = (currentTime - lastUpdateTime) / 1000.0;
    lastUpdateTime = currentTime;
    
    if (dt <= 0 || dt > 1.0) {
        dt = 0.01;  // Default to 100Hz if timing is off
    }
    
    // Calculate pitch and roll from accelerometer
    float accelPitch = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
    float accelRoll = atan2(-a.acceleration.x, a.acceleration.z) * 180.0 / PI;
    
    // Apply complementary filter
    complementaryFilter(accelPitch, accelRoll, dt);
    
    // Integrate yaw from gyroscope
    currentYaw += g.gyro.z * dt * 180.0 / PI;
    
    // Normalize yaw to -180 to 180
    while (currentYaw > 180) currentYaw -= 360;
    while (currentYaw < -180) currentYaw += 360;
    
    // Update current data
    currentData.pitch = filteredPitch;
    currentData.roll = filteredRoll;
    currentData.yaw = currentYaw;
}

void MPU6050Sensor::complementaryFilter(float accelPitch, float accelRoll, float dt) {
    // Integrate gyro data
    float gyroPitch = filteredPitch + currentData.gyroX * dt * 180.0 / PI;
    float gyroRoll = filteredRoll + currentData.gyroY * dt * 180.0 / PI;
    
    // Combine with accelerometer data using complementary filter
    filteredPitch = ALPHA * gyroPitch + (1.0 - ALPHA) * accelPitch;
    filteredRoll = ALPHA * gyroRoll + (1.0 - ALPHA) * accelRoll;
}
