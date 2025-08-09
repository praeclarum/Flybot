#include "MPU6050.h"

#define MPU6050_ACCEL_XOUT_H    0x3B
#define MPU6050_GYRO_XOUT_H     0x43

#define MPU6050_SMPLRT_DIV  	0x19
#define MPU6050_CONFIG      	0x1A
#define MPU6050_GYRO_CONFIG 	0x1B
#define MPU6050_ACCEL_CONFIG	0x1C
#define MPU6050_PWR_MGMT_1      0x6B

void MPU6050::begin() {
	writeReg(MPU6050_SMPLRT_DIV, 0x00);
	writeReg(MPU6050_CONFIG, 0x00);
	writeReg(MPU6050_GYRO_CONFIG, 0x08);
	writeReg(MPU6050_ACCEL_CONFIG, 0x00);
	writeReg(MPU6050_PWR_MGMT_1, 0x01);
}

MPUData MPU6050::readData() {
    MPUData data;

    // Read gyroscope data
    i2c.beginTransmission(address);
    i2c.write(MPU6050_GYRO_XOUT_H);
    i2c.endTransmission(false);
    i2c.requestFrom(address, 6, true);
    data.gyroX = readGyro(0);
    data.gyroY = readGyro(1);
    data.gyroZ = readGyro(2);

    // Read accelerometer data
    i2c.beginTransmission(address);
    i2c.write(MPU6050_ACCEL_XOUT_H);
    i2c.endTransmission(false);
    i2c.requestFrom(address, 6, true);
    data.accelX = readAccel(0);
    data.accelY = readAccel(1);
    data.accelZ = readAccel(2);

    return data;
}

float MPU6050::readGyro(uint8_t axis) {
    uint8_t highByte = i2c.read();
    uint8_t lowByte = i2c.read();
    int16_t rawValue = (highByte << 8) | lowByte;
    return static_cast<float>(rawValue) / 131.0f; // Convert to degrees per second
}

float MPU6050::readAccel(uint8_t axis) {
    uint8_t highByte = i2c.read();
    uint8_t lowByte = i2c.read();
    int16_t rawValue = (highByte << 8) | lowByte;
    return static_cast<float>(rawValue) / 16384.0f; // Convert to g's
}

void MPU6050::writeReg(uint8_t registerAddress, uint8_t data) {
    i2c.beginTransmission(address);
    i2c.write(registerAddress);
    i2c.write(data);
    i2c.endTransmission();
}
