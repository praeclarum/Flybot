#include <Arduino.h>
#include "ConfigValue.h"

class PID {
    ConfigValue kp;
    ConfigValue ki;
    ConfigValue kd;
    ConfigValue dfilter;
    ConfigValue ilimit;
    ConfigValue dlimit;
    ConfigValue limit;

    float errorIntegral;
    float lastError;
    float lastDTerm;
    int updateCount;
    unsigned long lastUpdateMicros;

    float lastOutput;

public:
    PID(const String &name,
        float defaultKp, float defaultKi, float defaultKd,
        float defaultDFilter,
        float defaultILimit, float defaultDLimit, float defaultLimit);
    virtual ~PID();

    float updateError(float error);

    void resetErrorIntegral();

    inline float getOutput() const {
        return lastOutput;
    }
};

class TrackingPID : public PID {
    float target;
    float position;
public:
    TrackingPID(const String &name, 
        float defaultKp, float defaultKi, float defaultKd,
        float defaultDFilter,
        float defaultILimit, float defaultDLimit, float defaultLimit);
    virtual ~TrackingPID();

    inline float getTarget() const {
        return target;
    }
    inline float getPosition() const {
        return position;
    }

    float update(float newPosition, float newTarget);
};
