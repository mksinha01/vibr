#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "logger.h"

// ============================================================================
// INDUSTRY STANDARD STATE MACHINE
// Implements finite state machine with proper transitions and error handling
// ============================================================================

enum RobotState {
    STATE_INIT,              // System initialization
    STATE_IDLE,              // Waiting for commands
    STATE_CONNECTING,        // Connecting to server
    STATE_READY,             // Connected and ready
    STATE_NAVIGATING,        // Following path
    STATE_OBSTACLE_AVOID,    // Avoiding obstacles
    STATE_TURNING,           // Executing turn
    STATE_PAUSED,            // Temporarily paused
    STATE_RETURNING_HOME,    // Returning to base
    STATE_EMERGENCY_STOP,    // Emergency stop activated
    STATE_ERROR,             // Error condition
    STATE_RECOVERY,          // Attempting recovery
    STATE_MAINTENANCE        // Maintenance mode
};

enum StateEvent {
    EVENT_NONE,
    EVENT_INIT_COMPLETE,
    EVENT_WIFI_CONNECTED,
    EVENT_WIFI_DISCONNECTED,
    EVENT_COMMAND_RECEIVED,
    EVENT_PATH_COMPLETE,
    EVENT_OBSTACLE_DETECTED,
    EVENT_OBSTACLE_CLEARED,
    EVENT_TURN_COMPLETE,
    EVENT_EMERGENCY_STOP,
    EVENT_LOW_BATTERY,
    EVENT_ERROR_OCCURRED,
    EVENT_RECOVERY_SUCCESS,
    EVENT_TIMEOUT,
    EVENT_USER_PAUSE,
    EVENT_USER_RESUME
};

class StateMachine {
private:
    RobotState currentState;
    RobotState previousState;
    unsigned long stateStartTime;
    unsigned long lastTransitionTime;
    
    // State transition table
    struct StateTransition {
        RobotState fromState;
        StateEvent event;
        RobotState toState;
        bool (*guard)();  // Optional guard condition
    };
    
    static StateTransition transitions[];
    static const int transitionCount;
    
    // State entry/exit handlers
    void onStateEntry(RobotState state);
    void onStateExit(RobotState state);
    
    // State update handlers
    void updateInit();
    void updateIdle();
    void updateConnecting();
    void updateReady();
    void updateNavigating();
    void updateObstacleAvoid();
    void updateTurning();
    void updatePaused();
    void updateReturningHome();
    void updateEmergencyStop();
    void updateError();
    void updateRecovery();
    void updateMaintenance();
    
    // Validation
    bool isValidTransition(RobotState from, StateEvent event);
    
    // Metrics
    unsigned long stateTimeMs[20];  // Time spent in each state
    unsigned int stateEnterCount[20];  // Entry count for each state
    
public:
    StateMachine();
    
    void init();
    void update();
    
    // State control
    bool processEvent(StateEvent event);
    void forceState(RobotState state);  // Emergency state override
    
    // Getters
    RobotState getCurrentState() const { return currentState; }
    RobotState getPreviousState() const { return previousState; }
    const char* getStateName(RobotState state) const;
    const char* getEventName(StateEvent event) const;
    unsigned long getTimeInCurrentState() const;
    
    // State queries
    bool isInState(RobotState state) const { return currentState == state; }
    bool canNavigate() const;
    bool isEmergency() const;
    bool needsRecovery() const;
    
    // Metrics
    void printMetrics();
    void resetMetrics();
};

#endif // STATE_MACHINE_H
