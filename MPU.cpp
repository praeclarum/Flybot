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

MPUData MPU::read()
{
    auto data = readUncalibrated();
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
    return data;
}

void MPU::beginCalibration()
{
    isCalibrating = true;
    calData = readUncalibrated();
    calCount = 1;
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
