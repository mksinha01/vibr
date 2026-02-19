#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <TinyGPS++.h>
#ifdef ESP32
    #include <HardwareSerial.h>
#else
    #include <SoftwareSerial.h>
#endif
#include "config.h"
#include "logger.h"

// ============================================================================
// GPS HAL - Global Positioning System
// Industry-standard GPS interface with accuracy monitoring
// ============================================================================

struct GPSData {
    // Position
    double latitude;
    double longitude;
    float altitude;  // meters
    
    // Velocity
    float speedKmh;
    float course;  // degrees
    
    // Quality indicators
    uint8_t satellites;
    float hdop;  // Horizontal dilution of precision
    bool fix;
    bool dataValid;
    
    // Timestamp
    uint8_t hour, minute, second;
    uint8_t day, month;
    uint16_t year;
    
    unsigned long timestamp;
};

class GPS {
private:
    TinyGPSPlus gps;
#ifdef ESP32
    HardwareSerial* gpsSerial;
#else
    SoftwareSerial* gpsSerial;  // ESP8266 uses software serial
#endif
    
    // Current state
    GPSData currentData;
    bool initialized;
    unsigned long lastUpdateTime;
    unsigned long lastFixTime;
    
    // Home position
    double homeLat, homeLon;
    bool homeSet;
    
    // Health monitoring
    unsigned int errorCount;
    unsigned int noFixCount;
    
    // Internal functions
    bool parseData();
    void updateDataStruct();
    bool hasGoodFix() const;
    
public:
    GPS();
    
    // Initialization
    bool init();
    void test();
    
    // Data acquisition
    bool update();
    GPSData getData() const { return currentData; }
    
    // Position queries
    double getLatitude() const { return currentData.latitude; }
    double getLongitude() const { return currentData.longitude; }
    bool hasFix() const { return currentData.fix; }
    uint8_t getSatellites() const { return currentData.satellites; }
    
    // Navigation helpers
    float distanceTo(double lat, double lon) const;  // meters
    float courseTo(double lat, double lon) const;    // degrees
    void setHome();
    float distanceToHome() const;
    float courseToHome() const;
    bool isAtLocation(double lat, double lon, float radiusM = 5.0) const;
    
    // Quality checks
    bool isAccurate() const;
    bool isHealthy() const;
    
    // Diagnostics
    void printData();
    void selfTest();
    unsigned int getErrorCount() const { return errorCount; }
};

#endif // GPS_H
