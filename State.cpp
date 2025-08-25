#include "State.h"

static State currentState;

const State &getState() {
    return currentState;
}

void stateSetHardwareFlag(HardwareFlag flag, bool value) {
    if (value) {
        currentState.hardwareFlags |= flag;
    } else {
        currentState.hardwareFlags &= ~flag;
    }
}

void stateSetFlightStatus(FlightStatus status) {
    currentState.flightStatus = status;
}

void stateUpdateOrientation(float pitchRadians, float rollRadians, float yawRadians, bool ok) {
    currentState.pitchRadians = pitchRadians;
    currentState.rollRadians = rollRadians;
    currentState.yawRadians = yawRadians;
    stateSetHardwareFlag(HF_MPU_OK, ok);
}

void stateUpdateRC(float pitch, float roll, float yaw, float throttle, bool ok) {
    currentState.rcPitchRadians = pitch;
    currentState.rcRollRadians = roll;
    currentState.rcYaw = yaw;
    currentState.rcThrottle = throttle;
    stateSetHardwareFlag(HF_RC_OK, ok);
}

void stateUpdateControlErrors(float pitchErrorRadians, float rollErrorRadians) {
    currentState.pitchErrorRadians = pitchErrorRadians;
    currentState.rollErrorRadians = rollErrorRadians;
}

void stateUpdateMotorCommands(float motor1, float motor2, float motor3, float motor4, float motor5, float motor6) {
    currentState.motor1Command = motor1;
    currentState.motor2Command = motor2;
    currentState.motor3Command = motor3;
    currentState.motor4Command = motor4;
    currentState.motor5Command = motor5;
    currentState.motor6Command = motor6;
}
