#pragma once

#include <Wire.h>
#include <cstdint>

#include "MPU.h"

class MPU6050 : public MPU {
private:
    TwoWire &i2c;
    const uint8_t address;
    MPUData data;

    void writeReg(uint8_t registerAddress, uint8_t data);
    float readGyro(uint8_t axis);
    float readAccel(uint8_t axis);
protected:
    MPUData readUncalibrated();
public:
    MPU6050(TwoWire &wire = Wire, uint8_t address = 0x68) : i2c(wire), address(address) {}
    ~MPU6050() {}
    void begin();
};
