#include "state_machine.h"

static const char* TAG = "StateMachine";

// State transition table - Industry standard FSM design
StateMachine::StateTransition StateMachine::transitions[] = {
    // From INIT
    {STATE_INIT, EVENT_INIT_COMPLETE, STATE_CONNECTING, nullptr},
    {STATE_INIT, EVENT_ERROR_OCCURRED, STATE_ERROR, nullptr},
    
    // From CONNECTING
    {STATE_CONNECTING, EVENT_WIFI_CONNECTED, STATE_READY, nullptr},
    {STATE_CONNECTING, EVENT_TIMEOUT, STATE_ERROR, nullptr},
    {STATE_CONNECTING, EVENT_EMERGENCY_STOP, STATE_EMERGENCY_STOP, nullptr},
    
    // From IDLE/READY
    {STATE_READY, EVENT_COMMAND_RECEIVED, STATE_NAVIGATING, nullptr},
    {STATE_READY, EVENT_WIFI_DISCONNECTED, STATE_CONNECTING, nullptr},
    {STATE_READY, EVENT_EMERGENCY_STOP, STATE_EMERGENCY_STOP, nullptr},
    {STATE_READY, EVENT_LOW_BATTERY, STATE_RETURNING_HOME, nullptr},
    
    // From NAVIGATING
    {STATE_NAVIGATING, EVENT_OBSTACLE_DETECTED, STATE_OBSTACLE_AVOID, nullptr},
    {STATE_NAVIGATING, EVENT_PATH_COMPLETE, STATE_READY, nullptr},
    {STATE_NAVIGATING, EVENT_EMERGENCY_STOP, STATE_EMERGENCY_STOP, nullptr},
    {STATE_NAVIGATING, EVENT_USER_PAUSE, STATE_PAUSED, nullptr},
    {STATE_NAVIGATING, EVENT_LOW_BATTERY, STATE_RETURNING_HOME, nullptr},
    {STATE_NAVIGATING, EVENT_ERROR_OCCURRED, STATE_ERROR, nullptr},
    
    // From OBSTACLE_AVOID
    {STATE_OBSTACLE_AVOID, EVENT_OBSTACLE_CLEARED, STATE_NAVIGATING, nullptr},
    {STATE_OBSTACLE_AVOID, EVENT_EMERGENCY_STOP, STATE_EMERGENCY_STOP, nullptr},
    {STATE_OBSTACLE_AVOID, EVENT_TIMEOUT, STATE_ERROR, nullptr},
    
    // From PAUSED
    {STATE_PAUSED, EVENT_USER_RESUME, STATE_NAVIGATING, nullptr},
    {STATE_PAUSED, EVENT_EMERGENCY_STOP, STATE_EMERGENCY_STOP, nullptr},
    {STATE_PAUSED, EVENT_TIMEOUT, STATE_READY, nullptr},
    
    // From RETURNING_HOME
    {STATE_RETURNING_HOME, EVENT_PATH_COMPLETE, STATE_IDLE, nullptr},
    {STATE_RETURNING_HOME, EVENT_EMERGENCY_STOP, STATE_EMERGENCY_STOP, nullptr},
    
    // From EMERGENCY_STOP (can only recover)
    {STATE_EMERGENCY_STOP, EVENT_USER_RESUME, STATE_RECOVERY, nullptr},
    
    // From ERROR
    {STATE_ERROR, EVENT_NONE, STATE_RECOVERY, nullptr},
    
    // From RECOVERY
    {STATE_RECOVERY, EVENT_RECOVERY_SUCCESS, STATE_READY, nullptr},
    {STATE_RECOVERY, EVENT_ERROR_OCCURRED, STATE_ERROR, nullptr},
};

const int StateMachine::transitionCount = sizeof(transitions) / sizeof(StateTransition);

StateMachine::StateMachine() 
    : currentState(STATE_INIT)
    , previousState(STATE_INIT)
    , stateStartTime(0)
    , lastTransitionTime(0) {
    memset(stateTimeMs, 0, sizeof(stateTimeMs));
    memset(stateEnterCount, 0, sizeof(stateEnterCount));
}

