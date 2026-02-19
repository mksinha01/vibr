#include "logger.h"
#include <stdarg.h>
#include <stdio.h>

// Static member initialization
LogLevel Logger::currentLevel = LOG_INFO;
char Logger::buffer[LOG_BUFFER_SIZE];
bool Logger::logInProgress = false;  // Simple lock for ESP8266
void (*Logger::remoteLogCallback)(const char*) = nullptr;

void Logger::init(LogLevel level) {
    currentLevel = level;
    logInProgress = false;
    
    Serial.println("\n╔════════════════════════════════════════════════╗");
    Serial.println("║     DELIVERY ROBOT - LOGGING SYSTEM INIT      ║");
    Serial.println("╚════════════════════════════════════════════════╝");
    logSystemInfo();
}

void Logger::setLevel(LogLevel level) {
    currentLevel = level;
    info("Logger", "Log level changed to %d", level);
}

void Logger::setRemoteCallback(void (*callback)(const char*)) {
    remoteLogCallback = callback;
}

const char* Logger::getLevelString(LogLevel level) {
    switch(level) {
        case LOG_ERROR:   return "ERROR";
        case LOG_WARN:    return "WARN ";
        case LOG_INFO:    return "INFO ";
        case LOG_DEBUG:   return "DEBUG";
        case LOG_VERBOSE: return "VERB ";
        default:          return "UNKNOWN";
    }
}

const char* Logger::getColorCode(LogLevel level) {
    switch(level) {
        case LOG_ERROR:   return "\033[1;31m"; // Red
        case LOG_WARN:    return "\033[1;33m"; // Yellow
        case LOG_INFO:    return "\033[1;32m"; // Green
        case LOG_DEBUG:   return "\033[1;36m"; // Cyan
        case LOG_VERBOSE: return "\033[1;37m"; // White
        default:          return "\033[0m";    // Reset
    }
}

void Logger::log(LogLevel level, const char* tag, const char* format, ...) {
    if (level > currentLevel) return;
    
    // Simple lock for ESP8266 (no RTOS)
    if (logInProgress) return;  // Skip if already logging
    logInProgress = true;
    
    // Format timestamp
    unsigned long timestamp = millis();
    unsigned long seconds = timestamp / 1000;
    unsigned long milliseconds = timestamp % 1000;
    
    // Build log message
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    
    // Print to serial with color
    if (LOG_TO_SERIAL) {
        Serial.printf("%s[%6lu.%03lu][%s][%s] %s\033[0m\n",
            getColorCode(level),
            seconds,
            milliseconds,
            getLevelString(level),
            tag,
            buffer
        );
    }
    
    // Send to remote logging if enabled
    if (LOG_TO_WEBSOCKET && remoteLogCallback != nullptr) {
        char remoteBuffer[LOG_BUFFER_SIZE + 100];
        snprintf(remoteBuffer, sizeof(remoteBuffer),
            "{\"timestamp\":%lu,\"level\":\"%s\",\"tag\":\"%s\",\"message\":\"%s\"}",
            timestamp, getLevelString(level), tag, buffer
        );
        remoteLogCallback(remoteBuffer);
    }
    
    logInProgress = false;
}

void Logger::error(const char* tag, const char* format, ...) {
    if (LOG_ERROR > currentLevel) return;
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    log(LOG_ERROR, tag, "%s", buffer);
}

void Logger::warn(const char* tag, const char* format, ...) {
    if (LOG_WARN > currentLevel) return;
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    log(LOG_WARN, tag, "%s", buffer);
}

void Logger::info(const char* tag, const char* format, ...) {
    if (LOG_INFO > currentLevel) return;
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    log(LOG_INFO, tag, "%s", buffer);
}

void Logger::debug(const char* tag, const char* format, ...) {
    if (LOG_DEBUG > currentLevel) return;
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    log(LOG_DEBUG, tag, "%s", buffer);
}

void Logger::verbose(const char* tag, const char* format, ...) {
    if (LOG_VERBOSE > currentLevel) return;
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);
    log(LOG_VERBOSE, tag, "%s", buffer);
}

void Logger::logSystemInfo() {
    info("System", "═══════════════════════════════════════════");
    info("System", "Firmware Version: %s", FIRMWARE_VERSION);
    info("System", "Build Date: %s %s", BUILD_DATE, BUILD_TIME);
    info("System", "CPU Frequency: %d MHz", ESP.getCpuFreqMHz());
    info("System", "Flash Size: %d bytes", ESP.getFlashChipSize());
    info("System", "Free Heap: %d bytes", ESP.getFreeHeap());
    info("System", "Chip ID: %08X", ESP.getChipId());  // ESP8266 specific
    info("System", "SDK Version: %s", ESP.getSdkVersion());
    info("System", "Core Version: %s", ESP.getCoreVersion().c_str());
    info("System", "═══════════════════════════════════════════");
}

void Logger::logMemoryStats() {
    info("Memory", "Free Heap: %d bytes", ESP.getFreeHeap());
    // Note: ESP8266 doesn't have getMaxAllocHeap()
    info("Memory", "Heap info available via ESP.getFreeHeap() only");
}
