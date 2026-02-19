// ============================================================================
// TEST MODE CONFIGURATION - Use this for initial hardware testing
// ============================================================================
// 
// USAGE:
// 1. Rename your current config.h to config_production.h (as backup)
// 2. Copy this file as config.h
// 3. Upload to ESP8266
// 4. Your robot will run without WiFi and with relaxed sensor requirements
//
// Once hardware is verified, switch back to production config.
// ============================================================================

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Version Information
#define FIRMWARE_VERSION "1.0.0-TEST-MODE"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// ============================================================================
// TEST MODE FLAGS - ENABLE FOR HARDWARE TESTING
// ============================================================================

#define TEST_MODE_ENABLED                // Master test mode flag
#define DISABLE_WIFI_REQUIREMENT         // Robot runs without WiFi
#define DISABLE_GPS_REQUIREMENT          // Robot runs without GPS
#define RELAX_SONAR_CHECKS              // Don't fail on sonar errors
#define DISABLE_SELF_TEST               // Skip self-test on startup
//#define MOTORS_ONLY_TEST                // Only test motors (disable all sensors)

// ============================================================================
// HARDWARE PIN CONFIGURATION - ESP8266 NodeMCU
// ============================================================================

// Motor Driver L298D Pins
#define MOTOR_LEFT_PWM      5    // D1 (GPIO5)
#define MOTOR_LEFT_IN1      0    // D3 (GPIO0)
#define MOTOR_LEFT_IN2      2    // D4 (GPIO2)
#define MOTOR_RIGHT_PWM     14   // D5 (GPIO14)
#define MOTOR_RIGHT_IN1     12   // D6 (GPIO12)
#define MOTOR_RIGHT_IN2     13   // D7 (GPIO13) - SHARED with Sonar Echo

// Ultrasonic Sensor Pins
#define SONAR_TRIGGER_PIN   15   // D8 (GPIO15)
#define SONAR_ECHO_PIN      13   // D7 (GPIO13) - SHARED with Motor Right IN2

// MPU6050 I2C Pins
#define I2C_SDA             4    // D2 (GPIO4)
#define I2C_SCL             5    // D1 (GPIO5) - SHARED with Motor Left PWM

// GPS Serial Pins
#define GPS_RX_PIN          3    // RX (GPIO3)
#define GPS_TX_PIN          1    // TX (GPIO1)

// Emergency Stop Button
#define EMERGENCY_STOP_PIN  16   // D0 (GPIO16)

// Status LED
#define STATUS_LED_PIN      2    // D4 (GPIO2)

// ============================================================================
// SENSOR CONFIGURATION - RELAXED FOR TESTING
// ============================================================================

// MPU6050 Settings
#define MPU6050_SAMPLE_RATE       50
#define MPU6050_GYRO_RANGE        250
#define MPU6050_ACCEL_RANGE       2
#define MPU6050_I2C_ADDRESS       0x68

// GPS Settings (NOT REQUIRED in test mode)
#define GPS_BAUD_RATE             9600
#define GPS_UPDATE_RATE           1
#define GPS_MIN_SATELLITES        4
#define GPS_ACCURACY_THRESHOLD    5.0

// Sonar Settings - RELAXED
#define SONAR_MAX_DISTANCE        400    // cm
#define SONAR_SAMPLE_RATE         5      // Hz (reduced)
#define OBSTACLE_THRESHOLD        30     // cm (closer threshold for testing)
#define OBSTACLE_CRITICAL_DIST    15     // cm (closer)

// ============================================================================
// MOTOR CONTROL CONFIGURATION
// ============================================================================

// Motor PWM Settings
#define MOTOR_PWM_FREQUENCY       1000
#define MOTOR_PWM_RANGE           1023
#define MOTOR_BASE_SPEED          400    // Slower for testing
#define MOTOR_MAX_SPEED           800    // Limited for safety
#define MOTOR_MIN_SPEED           256
#define MOTOR_TURN_SPEED          350    // Slower turns

// Distance Calibration
#define MM_PER_PWM_SECOND     25.0
#define TURN_RATE_DEG_PER_SEC 45.0

// ============================================================================
// COMMUNICATION CONFIGURATION - WiFi OPTIONAL
// ============================================================================

