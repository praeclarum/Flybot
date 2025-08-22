// MotorControl.cpp
// Mixer algorithm for mapping control torques and thrust to per-motor outputs
// Private implementation, not exposed via header

#include <cmath>
#include <algorithm>

#include "Motors.h"
#include "Config.h" // For MotorConfig

void MotorMixer::updateMotorMix() {
    const auto motors = airframeConfig.getMotorConfigs();
    numMotors = motors.size();
    // Build mixer matrix
    float maxX = 0, maxY = 0;
    for (const auto m : motors) {
        if (std::abs(m->x.getFloat()) > maxX) maxX = std::abs(m->x.getFloat());
        if (std::abs(m->y.getFloat()) > maxY) maxY = std::abs(m->y.getFloat());
    }
    if (maxX < 1.0f) maxX = 1.0f; // Prevent division by zero
    if (maxY < 1.0f) maxY = 1.0f; // Prevent division by zero
    for (size_t i = 0; i < numMotors; ++i) {
        reversed[i] = motors[i]->direction.getInt() < 0;
        const float dir = reversed[i] ? -1.0f : 1.0f;
        mixerMatrix[i].thrust = dir; // All motors contribute equally to thrust
        mixerMatrix[i].pitch  = motors[i]->y.getFloat() / maxY * dir; // Pitch: y offset (rotation about X)
        mixerMatrix[i].roll   = -motors[i]->x.getFloat() / maxX * dir; // Roll: x offset (rotation about Y)
        mixerMatrix[i].yaw    = dir; // Yaw: motor direction
    }
}

// Inputs: ControlInput (thrust, pitch, roll, yaw) in normalized units
// Output: per-motor PWM in [-1, +1]
void MotorMixer::mix(const MixValues& mixValues) {
    float maxOut = 0.0f;
    for (size_t i = 0; i < numMotors; ++i) {
        float newOutput = 0.0f;
        newOutput += mixerMatrix[i].thrust * mixValues.thrust;
        newOutput += mixerMatrix[i].pitch  * mixValues.pitch;
        newOutput += mixerMatrix[i].roll   * mixValues.roll;
        newOutput += mixerMatrix[i].yaw    * mixValues.yaw;
        if (reversed[i]) {
            if (newOutput > 0.0f) newOutput = 0.0f;
            if (-newOutput > maxOut) maxOut = -newOutput;
        }
        else {
            if (newOutput < 0.0f) newOutput = 0.0f;
            if (newOutput > maxOut) maxOut = newOutput;
        }
        outputs[i] = newOutput;
    }
    if (maxOut > 1.0f) {
        const float invMax = 1.0f / maxOut;
        // Normalize outputs to [-1, +1]
        for (size_t i = 0; i < numMotors; ++i) {
            outputs[i] *= invMax;
        }
    }
}
