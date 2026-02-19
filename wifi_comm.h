#ifndef WIFI_COMM_H
#define WIFI_COMM_H

#include <Arduino.h>
#ifdef ESP32
    #include <WiFi.h>
#else
    #include <ESP8266WiFi.h>
#endif
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "config.h"
#include "logger.h"

// ============================================================================
// WIFI COMMUNICATION MODULE
// Industry-standard network communication with auto-reconnect
// ============================================================================

enum ConnectionStatus {
    CONN_DISCONNECTED,
    CONN_CONNECTING,
    CONN_CONNECTED,
    CONN_ERROR
};

struct CommandMessage {
    String type;  // "move", "turn", "stop", "status_request"
    float param1;  // distance or angle
    float param2;  // speed
    unsigned long timestamp;
    bool valid;
};

class WiFiComm {
private:
    WebSocketsClient webSocket;
    ConnectionStatus status;
    
    // Connection management
    unsigned long lastConnectionAttempt;
    unsigned int reconnectAttempts;
    bool autoReconnect;
    
    // Message handling
    StaticJsonDocument<JSON_BUFFER_SIZE> jsonDoc;
    CommandMessage lastCommand;
    
    // Callbacks
    void (*commandCallback)(CommandMessage cmd);
    void (*statusCallback)(ConnectionStatus status);
    
    // Internal functions
    void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
    static void webSocketEventStatic(WStype_t type, uint8_t* payload, size_t length);
    bool parseCommand(const char* json, CommandMessage& cmd);
    void handleReconnection();
    
    static WiFiComm* instance;  // For static callback
    
public:
    WiFiComm();
    
    // Initialization
    bool init();
    bool connect();
    void disconnect();
    
    // Connection management
    void update();  // Call regularly in loop
    bool isConnected() const { return status == CONN_CONNECTED; }
    ConnectionStatus getStatus() const { return status; }
    int getSignalStrength();  // RSSI in dBm
    
    // Message sending
    bool sendTelemetry(const char* data);
    bool sendStatus(const char* state, const char* message);
    bool sendPosition(double lat, double lon, float heading);
    bool sendError(const char* error);
    
    // Callbacks
    void setCommandCallback(void (*callback)(CommandMessage));
    void setStatusCallback(void (*callback)(ConnectionStatus));
    
    // Diagnostics
    void printStatus();
    String getIPAddress();
    String getMACAddress();
};

#endif // WIFI_COMM_H
