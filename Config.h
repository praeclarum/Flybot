#pragma once

#include <vector>

#include "ConfigValue.h"

class MotorConfig {
public:
    ConfigValue x;          // X position relative to COM
    ConfigValue y;          // Y position relative to COM
    ConfigValue direction;  // +1 for CCW, -1 for CW

    MotorConfig(int index)
        : x("motor" + String(index) + ".x", "The X position of the motor relative to the center of mass", Value::fromFloat(0.0f))
        , y("motor" + String(index) + ".y", "The Y position of the motor relative to the center of mass", Value::fromFloat(0.0f))
        , direction("motor" + String(index) + ".direction", "The direction of the motor (1 for CCW, -1 for CW)", Value::fromInt(1))
        {
    }
};

class AirframeConfig {
public:
    ConfigValue numMotors;
    MotorConfig motor1;
    MotorConfig motor2;
    MotorConfig motor3;
    MotorConfig motor4;
    MotorConfig motor5;
    MotorConfig motor6;
    ConfigValue comX;
    ConfigValue comY;

    AirframeConfig()
        : numMotors("numMotors", "The number of motors on this airframe", Value::fromInt(4))
        , motor1(1)
        , motor2(2)
        , motor3(3)
        , motor4(4)
        , motor5(5)
        , motor6(6)
        , comX("com.x", "The X position of the center of mass", Value::fromFloat(0.0f))
        , comY("com.y", "The Y position of the center of mass", Value::fromFloat(0.0f))
        {
    }

    inline std::vector<MotorConfig*> getMotorConfigs() {
        std::vector<MotorConfig*> motors;
        int num = numMotors.getInt();
        if (num <= 0) {
            ESP_LOGW("Config", "No motors configured");
            return motors;
        }
        if (num > 8) {
            ESP_LOGW("Config", "Number of motors exceeds maximum supported (8), clamping to 8");
            num = 8;
        }
        motors.reserve(num);
        for (int i = 1; i <= num; ++i) {
            switch (i) {
                case 1: motors.push_back(&motor1); break;
                case 2: motors.push_back(&motor2); break;
                case 3: motors.push_back(&motor3); break;
                case 4: motors.push_back(&motor4); break;
                case 5: motors.push_back(&motor5); break;
                case 6: motors.push_back(&motor6); break;
                default: ESP_LOGW("Config", "Unexpected motor index %d, skipping", i);
            }
        }
        return motors;
    }
};

extern AirframeConfig airframeConfig;
