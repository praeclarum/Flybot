#pragma once

#include <cstdint>

enum StatusFlag {
    SF_Armed    = 0x00000001,
    SF_MPU_OK   = 0x00000002,
    SF_RC_OK    = 0x00000004,
};

struct State {
    float pitchRadians;
    float rollRadians;
    float yawRadians;

    float rcPitchRadians;
    float rcRollRadians;
    float rcYaw;
    float rcThrottle;
    
    float pitchErrorRadians;
    float rollErrorRadians;

    float motor1Command;
    float motor2Command;
    float motor3Command;
    float motor4Command;
    float motor5Command;
    float motor6Command;

    std::uint32_t statusFlags;

    State()
        : pitchRadians(0.0f), rollRadians(0.0f), yawRadians(0.0f)
        , rcPitchRadians(0.0f), rcRollRadians(0.0f), rcYaw(0.0f)
        , rcThrottle(0.0f)
        , pitchErrorRadians(0.0f), rollErrorRadians(0.0f)
        , motor1Command(0.0f), motor2Command(0.0f), motor3Command(0.0f)
        , motor4Command(0.0f), motor5Command(0.0f), motor6Command(0.0f)
        , statusFlags(0)
    {}

    bool hasStatusFlag(StatusFlag flag) const {
        return (statusFlags & flag) != 0;
    }
};

const State &getState();

void stateUpdateOrientation(float pitchRadians, float rollRadians, float yawRadians, bool ok);
void stateUpdateRC(float pitch, float roll, float yaw, float throttle, bool ok);
void stateUpdateControlErrors(float pitchErrorRadians, float rollErrorRadians);
void stateUpdateMotorCommands(float motor1, float motor2, float motor3, float motor4, float motor5, float motor6);
void stateSetStatusFlag(StatusFlag flag, bool value);
