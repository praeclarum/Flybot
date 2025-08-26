// MotorControl.cpp
// Mixer algorithm for mapping control torques and thrust to per-motor outputs
// Private implementation, not exposed via header

#include <cmath>
#include <algorithm>
#include <Arduino.h>

#include "Motors.h"
#include "Config.h" // For MotorConfig
#include "State.h"

static const uint32_t pwmFrequency = 50;

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
        mixerMatrix[i].thrust = 1.0f; // All motors contribute equally to thrust
        mixerMatrix[i].pitch  = motors[i]->y.getFloat() / maxY; // Pitch: y offset (rotation about X)
        mixerMatrix[i].roll   = -motors[i]->x.getFloat() / maxX; // Roll: x offset (rotation about Y)
        mixerMatrix[i].yaw    = motors[i]->direction.getInt() < 0 ? -1.0f : 1.0f; // Yaw: motor direction
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
        if (newOutput < 0.0f) newOutput = 0.0f;
        if (newOutput > maxOut) maxOut = newOutput;
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

static void hwSetSpeed(uint8_t pin, float speed, bool print) {
    // Speed is 0.0 to 1.0
    if (speed < 0.0f) speed = 0.0f;
    if (speed > 1.0f) speed = 1.0f;
    // Pulse width is 1.0e-3 to 2.0e-3
    const float pulseWidth = speed * 1.0e-3f + 1.0e-3f;
    // Duty cycle is 0.0 to 1.0
    float dutyCycle = pulseWidth * float(pwmFrequency);
    if (dutyCycle < 0.0f) dutyCycle = 0.0f;
    else if (dutyCycle > 1.0f) dutyCycle = 1.0f;
    const uint16_t pwm = int(dutyCycle * 4095.0f + 0.5f);
    if (print) {
        ESP_LOGI("Motors", "Set pin %2d to speed=%6.3f, pulse width=%5.3f ms (pwm=%4d)", pin, speed, pulseWidth * 1000.0f, pwm);
    }
    ledcWrite(pin, pwm);
}

static void initMotorPin(uint8_t pin, uint8_t channel) {
    digitalWrite(pin, 0);
    pinMode(pin, OUTPUT);
    ledcAttachChannel(pin, pwmFrequency, 12, channel);
    hwSetSpeed(pin, 0.0f, true);
}

void motorsSetup() {
    initMotorPin(2, 1);
    initMotorPin(4, 2);
    initMotorPin(12, 3);
    initMotorPin(13, 4);
    initMotorPin(14, 5);
    initMotorPin(15, 6);
}

static unsigned long lastPrintMillis = 0;

void motorsSendCommands(float motor1, float motor2, float motor3, float motor4, float motor5, float motor6) {
    const auto now = millis();
    bool shouldPrint = false;
    if (now - lastPrintMillis > 1000) {
        lastPrintMillis = now;
        shouldPrint = true;
    }

    // motor1 = -1.0f;
    // motor2 = motor1;
    // motor3 = motor1;
    // motor4 = motor1;
    // motor5 = motor1;
    // motor6 = motor1;
    hwSetSpeed(2, motor1, shouldPrint);
    hwSetSpeed(4, motor2, shouldPrint);
    hwSetSpeed(12, motor3, shouldPrint);
    hwSetSpeed(13, motor4, shouldPrint);
    hwSetSpeed(14, motor5, false);
    hwSetSpeed(15, motor6, false);
    stateUpdateMotorCommands(motor1, motor2, motor3, motor4, motor5, motor6);
}
