#pragma once

#include <cstring>

#define MAX_MOTORS 6

struct MixValues {
    float thrust;
    float pitch;
    float roll;
    float yaw;

    MixValues() : thrust(0), pitch(0), roll(0), yaw(0) {}
};

class MotorMixer {
private:
    size_t numMotors;
    MixValues mixerMatrix[MAX_MOTORS];
    bool reversed[MAX_MOTORS];
    float outputs[MAX_MOTORS];
public:
    MotorMixer()
        : numMotors(0) {
        std::memset(mixerMatrix, 0, sizeof(mixerMatrix));
        std::memset(outputs, 0, sizeof(outputs));
        std::memset(reversed, 0, sizeof(reversed));
    }
    size_t getNumMotors() const {
        return numMotors;
    }
    float getMotorCommand(size_t motorIndex) const {
        if (motorIndex < numMotors) {
            return outputs[motorIndex];
        }
        return 0.0f;
    }
    void updateMotorMix();
    void mix(const MixValues& values);
};
