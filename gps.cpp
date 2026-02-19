#include "gps.h"

static const char* TAG = "GPS";

GPS::GPS()
    : gpsSerial(nullptr)
    , initialized(false)
    , lastUpdateTime(0)
    , lastFixTime(0)
    , homeLat(0), homeLon(0)
    , homeSet(false)
    , errorCount(0)
    , noFixCount(0) {
    memset(&currentData, 0, sizeof(GPSData));
}

bool GPS::init() {
    LOGI("Initializing GPS module...");
    
    // Initialize GPS serial port
#ifdef ESP32
    gpsSerial = new HardwareSerial(2);
    gpsSerial->begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    LOGI("GPS using HardwareSerial (ESP32)");
#else
    // ESP8266 uses SoftwareSerial
    gpsSerial = new SoftwareSerial(GPS_RX_PIN, GPS_TX_PIN);
    gpsSerial->begin(GPS_BAUD_RATE);
    LOGI("GPS using SoftwareSerial (ESP8266) on pins RX:%d TX:%d", GPS_RX_PIN, GPS_TX_PIN);
#endif
    
    delay(100);
    
    if (gpsSerial) {
        LOGI("GPS serial initialized on pins RX:%d TX:%d", GPS_RX_PIN, GPS_TX_PIN);
        initialized = true;
        
        // Wait for initial data
        LOGI("Waiting for GPS fix... (this may take 30-60 seconds)");
        unsigned long startTime = millis();
        
        while (millis() - startTime < 3000) {  // 3 second initial check
            update();
            delay(100);
        }
        
        if (currentData.fix) {
            LOGI("GPS fix acquired!");
        } else {
            LOGW("No GPS fix yet, but initialization successful");
        }
        
        return true;
    }
    
    LOGE("Failed to initialize GPS serial");
    return false;
}

void GPS::test() {
    LOGI("Starting GPS test...");
    LOGI("Acquiring GPS data for 30 seconds...");
    
    unsigned long startTime = millis();
    int updateCount = 0;
    
    while (millis() - startTime < 30000) {
        if (update()) {
            updateCount++;
            if (updateCount % 10 == 0) {
                printData();
            }
        }
        delay(100);
    }
    
    LOGI("GPS test complete. Updates received: %d", updateCount);
}

bool GPS::update() {
    if (!initialized || !gpsSerial) return false;
    
    bool newData = false;
    
    // Read all available GPS data
    while (gpsSerial->available() > 0) {
        char c = gpsSerial->read();
        
        if (gps.encode(c)) {
            newData = true;
        }
    }
    
    if (newData) {
        if (parseData()) {
            updateDataStruct();
            lastUpdateTime = millis();
            
            if (currentData.fix) {
                lastFixTime = millis();
                noFixCount = 0;
            } else {
                noFixCount++;
                if (noFixCount % 100 == 0) {
                    LOGW("No GPS fix for %d updates", noFixCount);
                }
            }
            
            return true;
        }
    }
    
    return false;
}

float GPS::distanceTo(double lat, double lon) const {
    if (!currentData.fix) return -1;
    
    return TinyGPSPlus::distanceBetween(
        currentData.latitude, currentData.longitude,
        lat, lon
    );
}

float GPS::courseTo(double lat, double lon) const {
    if (!currentData.fix) return -1;
    
    return TinyGPSPlus::courseTo(
        currentData.latitude, currentData.longitude,
        lat, lon
    );
}

void GPS::setHome() {
    if (!currentData.fix) {
        LOGW("Cannot set home - no GPS fix");
        return;
    }
    
    homeLat = currentData.latitude;
    homeLon = currentData.longitude;
    homeSet = true;
    
    LOGI("Home position set: %.6f, %.6f", homeLat, homeLon);
}

float GPS::distanceToHome() const {
    if (!homeSet || !currentData.fix) return -1;
    
    return distanceTo(homeLat, homeLon);
}

float GPS::courseToHome() const {
    if (!homeSet || !currentData.fix) return -1;
    
    return courseTo(homeLat, homeLon);
}