// WiFi Settings (NOT REQUIRED in test mode)
#define WIFI_SSID             "TestNetwork"  // Change if you want WiFi
#define WIFI_PASSWORD         "password"
#define WIFI_TIMEOUT          5000       // Shorter timeout
#define WIFI_RETRY_DELAY      10000      // Retry less often
#define WIFI_MAX_RETRIES      2          // Give up faster

// WebSocket Settings (NOT REQUIRED)
#define SERVER_HOST           "192.168.1.100"
#define SERVER_PORT           8000
#define WS_RECONNECT_INTERVAL 10000
#define WS_PING_INTERVAL      60000
#define WS_TIMEOUT            120000

// ============================================================================
// SAFETY CONFIGURATION - RELAXED FOR TESTING
// ============================================================================

// Watchdog Timer - RELAXED
#define WATCHDOG_TIMEOUT      8000       // Longer timeout
#define TASK_TIMEOUT          4000       // More lenient

// Safety Thresholds - RELAXED
#define MAX_TILT_ANGLE        45.0       // More forgiving
#define MAX_ACCELERATION      4.0        // Higher limit
#define LOW_BATTERY_VOLTAGE   6.0        // Lower threshold (for testing)
#define CRITICAL_BATTERY      5.0        // Lower threshold

// Collision Prevention
#define EMERGENCY_STOP_DIST   10         // Closer for testing
#define COLLISION_CHECK_RATE  20         // Less frequent

// ============================================================================
// LOGGING CONFIGURATION - VERBOSE FOR DEBUGGING
// ============================================================================

// Log Levels: 0=NONE, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG, 5=VERBOSE
#define LOG_LEVEL             3          // INFO level (more verbose)

// Log Output
#define LOG_TO_SERIAL         true
#define LOG_TO_WEBSOCKET      false
#define LOG_BUFFER_SIZE       256

// ============================================================================
// PERFORMANCE CONFIGURATION
// ============================================================================

// Update Rates - RELAXED
#define MAIN_LOOP_RATE        10         // Hz (slower loop for debugging)
#define SENSOR_UPDATE_RATE    10         // Hz
#define CONTROL_UPDATE_RATE   10         // Hz
#define TELEMETRY_RATE        2          // Hz

// ============================================================================
// STATE MACHINE CONFIGURATION
// ============================================================================

// Timeouts
#define STATE_TRANSITION_TIMEOUT  10000  // Longer timeout
#define IDLE_TIMEOUT              600000
#define NAVIGATION_TIMEOUT        1200000

// ============================================================================
// MEMORY MANAGEMENT
// ============================================================================

#define JSON_BUFFER_SIZE      512
#define COMMAND_QUEUE_SIZE    5
#define TELEMETRY_QUEUE_SIZE  5

// ============================================================================
// DEBUG OPTIONS - ENABLE FOR TESTING
// ============================================================================

#define DEBUG_SENSORS              // Enable sensor debug
#define DEBUG_MOTORS               // Enable motor debug
//#define DEBUG_NAVIGATION         // Enable navigation debug
#define DEBUG_STATE_MACHINE        // Enable state machine debug
//#define SIMULATE_GPS             // Simulate GPS if no module
#define DISABLE_WATCHDOG           // Disable watchdog for testing

// ============================================================================
// TEST MODE HEALTH CHECK CONFIGURATION
// ============================================================================

// In test mode, only motors are required
// WiFi, GPS, and Sonar are OPTIONAL - won't cause emergency stop

// Safety Check Intervals - RELAXED
#define SAFETY_CHECK_RATE     5      // Hz (less frequent)
#define HEALTH_CHECK_RATE     1      // Hz

// Performance Limits - RELAXED
#define MAX_LOOP_TIME         200    // ms (more lenient)
#define MAX_CPU_USAGE         95     // %
#define MIN_FREE_HEAP         5000   // bytes

// ============================================================================
// TEST MODE NOTES
// ============================================================================
//
// What works in TEST MODE:
// ✅ Motors will run
// ✅ IMU will work (tilt detection, orientation)
// ✅ Basic state machine
// ✅ Emergency stop button
// ✅ Serial logging/debugging
//
// What is OPTIONAL (won't cause errors):
// ⚠️ WiFi - will try to connect but won't fail if unavailable
// ⚠️ GPS - not required for operation
// ⚠️ Sonar - readings used if available, but errors ignored
//
// To return to production mode:
// 1. Rename this file to config_test_mode.h
// 2. Rename config_production.h back to config.h
// 3. Re-upload code
//
// ============================================================================

#endif // CONFIG_H
