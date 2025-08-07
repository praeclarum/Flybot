// MotorControl.cpp
// Mixer algorithm for mapping control torques and thrust to per-motor outputs
// Private implementation, not exposed via header

#include <vector>
#include <cmath>

using namespace std;

struct ControlInput {
    float thrust;
    float pitch;
    float roll;
    float yaw;
};

struct MotorConfig {
    float x;      // X position relative to COM
    float y;      // Y position relative to COM
    int direction; // +1 for CW, -1 for CCW
};

class MotorMixer {
private:
    size_t numMotors;
    vector<ControlInput> mixerMatrix;
public:
    MotorMixer(const vector<MotorConfig>& motors)
        : numMotors(motors.size()), mixerMatrix(numMotors, ControlInput{}) {
        assert(numMotors > 0);
        // Build mixer matrix
        float maxX = 0, maxY = 0;
        for (const auto& m : motors) {
            if (std::abs(m.x) > maxX) maxX = std::abs(m.x);
            if (std::abs(m.y) > maxY) maxY = std::abs(m.y);
        }
        for (size_t i = 0; i < numMotors; ++i) {
            // [thrust, pitch, roll, yaw]
            mixerMatrix[i].thrust = 1.0f; // All motors contribute equally to thrust
            mixerMatrix[i].pitch  = motors[i].y / (maxY ? maxY : 1); // Pitch: y offset (rotation about X)
            mixerMatrix[i].roll   = motors[i].x / (maxX ? maxX : 1); // Roll: x offset (rotation about Y)
            mixerMatrix[i].yaw    = static_cast<float>(motors[i].direction); // Yaw: CW/CCW
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
