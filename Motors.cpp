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
    for (size_t i = 0; i < numMotors; ++i) {
        // [thrust, pitch, roll, yaw]
        mixerMatrix[i].thrust = 1.0f; // All motors contribute equally to thrust
        mixerMatrix[i].pitch  = motors[i]->y.getFloat() / (maxY ? maxY : 1); // Pitch: y offset (rotation about X)
        mixerMatrix[i].roll   = motors[i]->x.getFloat() / (maxX ? maxX : 1); // Roll: x offset (rotation about Y)
        mixerMatrix[i].yaw    = static_cast<float>(motors[i]->direction.getInt()); // Yaw: CW/CCW
    }
}

// Inputs: ControlInput (thrust, pitch, roll, yaw) in normalized units
// Output: per-motor PWM in [-1, +1]
void MotorMixer::mix(const MixValues& mixValues) {
    for (size_t i = 0; i < numMotors; ++i) {
        outputs[i]  = mixerMatrix[i].thrust * mixValues.thrust;
        outputs[i] += mixerMatrix[i].pitch  * mixValues.pitch;
        outputs[i] += mixerMatrix[i].roll   * mixValues.roll;
        outputs[i] += mixerMatrix[i].yaw    * mixValues.yaw;
        // Clamp output to [-1, +1]
        if (outputs[i] > 1.0f) outputs[i] = 1.0f;
        if (outputs[i] < -1.0f) outputs[i] = -1.0f;
    }
}