void StateMachine::init() {
    LOGI("State machine initialized");
    currentState = STATE_INIT;
    stateStartTime = millis();
    lastTransitionTime = millis();
    onStateEntry(STATE_INIT);
}

void StateMachine::update() {
    // Update current state logic
    switch(currentState) {
        case STATE_INIT:            updateInit(); break;
        case STATE_IDLE:            updateIdle(); break;
        case STATE_CONNECTING:      updateConnecting(); break;
        case STATE_READY:           updateReady(); break;
        case STATE_NAVIGATING:      updateNavigating(); break;
        case STATE_OBSTACLE_AVOID:  updateObstacleAvoid(); break;
        case STATE_TURNING:         updateTurning(); break;
        case STATE_PAUSED:          updatePaused(); break;
        case STATE_RETURNING_HOME:  updateReturningHome(); break;
        case STATE_EMERGENCY_STOP:  updateEmergencyStop(); break;
        case STATE_ERROR:           updateError(); break;
        case STATE_RECOVERY:        updateRecovery(); break;
        case STATE_MAINTENANCE:     updateMaintenance(); break;
    }
    
    // Update metrics
    unsigned long currentTime = millis();
    if (currentTime >= stateStartTime) {
        stateTimeMs[currentState] += currentTime - stateStartTime;
        stateStartTime = currentTime;
    }
}

bool StateMachine::processEvent(StateEvent event) {
    if (event == EVENT_NONE) return false;
    
    LOGD("Processing event: %s in state: %s", 
         getEventName(event), getStateName(currentState));
    
    // Find valid transition
    for (int i = 0; i < transitionCount; i++) {
        if (transitions[i].fromState == currentState && 
            transitions[i].event == event) {
            
            // Check guard condition if exists
            if (transitions[i].guard != nullptr && !transitions[i].guard()) {
                LOGW("Transition guard failed for event: %s", getEventName(event));
                return false;
            }
            
            // Valid transition found
            RobotState newState = transitions[i].toState;
            LOGI("State transition: %s -> %s (event: %s)",
                 getStateName(currentState), 
                 getStateName(newState),
                 getEventName(event));
            
            // Execute transition
            onStateExit(currentState);
            previousState = currentState;
            currentState = newState;
            stateStartTime = millis();
            lastTransitionTime = millis();
            stateEnterCount[currentState]++;
            onStateEntry(currentState);
            
            return true;
        }
    }
    
    LOGW("No valid transition for event: %s in state: %s", 
         getEventName(event), getStateName(currentState));
    return false;
}

void StateMachine::forceState(RobotState state) {
    LOGW("FORCED state change: %s -> %s", 
         getStateName(currentState), getStateName(state));
    onStateExit(currentState);
    previousState = currentState;
    currentState = state;
    stateStartTime = millis();
    onStateEntry(currentState);
}

// ============================================================================
// STATE ENTRY/EXIT HANDLERS
// ============================================================================

void StateMachine::onStateEntry(RobotState state) {
    LOGI(">>> Entering state: %s", getStateName(state));
    
    switch(state) {
        case STATE_INIT:
            // Initialize all subsystems
            break;
        case STATE_EMERGENCY_STOP:
            // Stop all motors immediately
            LOGE("EMERGENCY STOP ACTIVATED!");
            break;
        case STATE_ERROR:
            // Log error details
            LOGE("Entered ERROR state from %s", getStateName(previousState));
            break;
        default:
            break;
    }
}

void StateMachine::onStateExit(RobotState state) {
    LOGI("<<< Exiting state: %s", getStateName(state));
}

// ============================================================================
// STATE UPDATE FUNCTIONS
// ============================================================================

void StateMachine::updateInit() {
    // Handled by main initialization
}

void StateMachine::updateIdle() {
    // Low power mode, waiting for commands
}

void StateMachine::updateConnecting() {
    // WiFi connection handled by communications module
}

void StateMachine::updateReady() {
    // Ready for commands
}

void StateMachine::updateNavigating() {
    // Navigation logic handled by navigation module
}

void StateMachine::updateObstacleAvoid() {
    // Obstacle avoidance logic
}

void StateMachine::updateTurning() {
    // Turn execution
}

