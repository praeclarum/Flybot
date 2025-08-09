#include "State.h"

static State currentState;

const State &getState() {
    return currentState;
}

void stateUpdateOrientation(float pitchRadians, float rollRadians, float yawRadians) {
    currentState.pitchRadians = pitchRadians;
    currentState.rollRadians = rollRadians;
    currentState.yawRadians = yawRadians;
}
