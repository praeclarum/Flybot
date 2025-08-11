#include "MPU6050.h"

#define MPU6050_ACCEL_XOUT_H    0x3B
#define MPU6050_GYRO_XOUT_H     0x43

#define MPU6050_SMPLRT_DIV  	0x19
#define MPU6050_CONFIG      	0x1A
#define MPU6050_GYRO_CONFIG 	0x1B
#define MPU6050_ACCEL_CONFIG	0x1C
#define MPU6050_PWR_MGMT_1      0x6B

#define PI 3.14159265358979323846f

void i2cReset(TwoWire &i2c) {
    i2c.end();
    delay(100);
    i2c.begin();
}

bool i2cRequestFrom(TwoWire &i2c, uint8_t address, uint8_t reg, size_t len) {
    i2c.beginTransmission(address);
    i2c.write(reg);
    if (i2c.endTransmission(false) != 0) {
        i2cReset(i2c);
        return false;
    }
    if (i2c.requestFrom(address, len, true) != len) {
        i2cReset(i2c);
        return false;
    }
    return true;
}

bool MPU6050::writeInit() {
    needsInit = true;
    ESP_LOGI("MPU6050", "Initializing MPU6050 at address 0x%02X", address);
    if (!writeReg(MPU6050_SMPLRT_DIV, 0x00)) return false;
    if (!writeReg(MPU6050_CONFIG, 0x00)) return false;
    if (!writeReg(MPU6050_GYRO_CONFIG, 0x08)) return false; // +/- 500 degrees/sec
    if (!writeReg(MPU6050_ACCEL_CONFIG, 0x00)) return false; // +/-2g range
    if (!writeReg(MPU6050_PWR_MGMT_1, 0x01)) return false;
    needsInit = false;
    return true;
}

void MPU6050::begin() {
    needsInit = true;
}

bool MPU6050::readUncalibrated(MPUData &data) {
    if (needsInit) {
        if (!writeInit()) {
            return false;
        }
    }

    // Read gyroscope data
    if (!i2cRequestFrom(i2c, address, MPU6050_GYRO_XOUT_H, 6)) {
        needsInit = true;
        return false;
    }
    data.gyroX = readGyro(0);
    data.gyroY = readGyro(1);
    data.gyroZ = readGyro(2);

    // Read accelerometer data
    if (!i2cRequestFrom(i2c, address, MPU6050_ACCEL_XOUT_H, 6)) {
        needsInit = true;
        return false;
    }
    data.accelX = readAccel(0);
    data.accelY = readAccel(1);
    data.accelZ = readAccel(2);

    return true;
}

float MPU6050::readGyro(uint8_t axis) {
    uint8_t highByte = i2c.read();
    uint8_t lowByte = i2c.read();
    uint16_t rawValue = ((uint16_t)highByte << 8) | lowByte;
    return static_cast<float>((int16_t)rawValue) * (PI / 180.0f / 65.5f);
}

float MPU6050::readAccel(uint8_t axis) {
    uint8_t highByte = i2c.read();
    uint8_t lowByte = i2c.read();
    uint16_t rawValue = ((uint16_t)highByte << 8) | lowByte;
    return static_cast<float>((int16_t)rawValue) / 16384.0f; // Convert to g's
}

bool MPU6050::writeReg(uint8_t registerAddress, uint8_t data) {
    i2c.beginTransmission(address);
    i2c.write(registerAddress);
    i2c.write(data);
    if (i2c.endTransmission() != 0) {
        i2cReset(i2c);
        return false;
    }
    return true;
}