void StateMachine::updatePaused() {
    // Maintain position
}

void StateMachine::updateReturningHome() {
    // Return to base station
}

void StateMachine::updateEmergencyStop() {
    // All systems stopped, waiting for reset
}

void StateMachine::updateError() {
    // Error condition, attempt auto-recovery
    if (getTimeInCurrentState() > 5000) {
        processEvent(EVENT_NONE);  // Trigger recovery
    }
}

void StateMachine::updateRecovery() {
    // Recovery procedures
}

void StateMachine::updateMaintenance() {
    // Maintenance mode
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

const char* StateMachine::getStateName(RobotState state) const {
    switch(state) {
        case STATE_INIT:            return "INIT";
        case STATE_IDLE:            return "IDLE";
        case STATE_CONNECTING:      return "CONNECTING";
        case STATE_READY:           return "READY";
        case STATE_NAVIGATING:      return "NAVIGATING";
        case STATE_OBSTACLE_AVOID:  return "OBSTACLE_AVOID";
        case STATE_TURNING:         return "TURNING";
        case STATE_PAUSED:          return "PAUSED";
        case STATE_RETURNING_HOME:  return "RETURNING_HOME";
        case STATE_EMERGENCY_STOP:  return "EMERGENCY_STOP";
        case STATE_ERROR:           return "ERROR";
        case STATE_RECOVERY:        return "RECOVERY";
        case STATE_MAINTENANCE:     return "MAINTENANCE";
        default:                    return "UNKNOWN";
    }
}

const char* StateMachine::getEventName(StateEvent event) const {
    switch(event) {
        case EVENT_NONE:                return "NONE";
        case EVENT_INIT_COMPLETE:       return "INIT_COMPLETE";
        case EVENT_WIFI_CONNECTED:      return "WIFI_CONNECTED";
        case EVENT_WIFI_DISCONNECTED:   return "WIFI_DISCONNECTED";
        case EVENT_COMMAND_RECEIVED:    return "COMMAND_RECEIVED";
        case EVENT_PATH_COMPLETE:       return "PATH_COMPLETE";
        case EVENT_OBSTACLE_DETECTED:   return "OBSTACLE_DETECTED";
        case EVENT_OBSTACLE_CLEARED:    return "OBSTACLE_CLEARED";
        case EVENT_TURN_COMPLETE:       return "TURN_COMPLETE";
        case EVENT_EMERGENCY_STOP:      return "EMERGENCY_STOP";
        case EVENT_LOW_BATTERY:         return "LOW_BATTERY";
        case EVENT_ERROR_OCCURRED:      return "ERROR_OCCURRED";
        case EVENT_RECOVERY_SUCCESS:    return "RECOVERY_SUCCESS";
        case EVENT_TIMEOUT:             return "TIMEOUT";
        case EVENT_USER_PAUSE:          return "USER_PAUSE";
        case EVENT_USER_RESUME:         return "USER_RESUME";
        default:                        return "UNKNOWN_EVENT";
    }
}

unsigned long StateMachine::getTimeInCurrentState() const {
    return millis() - stateStartTime;
}

bool StateMachine::canNavigate() const {
    return currentState == STATE_NAVIGATING || 
           currentState == STATE_READY ||
           currentState == STATE_OBSTACLE_AVOID;
}

bool StateMachine::isEmergency() const {
    return currentState == STATE_EMERGENCY_STOP;
}

bool StateMachine::needsRecovery() const {
    return currentState == STATE_ERROR || 
           currentState == STATE_RECOVERY;
}

void StateMachine::printMetrics() {
    LOGI("=== State Machine Metrics ===");
    for (int i = 0; i < 13; i++) {
        if (stateEnterCount[i] > 0) {
            LOGI("State %s: Entered %d times, Total time: %lu ms",
                 getStateName((RobotState)i),
                 stateEnterCount[i],
                 stateTimeMs[i]);
        }
    }
}

void StateMachine::resetMetrics() {
    memset(stateTimeMs, 0, sizeof(stateTimeMs));
    memset(stateEnterCount, 0, sizeof(stateEnterCount));
    LOGI("Metrics reset");
}
