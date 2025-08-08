#include "PID.h"

PID::PID(
    const String &name, 
    float defaultKp, float defaultKi, float defaultKd,
    float defaultDFilter,
    float defaultILimit, float defaultDLimit, float defaultLimit)
    : kp(name + ".kp", "Proportional gain", Value::fromFloat(defaultKp))
    , ki(name + ".ki", "Integral gain", Value::fromFloat(defaultKi))
    , kd(name + ".kd", "Derivative gain", Value::fromFloat(defaultKd))
    , dfilter(name + ".dfilter", "Derivative term filter coefficient (0-1)", Value::fromFloat(defaultDFilter))
    , ilimit(name + ".ilimit", "Integral windup limit", Value::fromFloat(defaultILimit))
    , dlimit(name + ".dlimit", "Derivative term limit", Value::fromFloat(defaultDLimit))
    , limit(name + ".limit", "Output limit", Value::fromFloat(defaultLimit))
    , errorIntegral(0.0f)
    , lastError(0.0f)
    , lastDTerm(0.0f)
    , updateCount(0)
    , lastUpdateMicros(0)
    , lastOutput(0.0f) {
}

PID::~PID() {
}

float PID::updateError(float error) {
    updateCount++;
    unsigned long now = micros();
    if (updateCount == 1) {
        lastError = error;
        lastUpdateMicros = now;
        return lastOutput; // First update, no previous error to compare
    }

    const unsigned long deltaMicros = now - lastUpdateMicros;
    if (deltaMicros == 0) {
        return lastOutput; // No time has passed, return last output
    }
    lastUpdateMicros = now;

    float dt = deltaMicros * 1e-6f; // Convert to seconds

    // Proportional term
    float pTerm = kp.getFloat() * error;

    // Integral term
    errorIntegral += error * dt;
    if (errorIntegral > ilimit.getFloat()) {
        errorIntegral = ilimit.getFloat();
    } else if (errorIntegral < -ilimit.getFloat()) {
        errorIntegral = -ilimit.getFloat();
    }
    float iTerm = ki.getFloat() * errorIntegral;

    // Derivative term with filtering and limiting
    float rawDTerm = kd.getFloat() * (error - lastError) / dt;
    float filterCoeff = dfilter.getFloat();
    if (filterCoeff < 0.0f) filterCoeff = 0.0f;
    if (filterCoeff > 1.0f) filterCoeff = 1.0f;
    float dTerm = lastDTerm + filterCoeff * (rawDTerm - lastDTerm);
    
    // Apply D-term limiting
    float dLimitValue = dlimit.getFloat();
    if (dTerm > dLimitValue) {
        dTerm = dLimitValue;
    } else if (dTerm < -dLimitValue) {
        dTerm = -dLimitValue;
    }
    
    lastDTerm = dTerm;
    lastError = error;

    // Combine terms using negative feedback
    // (e.g., if if the error is positive, we want to reduce the output)
    float output = -pTerm - iTerm - dTerm;
    if (output > limit.getFloat()) {
        output = limit.getFloat();
    } else if (output < -limit.getFloat()) {
        output = -limit.getFloat();
    }

    lastOutput = output;
    return output;
}

void PID::resetErrorIntegral() {
    errorIntegral = 0.0f;
}

TrackingPID::TrackingPID(
    const String &name, 
        float defaultKp, float defaultKi, float defaultKd,
        float defaultDFilter,
        float defaultILimit, float defaultDLimit, float defaultLimit)
    : PID(name, defaultKp, defaultKi, defaultKd, defaultDFilter, defaultILimit, defaultDLimit, defaultLimit)
    , target(0.0f)
    , position(0.0f) {
}

TrackingPID::~TrackingPID() {
}

float TrackingPID::update(float newPosition, float newTarget) {
    const auto targetChangeDivisor = max(abs(newTarget), abs(target));
    
    // Reset integral term when target changes dramatically
    if (targetChangeDivisor > 0.0f && abs(newTarget - target) / targetChangeDivisor > 0.1f) {
        resetErrorIntegral();
    }
    target = newTarget;
    position = newPosition;
    float error = position - target;
    return updateError(error);
}
