// MotorControl.cpp
// Mixer algorithm for mapping control torques and thrust to per-motor outputs
// Private implementation, not exposed via header

#include <vector>
#include <cmath>

#include "Config.h" // For MotorConfig

using namespace std;

struct ControlInput {
    float thrust;
    float pitch;
    float roll;
    float yaw;
};

class MotorMixer {
private:
    size_t numMotors;
    vector<ControlInput> mixerMatrix;
public:
    MotorMixer()
        : numMotors(config.numMotors.getInt()), mixerMatrix(numMotors, ControlInput{}) {
        // Build mixer matrix
        float maxX = 0, maxY = 0;
        const auto motors = config.getMotorConfigs();
        for (const auto m : motors) {
            if (abs(m->x.getFloat()) > maxX) maxX = abs(m->x.getFloat());
            if (abs(m->y.getFloat()) > maxY) maxY = abs(m->y.getFloat());
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
    vector<float> mix(const ControlInput& control) const {
        vector<float> outputs(numMotors, 0.0f);
        for (size_t i = 0; i < numMotors; ++i) {
            outputs[i] += mixerMatrix[i].thrust * control.thrust;
            outputs[i] += mixerMatrix[i].pitch  * control.pitch;
            outputs[i] += mixerMatrix[i].roll   * control.roll;
            outputs[i] += mixerMatrix[i].yaw    * control.yaw;
            // Clamp output to [-1, +1]
            if (outputs[i] > 1.0f) outputs[i] = 1.0f;
            if (outputs[i] < -1.0f) outputs[i] = -1.0f;
        }
        return outputs;
    }
};
