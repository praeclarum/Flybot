#include "StateMachine.h"
#include "State.h"

class DisarmedState : public StateMachine {
protected:
    void beginState() override {
        stateSetFlightStatus(FS_Disarmed);
    }
    void updateState() override {
    }
public:
    DisarmedState() : StateMachine("Disarmed") {}
};

class FlyingState : public StateMachine {
protected:
    void beginState() override {
        stateSetFlightStatus(FS_Flying);
    }
    void updateState() override {
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
