#pragma once

struct State {
    float pitchRadians;
    float rollRadians;
    float yawRadians;

    float rcPitchRadians;
    float rcRollRadians;
    float rcYaw;
    float rcThrottle;
    bool rcOK;
    
    bool armed;

    float motor1Command;
    float motor2Command;
    float motor3Command;
    float motor4Command;
    float motor5Command;
    float motor6Command;

    State()
        : pitchRadians(0.0f), rollRadians(0.0f), yawRadians(0.0f)
        , rcPitchRadians(0.0f), rcRollRadians(0.0f), rcYaw(0.0f)
        , rcThrottle(0.0f)
        , rcOK(false)
        , armed(false)
        , motor1Command(0.0f), motor2Command(0.0f), motor3Command(0.0f)
        , motor4Command(0.0f), motor5Command(0.0f), motor6Command(0.0f)
    {}
};

const State &getState();

void stateUpdateOrientation(float pitchRadians, float rollRadians, float yawRadians);
void stateUpdateRC(float pitch, float roll, float yaw, float throttle, bool ok);
void stateUpdateMotorCommands(float motor1, float motor2, float motor3, float motor4, float motor5, float motor6);