bool GPS::isAtLocation(double lat, double lon, float radiusM) const {
    if (!currentData.fix) return false;
    
    float distance = distanceTo(lat, lon);
    return distance >= 0 && distance <= radiusM;
}

bool GPS::isAccurate() const {
    return currentData.fix &&
           currentData.satellites >= GPS_MIN_SATELLITES &&
           currentData.hdop < GPS_ACCURACY_THRESHOLD;
}

bool GPS::isHealthy() const {
    return initialized &&
           (millis() - lastUpdateTime < 5000) &&
           (millis() - lastFixTime < 30000) &&
           errorCount < 100;
}

void GPS::printData() {
    LOGI("GPS Data:");
    LOGI("  Fix: %s | Satellites: %d | HDOP: %.1f",
         currentData.fix ? "YES" : "NO",
         currentData.satellites,
         currentData.hdop);
    
    if (currentData.fix) {
        LOGI("  Position: %.6f, %.6f", 
             currentData.latitude, currentData.longitude);
        LOGI("  Altitude: %.1f m", currentData.altitude);
        LOGI("  Speed: %.1f km/h | Course: %.1f°",
             currentData.speedKmh, currentData.course);
        LOGI("  Time: %02d:%02d:%02d Date: %02d/%02d/%04d",
             currentData.hour, currentData.minute, currentData.second,
             currentData.day, currentData.month, currentData.year);
        
        if (homeSet) {
            LOGI("  Distance to home: %.1f m", distanceToHome());
        }
    }
}

void GPS::selfTest() {
    LOGI("Performing GPS self-test...");
    
    if (!initialized) {
        LOGE("Self-test failed: not initialized");
        return;
    }
    
    LOGI("Reading GPS data for 10 seconds...");
    
    unsigned long startTime = millis();
    int validUpdates = 0;
    int fixCount = 0;
    
    while (millis() - startTime < 10000) {
        if (update()) {
            validUpdates++;
            if (currentData.fix) fixCount++;
        }
        delay(100);
    }
    
    LOGI("Self-test results:");
    LOGI("  Valid updates: %d", validUpdates);
    LOGI("  Updates with fix: %d", fixCount);
    LOGI("  Current satellites: %d", currentData.satellites);
    
    if (validUpdates > 0) {
        LOGI("Self-test PASSED");
    } else {
        LOGE("Self-test FAILED - no data received");
    }
}

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

bool GPS::parseData() {
    if (!gps.location.isValid() && !gps.date.isValid() && !gps.time.isValid()) {
        return false;
    }
    
    return true;
}

void GPS::updateDataStruct() {
    // Position
    if (gps.location.isValid()) {
        currentData.latitude = gps.location.lat();
        currentData.longitude = gps.location.lng();
        currentData.fix = true;
    } else {
        currentData.fix = false;
    }
    
    // Altitude
    if (gps.altitude.isValid()) {
        currentData.altitude = gps.altitude.meters();
    }
    
    // Speed and course
    if (gps.speed.isValid()) {
        currentData.speedKmh = gps.speed.kmph();
    }
    
    if (gps.course.isValid()) {
        currentData.course = gps.course.deg();
    }
    
    // Quality indicators
    if (gps.satellites.isValid()) {
        currentData.satellites = gps.satellites.value();
    }
    
    if (gps.hdop.isValid()) {
        currentData.hdop = gps.hdop.hdop();
    }
    
    // Time and date
    if (gps.time.isValid()) {
        currentData.hour = gps.time.hour();
        currentData.minute = gps.time.minute();
        currentData.second = gps.time.second();
    }
    
    if (gps.date.isValid()) {
        currentData.day = gps.date.day();
        currentData.month = gps.date.month();
        currentData.year = gps.date.year();
    }
    
    currentData.dataValid = gps.location.isValid();
    currentData.timestamp = millis();
}

bool GPS::hasGoodFix() const {
    return currentData.fix &&
           currentData.satellites >= GPS_MIN_SATELLITES &&
           gps.location.age() < 1000;
}
