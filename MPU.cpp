#include "MPU.h"

MPU::MPU()
    : accelXCal("MPU.accelX", "Accelerometer X calibration")
    , accelYCal("MPU.accelY", "Accelerometer Y calibration")
    , accelZCal("MPU.accelZ", "Accelerometer Z calibration")
    , gyroXCal("MPU.gyroX", "Gyro X calibration")
    , gyroYCal("MPU.gyroY", "Gyro Y calibration")
    , gyroZCal("MPU.gyroZ", "Gyro Z calibration")
    , calCount(0)
{
}

bool MPU::readCalibrated(MPUData &data)
{
    if (!readUncalibrated(data)) {
        return false;
    }
    if (isCalibrating) {
        calData.accelX += data.accelX;
        calData.accelY += data.accelY;
        calData.accelZ += data.accelZ;
        calData.gyroX += data.gyroX;
        calData.gyroY += data.gyroY;
        calData.gyroZ += data.gyroZ;
        calCount++;
    }
    data.accelX = accelXCal.apply(data.accelX);
    data.accelY = accelYCal.apply(data.accelY);
    data.accelZ = accelZCal.apply(data.accelZ);
    data.gyroX = gyroXCal.apply(data.gyroX);
    data.gyroY = gyroYCal.apply(data.gyroY);
    data.gyroZ = gyroZCal.apply(data.gyroZ);
    return true;
}

void MPU::beginCalibration()
{
    isCalibrating = true;
    calData = MPUData();
    calCount = 0;
}

void MPU::endCalibration()
{
    const auto ax = calData.accelX / calCount;
    const auto ay = calData.accelY / calCount;
    const auto az = calData.accelZ / calCount;
    const auto gx = calData.gyroX / calCount;
    const auto gy = calData.gyroY / calCount;
    const auto gz = calData.gyroZ / calCount;
    isCalibrating = false;
    calCount = 0;
    const auto accelScale = 1.0f / sqrtf(ax * ax + ay * ay + az * az);
    accelXCal.set(accelScale, 0.0f);
    accelYCal.set(accelScale, 0.0f);
    accelZCal.set(accelScale, 0.0f);
    gyroXCal.set(1.0f, -gx);
    gyroYCal.set(1.0f, -gy);
    gyroZCal.set(1.0f, -gz);
}

static Quaternion madgwickUpdate(float deltat, const MPUData &data, const Quaternion &orientation, const float gyroMeasureError)
{
    // Read the previous state
    float SEq_1 = orientation.w;
    float SEq_2 = orientation.x;
    float SEq_3 = orientation.y;
    float SEq_4 = orientation.z;
    
    // Axulirary variables to avoid reapeated calcualtions
    const float halfSEq_1 = 0.5f * SEq_1;
    const float halfSEq_2 = 0.5f * SEq_2;
    const float halfSEq_3 = 0.5f * SEq_3;
    const float halfSEq_4 = 0.5f * SEq_4;
    const float twoSEq_1 = 2.0f * SEq_1;
    const float twoSEq_2 = 2.0f * SEq_2;
    const float twoSEq_3 = 2.0f * SEq_3;
    
    // Normalise the accelerometer measurement
    float a_x = data.accelX;
    float a_y = data.accelY;
    float a_z = data.accelZ;
    float norm = sqrtf(a_x * a_x + a_y * a_y + a_z * a_z);
    if (norm == 0.0f) return orientation; // Return unchanged orientation
    a_x /= norm;
    a_y /= norm;
    a_z /= norm;
    
    // Compute the objective function and Jacobian
    const float f_1 = twoSEq_2 * SEq_4 - twoSEq_1 * SEq_3 - a_x;
    const float f_2 = twoSEq_1 * SEq_2 + twoSEq_3 * SEq_4 - a_y;
    const float f_3 = 1.0f - twoSEq_2 * SEq_2 - twoSEq_3 * SEq_3 - a_z;
    const float J_11or24 = twoSEq_3; // J_11 negated in matrix multiplication
    const float J_12or23 = 2.0f * SEq_4; // J_12 negated in matrix multiplication
    const float J_13or22 = twoSEq_1;
    const float J_14or21 = twoSEq_2;
    const float J_32 = 2.0f * J_14or21; // negated in matrix multiplication
    const float J_33 = 2.0f * J_11or24; // negated in matrix multiplication
    
    // Compute the gradient (matrix multiplication)
    float SEqHatDot_1 = J_14or21 * f_2 - J_11or24 * f_1;
    float SEqHatDot_2 = J_12or23 * f_1 + J_13or22 * f_2 - J_32 * f_3;
    float SEqHatDot_3 = J_12or23 * f_2 - J_33 * f_3 - J_13or22 * f_1;
    float SEqHatDot_4 = J_14or21 * f_1 + J_11or24 * f_2;
    
    // Normalise the gradient
    norm = sqrt(SEqHatDot_1 * SEqHatDot_1 + SEqHatDot_2 * SEqHatDot_2 + SEqHatDot_3 * SEqHatDot_3 + SEqHatDot_4 * SEqHatDot_4);
    if (norm == 0.0f) return orientation; // Return unchanged orientation
    SEqHatDot_1 /= norm;
    SEqHatDot_2 /= norm;
    SEqHatDot_3 /= norm;
    SEqHatDot_4 /= norm;
    
    // Compute the quaternion derrivative measured by gyroscopes
    const float w_x = data.gyroX;
    const float w_y = data.gyroY;
    const float w_z = data.gyroZ;
    const float SEqDot_omega_1 = -halfSEq_2 * w_x - halfSEq_3 * w_y - halfSEq_4 * w_z;
    const float SEqDot_omega_2 = halfSEq_1 * w_x + halfSEq_3 * w_z - halfSEq_4 * w_y;
    const float SEqDot_omega_3 = halfSEq_1 * w_y - halfSEq_2 * w_z + halfSEq_4 * w_x;
    const float SEqDot_omega_4 = halfSEq_1 * w_z + halfSEq_2 * w_y - halfSEq_3 * w_x;
    
    // Compute then integrate the estimated quaternion derrivative
    // const float beta = sqrtf(3.0f / 4.0f) * gyroMeasError;
    const float beta = 0.8660254038 * gyroMeasureError;
    SEq_1 += (SEqDot_omega_1 - (beta * SEqHatDot_1)) * deltat;
    SEq_2 += (SEqDot_omega_2 - (beta * SEqHatDot_2)) * deltat;
    SEq_3 += (SEqDot_omega_3 - (beta * SEqHatDot_3)) * deltat;
    SEq_4 += (SEqDot_omega_4 - (beta * SEqHatDot_4)) * deltat;
    
    // Normalise quaternion
    norm = sqrt(SEq_1 * SEq_1 + SEq_2 * SEq_2 + SEq_3 * SEq_3 + SEq_4 * SEq_4);
    if (norm == 0.0f) return orientation; // Return unchanged orientation
    SEq_1 /= norm;
    SEq_2 /= norm;
    SEq_3 /= norm;
    SEq_4 /= norm;

    return Quaternion(SEq_1, SEq_2, SEq_3, SEq_4);
}

void MPU::update()
{
    const auto nowMicros = micros();
    if (updateCount == 0) {
        orientation = Quaternion();
    } else {
        MPUData data;
        if (!readCalibrated(data)) {
            return;
        }
        const float dt = (nowMicros - lastUpdateMicros) * 1e-6f;
        orientation = madgwickUpdate(dt, data, orientation, 0.1f);
    }
    lastUpdateMicros = nowMicros;
    updateCount++;
}
