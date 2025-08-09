#include <Arduino.h>
#include "Geometry.h"
#include "MPU.h"
#include "State.h"

#define CONTROL_LOOP_HZ 100
#define CONTROL_LOOP_INTERVAL_MICROS (1000000 / CONTROL_LOOP_HZ)

static const float deg2rad = 0.017453292519943295769236907684886f;
static const float rad2deg = 57.295779513082320876798154814105f;

static unsigned long nextControlLoopMicros = 0;
static int loopCounter = 0;

const int numCalCount = 300;

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
    
    // Read sensor data
    // const auto mpuData = mpu6050.read();
    // Serial.printf("%f,%f,%f\n", mpuData.accelX, mpuData.accelY, mpuData.accelZ);
    // Serial.printf("%f,%f,%f\n", mpuData.gyroX, mpuData.gyroY, mpuData.gyroZ);
    // Serial.println(dtMicros);
    if (loopCounter == 0) {
        mpu.beginCalibration();
    }
    else if (loopCounter == numCalCount) {
        mpu.endCalibration();
    }
    mpu.update();

    Quaternion qYaw, qRoll;
    const float pitchCommandDeg = 45.0f;
    Quaternion qPitch = Quaternion::fromEulerAngles(Vector(pitchCommandDeg * deg2rad, 0.0f, 0.0f));
    const auto currentOrientation = mpu.getOrientation();
    Quaternion qCmd = combineCommands(qYaw, qPitch, qRoll);
    Quaternion qError = getOrientationError(currentOrientation, qCmd);
    Vector orientEuler = currentOrientation.toEulerAngles();
    Vector errorEuler = qError.toEulerAngles();

    stateUpdateOrientation(orientEuler.x, orientEuler.y, orientEuler.z);

    // Serial.printf("%.3f,%.3f,%.3f\n", orientEuler.x * rad2deg, pitchCommandDeg, errorEuler.x * rad2deg);

    loopCounter++;
}
