#pragma once

#include <Arduino.h>
#include <functional>
#include "Value.h"

class ConfigValue {
    String name;
    String descriptionHtml;
    Value defaultValue;
    Value value;

    friend void configValuesLoad();
public:
    ConfigValue(const String &name, const String &descriptionHtml, Value defaultValue);
    inline const String &getName() const {
        return name;
    }
    inline const String &getDescriptionHtml() const {
        return descriptionHtml;
    }
    inline Value getDefaultValue() const {
        return defaultValue;
    }
    inline Value getValue() const {
        return value;
    }
    void setValue(const Value &newValue);
    void setValueString(const String &newValueString);
    inline int32_t getInt() const {
        return value.getInt();
    }
    inline float getFloat() const {
        return value.getFloat();
    }
    inline bool isInt() const {
        return value.isInt();
    }
    inline bool isFloat() const {
        return value.isFloat();
    }
    inline String toString(int decimalPlaces = -1) const {
        return value.toString(decimalPlaces);
    }
    
};

void configValuesLoad();
void configValuesIterate(const std::function<void(const String &, const Value &)> &callback);
void configDefaultValuesIterate(const std::function<void(const String &, const Value &)> &callback);
bool configValueSetString(const String &name, const String &valueString);
bool configValueRestore(const String &name);
