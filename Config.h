#pragma once

#include "ConfigValue.h"

class MotorConfig {
public:
    ConfigValue x;          // X position relative to COM
    ConfigValue y;          // Y position relative to COM
    ConfigValue direction;  // +1 for CCW, -1 for CW

    MotorConfig(int index)
        : x("motor[" + String(index) + "].x", "The X position of the motor relative to the center of mass", Value::fromFloat(0.0f))
        , y("motor[" + String(index) + "].y", "The Y position of the motor relative to the center of mass", Value::fromFloat(0.0f))
        , direction("motor[" + String(index) + "].direction", "The direction of the motor (1 for CCW, -1 for CW)", Value::fromInt(1))
        {
    }
};

class Config {
public:
    ConfigValue numMotors;
    MotorConfig motor0;
    MotorConfig motor1;
    MotorConfig motor2;
    MotorConfig motor3;
    MotorConfig motor4;
    MotorConfig motor5;
    MotorConfig motor6;
    MotorConfig motor7;

    Config()
        : numMotors("numMotors", "The number of motors on this airframe", Value::fromInt(4))
        , motor0(0)
        , motor1(1)
        , motor2(2)
        , motor3(3)
        , motor4(4)
        , motor5(5)
        , motor6(6)
        , motor7(7)
        {
    }

    std::vector<MotorConfig*> getMotorConfigs() {
        
    }
};

extern Config config;
