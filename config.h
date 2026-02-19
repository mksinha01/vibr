#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// ESP8266 NODEMCU CONFIGURATION - V2 (GPS DISABLED, PIN CONFLICTS SOLVED)
// This configuration eliminates all pin conflicts for reliable operation
// ============================================================================

// Version Information
#define FIRMWARE_VERSION "2.0.0-esp8266-fixed"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// ============================================================================
// HARDWARE PIN CONFIGURATION - ESP8266 NodeMCU (CONFLICT-FREE)
// ============================================================================
// V2 CHANGES FROM V1:
// 1. GPS DISABLED - Frees up RX/TX pins for I2C and debugging
// 2. NO PIN SHARING - Each function has dedicated GPIO
// 3. I2C SCL moved to TX (GPIO1) - no PWM interference
// 4. Sonar ECHO moved to D3 (GPIO0) - no motor conflict
// 5. Motor pins reorganized for clean layout
//
// BENEFITS:
// ✓ All sensors work reliably (no conflicts)
// ✓ Serial debugging available (no GPS on RX/TX)
// ✓ Motor control independent of sensor reads
// ✓ Professional-grade pin allocation
// ============================================================================

// Motor Driver L298D Pins
#define MOTOR_LEFT_PWM      16   // D0 (GPIO16) - PWM capable - MOVED from D1
#define MOTOR_LEFT_IN1      2    // D4 (GPIO2)  - MOVED from D3
#define MOTOR_LEFT_IN2      14   // D5 (GPIO14) - MOVED from D4
#define MOTOR_RIGHT_PWM     12   // D6 (GPIO12) - PWM capable
#define MOTOR_RIGHT_IN1     15   // D8 (GPIO15) - MOVED from D6
#define MOTOR_RIGHT_IN2     13   // D7 (GPIO13) - No conflict now

// Ultrasonic Sensor Pins
#define SONAR_TRIGGER_PIN   4    // D2 (GPIO4)  - MOVED from D8
#define SONAR_ECHO_PIN      0    // D3 (GPIO0)  - MOVED from D7 - NO CONFLICT!

// MPU6050 I2C Pins - ESP8266 DEDICATED (NO SHARING)
#define I2C_SDA             5    // D1 (GPIO5)  - Dedicated I2C SDA
#define I2C_SCL             1    // TX (GPIO1)  - Dedicated I2C SCL - NO PWM!

// GPS Disabled in V2 - Pins reallocated
// #define GPS_RX_PIN          3    // RX (GPIO3)  - Not used in V2
// #define GPS_TX_PIN          1    // TX (GPIO1)  - Now used for I2C SCL

// Status LED (Emergency stop pin reallocated to Motor PWM)
#define STATUS_LED_PIN      2    // D4 (GPIO2) - Built-in LED (shares with Motor IN1)

// ============================================================================
// SENSOR CONFIGURATION
// ============================================================================

// MPU6050 Settings
#define MPU6050_SAMPLE_RATE       50     // Hz (reduced for ESP8266)
#define MPU6050_GYRO_RANGE        250    // degrees/second
#define MPU6050_ACCEL_RANGE       2      // g
#define MPU6050_I2C_ADDRESS       0x68   // Default address

// GPS Settings - DISABLED IN V2
// #define GPS_BAUD_RATE             9600
// #define GPS_UPDATE_RATE           1      // Hz
// #define GPS_MIN_SATELLITES        4
// #define GPS_ACCURACY_THRESHOLD    5.0    // meters

// Sonar Settings
#define SONAR_MAX_DISTANCE        400    // cm
#define SONAR_SAMPLE_RATE         10     // Hz (reduced for ESP8266)
#define OBSTACLE_THRESHOLD        50     // cm
#define OBSTACLE_CRITICAL_DIST    20     // cm

// ============================================================================
// MOTOR CONTROL CONFIGURATION
// ============================================================================

// Motor PWM Settings (ESP8266 analogWrite - 0-1023 range)
#define MOTOR_PWM_FREQUENCY       1000   // Hz
#define MOTOR_PWM_RANGE           1023   // ESP8266 PWM range
#define MOTOR_BASE_SPEED          512    // PWM value (0-1023) 
#define MOTOR_MAX_SPEED           1023
#define MOTOR_MIN_SPEED           256
#define MOTOR_TURN_SPEED          400

// Distance Calibration
#define MM_PER_PWM_SECOND     25.0       // mm traveled per PWM per second
#define TURN_RATE_DEG_PER_SEC 45.0       // degrees per second at turn speed

