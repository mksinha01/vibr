#include "wifi_comm.h"

static const char* TAG = "WiFiComm";

WiFiComm* WiFiComm::instance = nullptr;

WiFiComm::WiFiComm()
    : status(CONN_DISCONNECTED)
    , lastConnectionAttempt(0)
    , reconnectAttempts(0)
    , autoReconnect(true)
    , commandCallback(nullptr)
    , statusCallback(nullptr) {
    
    instance = this;
    memset(&lastCommand, 0, sizeof(CommandMessage));
}

bool WiFiComm::init() {
    LOGI("Initializing WiFi communication...");
    
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);  // We handle reconnection manually
    
    return connect();
}

bool WiFiComm::connect() {
    if (status == CONN_CONNECTING || status == CONN_CONNECTED) {
        LOGW("Already connected or connecting");
        return true;
    }
    
    LOGI("Connecting to WiFi: %s", WIFI_SSID);
    status = CONN_CONNECTING;
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_TIMEOUT) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        LOGI("WiFi connected!");
        LOGI("IP Address: %s", WiFi.localIP().toString().c_str());
        LOGI("Signal Strength: %d dBm", WiFi.RSSI());
        
        // Connect to WebSocket server
        LOGI("Connecting to WebSocket server: %s:%d", SERVER_HOST, SERVER_PORT);
        webSocket.begin(SERVER_HOST, SERVER_PORT, "/ws");
        webSocket.onEvent(webSocketEventStatic);
        webSocket.setReconnectInterval(WS_RECONNECT_INTERVAL);
        
        status = CONN_CONNECTED;
        reconnectAttempts = 0;
        
        if (statusCallback) {
            statusCallback(CONN_CONNECTED);
        }
        
        return true;
    } else {
        LOGE("WiFi connection failed!");
        status = CONN_ERROR;
        reconnectAttempts++;
        
        if (statusCallback) {
            statusCallback(CONN_ERROR);
        }
        
        return false;
    }
}

void WiFiComm::disconnect() {
    LOGI("Disconnecting WiFi...");
    webSocket.disconnect();
    WiFi.disconnect();
    status = CONN_DISCONNECTED;
}

void WiFiComm::update() {
    // Handle WebSocket events
    if (status == CONN_CONNECTED) {
        webSocket.loop();
        
        // Check if WiFi is still connected
        if (WiFi.status() != WL_CONNECTED) {
            LOGW("WiFi connection lost!");
            status = CONN_DISCONNECTED;
            if (statusCallback) {
                statusCallback(CONN_DISCONNECTED);
            }
        }
    }
    
    // Handle auto-reconnection
    if (autoReconnect && status != CONN_CONNECTED && status != CONN_CONNECTING) {
        if (millis() - lastConnectionAttempt >= WIFI_RETRY_DELAY) {
            if (reconnectAttempts < WIFI_MAX_RETRIES) {
                LOGI("Attempting to reconnect (attempt %d/%d)...", 
                     reconnectAttempts + 1, WIFI_MAX_RETRIES);
                lastConnectionAttempt = millis();
                connect();
            } else {
                LOGE("Max reconnection attempts reached");
                autoReconnect = false;
            }
        }
    }
}

int WiFiComm::getSignalStrength() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    }
    return -100;  // No signal
}

bool WiFiComm::sendTelemetry(const char* data) {
    if (!isConnected()) {
        LOGW("Cannot send telemetry - not connected");
        return false;
    }
    
    jsonDoc.clear();
    jsonDoc["type"] = "telemetry";
    jsonDoc["data"] = data;
    jsonDoc["timestamp"] = millis();
    
    String output;
    serializeJson(jsonDoc, output);
    
    return webSocket.sendTXT(output);
}

bool WiFiComm::sendStatus(const char* state, const char* message) {
    if (!isConnected()) return false;
    
    jsonDoc.clear();
    jsonDoc["type"] = "status";
    jsonDoc["state"] = state;
    jsonDoc["message"] = message;
    jsonDoc["timestamp"] = millis();
    
    String output;
    serializeJson(jsonDoc, output);
    
    return webSocket.sendTXT(output);
}

