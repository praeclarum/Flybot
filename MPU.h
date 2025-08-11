#pragma once

#include "ConfigValue.h"
#include "Geometry.h"

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
    
    uint32_t updateCount;
    unsigned long lastUpdateMicros;
    bool readCalibrated(MPUData &data);

protected:
    virtual bool readUncalibrated(MPUData &data) = 0;

public:
    MPU();
    virtual ~MPU() {}
    virtual void begin() = 0;

    void update();

    Quaternion getOrientation() const {
        return orientation;
    }

    void beginCalibration();
    void endCalibration();
};
