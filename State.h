#pragma once

struct State {
    float rollRadians;
    float pitchRadians;
    float yawRadians;
    float throttle;
    bool armed;

    State()
        : rollRadians(0.0f), pitchRadians(0.0f), yawRadians(0.0f),
          throttle(0.0f), armed(false) {}
};

const State &getState();

void stateUpdateOrientation(float pitchRadians, float rollRadians, float yawRadians);
