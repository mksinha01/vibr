#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "config.h"

// ============================================================================
// INDUSTRY STANDARD LOGGING SYSTEM
// Multi-level logging with timestamps, source tracking, and remote logging
// ============================================================================

enum LogLevel {
    LOG_NONE = 0,
    LOG_ERROR = 1,
    LOG_WARN = 2,
    LOG_INFO = 3,
    LOG_DEBUG = 4,
    LOG_VERBOSE = 5
};

class Logger {
private:
    static LogLevel currentLevel;
    static char buffer[LOG_BUFFER_SIZE];
    static bool logInProgress;  // Simple lock for ESP8266 (no RTOS)
    static void (*remoteLogCallback)(const char* message);
    
    static const char* getLevelString(LogLevel level);
    static const char* getColorCode(LogLevel level);
    static void formatMessage(LogLevel level, const char* tag, const char* message);
    
public:
    static void init(LogLevel level = LOG_INFO);
    static void setLevel(LogLevel level);
    static void setRemoteCallback(void (*callback)(const char*));
    
    // Core logging functions
    static void log(LogLevel level, const char* tag, const char* format, ...);
    
    // Convenience functions
    static void error(const char* tag, const char* format, ...);
    static void warn(const char* tag, const char* format, ...);
    static void info(const char* tag, const char* format, ...);
    static void debug(const char* tag, const char* format, ...);
    static void verbose(const char* tag, const char* format, ...);
    
    // System information logging
    static void logSystemInfo();
    static void logMemoryStats();
};

// Macros for easy logging with automatic tag
#define LOG_TAG __FILE__

#define LOGE(format, ...) Logger::error(LOG_TAG, format, ##__VA_ARGS__)
#define LOGW(format, ...) Logger::warn(LOG_TAG, format, ##__VA_ARGS__)
#define LOGI(format, ...) Logger::info(LOG_TAG, format, ##__VA_ARGS__)
#define LOGD(format, ...) Logger::debug(LOG_TAG, format, ##__VA_ARGS__)
#define LOGV(format, ...) Logger::verbose(LOG_TAG, format, ##__VA_ARGS__)

#endif // LOGGER_H
