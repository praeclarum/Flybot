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

class ArmingWaitingForNoInputState : public StateMachine {
    unsigned long startMillis;
protected:
    void beginState() override {
        stateSetFlightStatus(FS_ArmingWaitingForNoInput);
        startMillis = millis();
    }
    void updateState() override;
public:
    ArmingWaitingForNoInputState() : StateMachine("ArmingWaitingForNoInput"), startMillis(0) {}
};

class FlyingState : public StateMachine {
protected:
    void beginState() override {
        stateSetFlightStatus(FS_Flying);
    }
    void updateState() override;
public:
    FlyingState() : StateMachine("Flying") {}
};

class DisarmingState : public StateMachine {
    unsigned long startMillis;
protected:
    void beginState() override {
        stateSetFlightStatus(FS_Disarming);
        startMillis = millis();
    }
    void updateState() override;
public:
    DisarmingState() : StateMachine("Disarming") {}
};

class DisarmingWaitingForNoInputState : public StateMachine {
    unsigned long startMillis;
protected:
    void beginState() override {
        stateSetFlightStatus(FS_DisarmingWaitingForNoInput);
        startMillis = millis();
    }
    void updateState() override;
public:
    DisarmingWaitingForNoInputState() : StateMachine("DisarmingWaitingForNoInput"), startMillis(0) {}
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
        if (millis() - startMillis > 2000) { // 2 seconds debounce
            transitionState(new ArmingWaitingForNoInputState());
        }
    }
    else {
        transitionState(new DisarmedState());
    }
}

void ArmingWaitingForNoInputState::updateState() {
    const auto noInput = rcIsNoInput();
    if (noInput) {
        transitionState(new FlyingState());
    }
}

void FlyingState::updateState() {
    const auto arming = rcIsArming();
    if (arming) {
        transitionState(new DisarmingState());
    }
}

void DisarmingState::updateState() {
    const auto arming = rcIsArming();
    if (arming) {
        if (millis() - startMillis > 2000) { // 2 seconds debounce
            transitionState(new DisarmingWaitingForNoInputState());
        }
    }
    else {
        transitionState(new FlyingState());
    }
}

void DisarmingWaitingForNoInputState::updateState() {
    const auto noInput = rcIsNoInput();
    if (noInput) {
        transitionState(new DisarmedState());
    }
}

FlightState flightState;

void FlightState::beginState() {
    transitionSubState(new DisarmedState());
}

void FlightState::updateState() {
    // Everything handled by sub states
}
