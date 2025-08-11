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

    State()
        : pitchRadians(0.0f), rollRadians(0.0f), yawRadians(0.0f)
        , rcPitchRadians(0.0f), rcRollRadians(0.0f), rcYaw(0.0f)
        , rcThrottle(0.0f)
        , rcOK(false)
        , armed(false)
    {}
};

const State &getState();

void stateUpdateOrientation(float pitchRadians, float rollRadians, float yawRadians);
void stateUpdateRC(float pitch, float roll, float yaw, float throttle, bool ok);