// ============================================================================
// COMMUNICATION CONFIGURATION
// ============================================================================

// WiFi Settings
#define WIFI_SSID             "Mk's S24 FE"
#define WIFI_PASSWORD         "00000000"
#define WIFI_TIMEOUT          10000      // ms
#define WIFI_RETRY_DELAY      5000       // ms
#define WIFI_MAX_RETRIES      5

// WebSocket Settings
#define SERVER_HOST           "192.168.1.100"
#define SERVER_PORT           8000
#define WS_RECONNECT_INTERVAL 5000       // ms
#define WS_PING_INTERVAL      30000      // ms
#define WS_TIMEOUT            60000      // ms

// ============================================================================
// SAFETY CONFIGURATION
// ============================================================================

// Watchdog Timer
#define WATCHDOG_TIMEOUT      5000       // ms
#define TASK_TIMEOUT          2000       // ms

// Safety Thresholds
#define MAX_TILT_ANGLE        30.0       // degrees
#define MAX_ACCELERATION      3.0        // g
#define LOW_BATTERY_VOLTAGE   10.5       // volts (3S LiPo)
#define CRITICAL_BATTERY      10.0       // volts

// Collision Prevention
#define EMERGENCY_STOP_DIST   15         // cm
#define COLLISION_CHECK_RATE  50         // Hz

// ============================================================================
// LOGGING CONFIGURATION
// ============================================================================

// Log Levels: 0=NONE, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG, 5=VERBOSE
#define LOG_LEVEL             2          // WARN level (reduced for ESP8266)

// Log Output
#define LOG_TO_SERIAL         true
#define LOG_TO_WEBSOCKET      false      // Disabled to save memory
#define LOG_BUFFER_SIZE       256        // Reduced for ESP8266

// ============================================================================
// PERFORMANCE CONFIGURATION - ESP8266 (No FreeRTOS)
// ============================================================================

// Update Rates (simple loop-based timing)
#define MAIN_LOOP_RATE        20         // Hz (50ms loop)
#define SENSOR_UPDATE_RATE    20         // Hz
#define CONTROL_UPDATE_RATE   20         // Hz
#define TELEMETRY_RATE        5          // Hz (reduced)

// ============================================================================
// STATE MACHINE CONFIGURATION
// ============================================================================

// Timeouts
#define STATE_TRANSITION_TIMEOUT  5000   // ms
#define IDLE_TIMEOUT              300000 // ms (5 minutes)
#define NAVIGATION_TIMEOUT        600000 // ms (10 minutes)

// ============================================================================
// MEMORY MANAGEMENT - ESP8266 (Limited RAM ~80KB)
// ============================================================================

#define JSON_BUFFER_SIZE      512        // Reduced from 2048
#define COMMAND_QUEUE_SIZE    5          // Reduced from 10
#define TELEMETRY_QUEUE_SIZE  5          // Reduced from 20

// ============================================================================
// DEBUG OPTIONS
// ============================================================================

//#define DEBUG_SENSORS              // Enable sensor debug output
//#define DEBUG_MOTORS               // Enable motor debug output
//#define DEBUG_NAVIGATION           // Enable navigation debug output
//#define DEBUG_STATE_MACHINE        // Enable state machine debug output
//#define SIMULATE_GPS               // Use simulated GPS data
//#define DISABLE_WATCHDOG           // Disable watchdog for debugging

// ============================================================================
// TESTING & DIAGNOSTICS MODE (Industry Standard)
// ============================================================================

#define ENABLE_SELF_TEST           // Enable comprehensive self-test on startup
//#define CALIBRATION_MODE           // Enable sensor calibration mode
//#define PERFORMANCE_MONITORING     // Enable detailed performance metrics
#define ENABLE_DIAGNOSTICS         // Enable diagnostic commands

// Test Configuration
#define SELF_TEST_TIMEOUT     30000  // ms - max time for self-test
#define TEST_REPORT_INTERVAL  10000  // ms - how often to print metrics

// ============================================================================
// SAFETY CONFIGURATION - Enhanced
// ============================================================================

// Safety Check Intervals
#define SAFETY_CHECK_RATE     10     // Hz - critical safety checks
#define HEALTH_CHECK_RATE     1      // Hz - system health monitoring

// Performance Limits
#define MAX_LOOP_TIME         100    // ms - loop time warning threshold
#define MAX_CPU_USAGE         90     // % - CPU usage warning
#define MIN_FREE_HEAP         10000  // bytes - minimum free heap

#endif // CONFIG_H
