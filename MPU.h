#pragma once

#include "ConfigValue.h"

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

struct Quaternion {
    float w, x, y, z;

    Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f)
        : w(w), x(x), y(y), z(z) {}

    void normalize() {
        float norm = sqrtf(w * w + x * x + y * y + z * z);
        if (norm > 0.0f) {
            w /= norm;
            x /= norm;
            y /= norm;
            z /= norm;
        }
    }
};

class LinearCal {
    ConfigValue scale;
    ConfigValue offset;
public:
    LinearCal(const String &name, const String &description)
        : scale(name + ".scale", description + " scale factor", Value::fromFloat(1.0f))
        , offset(name + ".offset", description + " offset", Value::fromFloat(0.0f))
    {}

    float apply(float input) const {
        return (input * scale.getValue().getFloat()) + offset.getValue().getFloat();
    }

    void set(float newScale, float newOffset) {
        scale.setValue(Value::fromFloat(newScale));
        offset.setValue(Value::fromFloat(newOffset));
    }
};

class MPU {
    LinearCal accelXCal;
    LinearCal accelYCal;
    LinearCal accelZCal;
    LinearCal gyroXCal;
    LinearCal gyroYCal;
    LinearCal gyroZCal;

    bool isCalibrating;
    uint32_t calCount;
    MPUData calData;

    Quaternion orientation;
protected:
    virtual MPUData readUncalibrated() = 0;
public:
    MPU();
    virtual ~MPU() {}
    virtual void begin() = 0;
    MPUData read();

    Quaternion getOrientation() const {
        return orientation;
    }

    void beginCalibration();
    void endCalibration();
};
