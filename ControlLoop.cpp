#include <Arduino.h>
#include "Geometry.h"
#include "MPU.h"
#include "State.h"
#include "PID.h"
#include "Motors.h"
#include "StateMachine.h"

#define CONTROL_LOOP_HZ 100
#define CONTROL_LOOP_INTERVAL_MICROS (1000000 / CONTROL_LOOP_HZ)

static const float deg2rad = 0.017453292519943295769236907684886f;
static const float rad2deg = 57.295779513082320876798154814105f;
static const float piOver2 = 1.5707963267948966192313216916398f;

static unsigned long nextControlLoopMicros = 0;
static int loopCounter = 0;

const int numCalCount = 300;

static const float dDefaultFilter = 0.1f;
static const float dDefaultLimit = 0.2f;
static const float iDefaultLimit = 0.2f;
static const float defaultLimit = 1.0f;

PID pitchPID("pitchPID", 1.0f / (piOver2), 0.0f, 0.0f,
    dDefaultFilter, iDefaultLimit, dDefaultLimit, defaultLimit);
PID rollPID("rollPID", 1.0f / (piOver2), 0.0f, 0.0f,
    dDefaultFilter, iDefaultLimit, dDefaultLimit, defaultLimit);
MotorMixer motorMixer;

static Quaternion combineCommands(const Quaternion& qYaw, const Quaternion& qPitch, const Quaternion& qRoll) {
    // Typical order: yaw * pitch * roll
    return qYaw * qPitch * qRoll;
}

static Quaternion getOrientationError(const Quaternion &current, const Quaternion &target) {
    return current * target.inverse();
}

void controlLoop(MPU &mpu) {
    unsigned long nowMicros = micros();
    if (nextControlLoopMicros == 0) {
        nextControlLoopMicros = nowMicros + CONTROL_LOOP_INTERVAL_MICROS;
        return;
    }
    if (nowMicros < nextControlLoopMicros) {
        return;
    }
    nextControlLoopMicros += CONTROL_LOOP_INTERVAL_MICROS;
    if (nextControlLoopMicros <= nowMicros) {
        nextControlLoopMicros = nowMicros + CONTROL_LOOP_INTERVAL_MICROS;
    }
    
    //
    // Read sensor data
    //
    if (loopCounter == 0) {
        mpu.beginCalibration();
    }
    else if (loopCounter == numCalCount) {
        mpu.endCalibration();
    }
    const bool mpuOk = mpu.update();
    const auto currentOrientation = mpu.getOrientation();
    const Vector orientEuler = currentOrientation.toEulerAngles();
    stateUpdateOrientation(orientEuler.x, orientEuler.y, orientEuler.z, mpuOk);

    //
    // Run state machine
    //
    flightState.update();

    //
    // Compute control errors
    //
    const State stateBeforeCommands = getState();
    const float pitchCommandRad = stateBeforeCommands.rcPitchRadians;
    const float rollCommandRad = stateBeforeCommands.rcRollRadians;
    const Quaternion qYaw;
    const auto qPitch = Quaternion::fromEulerAngles(Vector(pitchCommandRad, 0.0f, 0.0f));
    const auto qRoll = Quaternion::fromEulerAngles(Vector(0.0f, rollCommandRad, 0.0f));
    const auto qCmd = combineCommands(qYaw, qPitch, qRoll);
    const auto qError = getOrientationError(currentOrientation, qCmd);
    const Vector errorEuler = qError.toEulerAngles();
    stateUpdateControlErrors(errorEuler.x, errorEuler.y);

    //
    // Compute PID outputs
    //
    // Serial.printf("%.3f,%.3f,%.3f\n", orientEuler.x * rad2deg, pitchCommandDeg, errorEuler.x * rad2deg);
    const float pitchOutput = pitchPID.updateError(errorEuler.x);
    const float rollOutput = rollPID.updateError(errorEuler.y);

    //
    // Mix outputs into motor commands
    //
    motorMixer.updateMotorMix();
    MixValues mixValues;
    mixValues.thrust = stateBeforeCommands.rcThrottle;
    mixValues.pitch = pitchOutput;
    mixValues.roll = rollOutput;
    mixValues.yaw = stateBeforeCommands.rcYaw;
    motorMixer.mix(mixValues);
    stateUpdateMotorCommands(
        motorMixer.getMotorCommand(0),
        motorMixer.getMotorCommand(1),
        motorMixer.getMotorCommand(2),
        motorMixer.getMotorCommand(3),
        motorMixer.getMotorCommand(4),
        motorMixer.getMotorCommand(5));

    loopCounter++;
}
