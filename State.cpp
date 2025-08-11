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

void stateUpdateRC(float pitch, float roll, float yaw, float throttle, bool ok) {
    currentState.rcPitch = pitch;
    currentState.rcRoll = roll;
    currentState.rcYaw = yaw;
    currentState.rcThrottle = throttle;
    currentState.rcOK = ok;
}
