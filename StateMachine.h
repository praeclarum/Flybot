#pragma once

#include <Arduino.h>
#include <esp_log.h>

class StateMachine {
    String name;
    StateMachine *subState;
    StateMachine *nextState;
protected:
    virtual void beginState() = 0;
    virtual void updateState() = 0;

    void transitionState(StateMachine *newState) {
        delete nextState;
        nextState = newState;
    }
    void transitionSubState(StateMachine *newSubState) {
        ESP_LOGI("StateMachine", "%s transitioning from %s to %s", name.c_str(), subState ? subState->name.c_str() : "None", newSubState ? newSubState->name.c_str() : "None");
        delete subState;
        subState = newSubState;
        if (subState) {
            subState->beginState();
        }
    }
public:
    StateMachine(const String &name) : name(name), subState(nullptr), nextState(nullptr) {}
    virtual ~StateMachine() {
        delete subState;
        delete nextState;
    }
    void update() {
        if (subState) {
            subState->update();
            StateMachine *nextSubState = subState->nextState;
            subState->nextState = nullptr;
            if (nextSubState) {
                transitionSubState(nextSubState);
            }
        }
        else {
            updateState();
        }
    }
};

class FlightState : public StateMachine {
protected:
    void beginState();
    void updateState();
public:
    FlightState() : StateMachine("FlightState") {}
    ~FlightState() {}
};

extern FlightState flightState;