bool WiFiComm::sendPosition(double lat, double lon, float heading) {
    if (!isConnected()) return false;
    
    jsonDoc.clear();
    jsonDoc["type"] = "position";
    jsonDoc["latitude"] = lat;
    jsonDoc["longitude"] = lon;
    jsonDoc["heading"] = heading;
    jsonDoc["timestamp"] = millis();
    
    String output;
    serializeJson(jsonDoc, output);
    
    return webSocket.sendTXT(output);
}

bool WiFiComm::sendError(const char* error) {
    if (!isConnected()) return false;
    
    jsonDoc.clear();
    jsonDoc["type"] = "error";
    jsonDoc["error"] = error;
    jsonDoc["timestamp"] = millis();
    
    String output;
    serializeJson(jsonDoc, output);
    
    return webSocket.sendTXT(output);
}

void WiFiComm::setCommandCallback(void (*callback)(CommandMessage)) {
    commandCallback = callback;
}

void WiFiComm::setStatusCallback(void (*callback)(ConnectionStatus)) {
    statusCallback = callback;
}

void WiFiComm::printStatus() {
    LOGI("WiFi Status:");
    LOGI("  Connection: %s", 
         status == CONN_CONNECTED ? "Connected" :
         status == CONN_CONNECTING ? "Connecting" :
         status == CONN_DISCONNECTED ? "Disconnected" : "Error");
    
    if (WiFi.status() == WL_CONNECTED) {
        LOGI("  IP Address: %s", WiFi.localIP().toString().c_str());
        LOGI("  Signal: %d dBm", WiFi.RSSI());
        LOGI("  MAC: %s", WiFi.macAddress().c_str());
    }
    
    LOGI("  Reconnect attempts: %d", reconnectAttempts);
}

String WiFiComm::getIPAddress() {
    return WiFi.localIP().toString();
}

String WiFiComm::getMACAddress() {
    return WiFi.macAddress();
}

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

void WiFiComm::webSocketEventStatic(WStype_t type, uint8_t* payload, size_t length) {
    if (instance) {
        instance->webSocketEvent(type, payload, length);
    }
}

void WiFiComm::webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            LOGW("WebSocket disconnected");
            status = CONN_DISCONNECTED;
            break;
            
        case WStype_CONNECTED:
            LOGI("WebSocket connected to: %s", payload);
            status = CONN_CONNECTED;
            
            // Send initial handshake
            sendStatus("READY", "Robot connected and ready");
            break;
            
        case WStype_TEXT: {
            LOGD("WebSocket message received: %s", payload);
            
            // Parse and handle command
            CommandMessage cmd;
            if (parseCommand((char*)payload, cmd)) {
                lastCommand = cmd;
                if (commandCallback) {
                    commandCallback(cmd);
                }
            }
            break;
        }
            
        case WStype_ERROR:
            LOGE("WebSocket error");
            status = CONN_ERROR;
            break;
            
        case WStype_PING:
            LOGV("WebSocket ping");
            break;
            
        case WStype_PONG:
            LOGV("WebSocket pong");
            break;
            
        default:
            break;
    }
}

bool WiFiComm::parseCommand(const char* json, CommandMessage& cmd) {
    DeserializationError error = deserializeJson(jsonDoc, json);
    
    if (error) {
        LOGE("JSON parse error: %s", error.c_str());
        return false;
    }
    
    cmd.type = jsonDoc["type"].as<String>();
    cmd.param1 = jsonDoc["param1"] | 0.0f;
    cmd.param2 = jsonDoc["param2"] | 0.0f;
    cmd.timestamp = jsonDoc["timestamp"] | 0UL;
    cmd.valid = true;
    
    LOGI("Command received: %s (%.1f, %.1f)", 
         cmd.type.c_str(), cmd.param1, cmd.param2);
    
    return true;
}
