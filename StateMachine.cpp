#include "StateMachine.h"
#include "State.h"

class DisarmedState : public StateMachine {
protected:
    void beginState() override {
        // stateClearStatusFlag(SF_Armed);
    }
    void updateState() override {
        // DisarmedState specific update logic
    }
public:
    DisarmedState() : StateMachine("Disarmed") {}
};

class FlyingState : public StateMachine {
protected:
    void beginState() override {
        // stateSetStatusFlag(SF_Armed);
    }
    void updateState() override {
        // DisarmedState specific update logic
    }
public:
    FlyingState() : StateMachine("Flying") {}
};

FlightState flightState;

void FlightState::beginState() {
    transitionSubState(new DisarmedState());
}

void FlightState::updateState() {
    // Everything handled by sub states
}
