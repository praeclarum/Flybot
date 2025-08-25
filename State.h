#pragma once

#include <cstdint>

enum FlightStatus {
    FS_Disarmed             = 0,
    FS_Arming               = 1,
    FS_WaitingForNoInput    = 2,
    FS_Flying               = 3,
};

enum HardwareFlag {
    HF_MPU_OK   = 0x00000001,
    HF_RC_OK    = 0x00000002,
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

    FlightStatus flightStatus;
    std::uint32_t hardwareFlags;

    State()
        : pitchRadians(0.0f), rollRadians(0.0f), yawRadians(0.0f)
        , rcPitchRadians(0.0f), rcRollRadians(0.0f), rcYaw(0.0f)
        , rcThrottle(0.0f)
        , pitchErrorRadians(0.0f), rollErrorRadians(0.0f)
        , motor1Command(0.0f), motor2Command(0.0f), motor3Command(0.0f)
        , motor4Command(0.0f), motor5Command(0.0f), motor6Command(0.0f)
        , flightStatus(FS_Disarmed)
        , hardwareFlags(0)
    {}

    bool hasHardwareFlag(HardwareFlag flag) const {
        return (hardwareFlags & static_cast<std::uint32_t>(flag)) != 0;
    }
    float rcPitchDegrees() const {
        return rcPitchRadians * (180.0f / 3.14159265359f);
    }
    float rcRollDegrees() const {
        return rcRollRadians * (180.0f / 3.14159265359f);
    }
};

const State &getState();

void stateUpdateOrientation(float pitchRadians, float rollRadians, float yawRadians, bool ok);
void stateUpdateRC(float pitch, float roll, float yaw, float throttle, bool ok);
void stateUpdateControlErrors(float pitchErrorRadians, float rollErrorRadians);
void stateUpdateMotorCommands(float motor1, float motor2, float motor3, float motor4, float motor5, float motor6);
void stateSetHardwareFlag(HardwareFlag flag, bool value);
void stateSetFlightStatus(FlightStatus status);
