#include "StateMachine.h"
#include "State.h"
#include "RadioController.h"

class DisarmedState : public StateMachine {
protected:
    void beginState() override {
        stateSetFlightStatus(FS_Disarmed);
    }
    void updateState() override;
public:
    DisarmedState() : StateMachine("Disarmed") {}
};

class ArmingState : public StateMachine {
    unsigned long startMillis;
protected:
    void beginState() override {
        stateSetFlightStatus(FS_Arming);
        startMillis = millis();
    }
    void updateState() override;
public:
    ArmingState() : StateMachine("Arming"), startMillis(0) {}
};

class WaitingForNoInputState : public StateMachine {
    unsigned long startMillis;
protected:
    void beginState() override {
        stateSetFlightStatus(FS_WaitingForNoInput);
        startMillis = millis();
    }
    void updateState() override;
public:
    WaitingForNoInputState() : StateMachine("WaitingForNoInput"), startMillis(0) {}
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

void DisarmedState::updateState() {
    const auto a = rcIsArming();
    if (a) {
        transitionState(new ArmingState());
    }
}

void ArmingState::updateState() {
    const auto arming = rcIsArming();
    if (arming) {
        if (millis() - startMillis > 3000) { // 3 seconds debounce
            transitionState(new WaitingForNoInputState());
        }
    }
    else {
        transitionState(new DisarmedState());
    }
}

void WaitingForNoInputState::updateState() {
    const auto noInput = rcIsNoInput();
    if (noInput) {
        transitionState(new FlyingState());
    }
    else {
        if (millis() - startMillis > 3000) {
            transitionState(new DisarmedState());
        }
    }
}

FlightState flightState;

void FlightState::beginState() {
    transitionSubState(new DisarmedState());
}

void FlightState::updateState() {
    // Everything handled by sub states
}
