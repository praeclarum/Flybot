#pragma once

#include <Wire.h>
#include <cstdint>

struct MPUData {
    float accelX; // Acceleration in g's
    float accelY; // Acceleration in g's
    float accelZ; // Acceleration in g's
    float gyroX;  // Gyro rate in radians per second
    float gyroY;  // Gyro rate in radians per second
    float gyroZ;  // Gyro rate in radians per second

    MPUData()
        : accelX(0.0f), accelY(0.0f), accelZ(0.0f),
          gyroX(0.0f), gyroY(0.0f), gyroZ(0.0f) {}
};

class MPU6050 {
private:
    TwoWire &i2c;
    const uint8_t address;
    MPUData data;

    void writeReg(uint8_t registerAddress, uint8_t data);
    float readGyro(uint8_t axis);
    float readAccel(uint8_t axis);
public:
    MPU6050(TwoWire &wire = Wire, uint8_t address = 0x68) : i2c(wire), address(address) {}
    void begin();
    MPUData readData();
};
