#pragma once

#include <cstdint>

#include <esp_log.h>

enum ValueType {
    VT_Int = 0,
    VT_Float = 1,
};

class Value {
    ValueType type;
    union {
        std::int32_t intValue;
        float floatValue;
    } value;
    inline Value(ValueType type) : type(type) {
        value.intValue = 0; // Default to zero
    }
public:
    inline static Value fromInt(std::int32_t v) {
        Value val(VT_Int);
        val.value.intValue = v;
        return val;
    }
    inline static Value fromFloat(float v) {
        Value val(VT_Float);
        val.value.floatValue = v;
        return val;
    }
    inline ValueType getType() const {
        return type;
    }
    inline std::int32_t getInt() const {
        if (type != VT_Int) {
            ESP_LOGW("Config", "getInt called on non-int value");
            return (std::int32_t)value.floatValue;
        }
        return value.intValue;
    }
    inline float getFloat() const {
        if (type != VT_Float) {
            ESP_LOGW("Config", "getFloat called on non-float value");
            return (float)value.intValue;
        }
        return value.floatValue;
    }
    inline bool isInt() const {
        return type == VT_Int;
    }
    inline bool isFloat() const {
        return type == VT_Float;
    }
    inline bool operator==(const Value& other) const {
        if (type != other.type) return false;
        if (type == VT_Int) {
            return value.intValue == other.value.intValue;
        } else {
            return value.floatValue == other.value.floatValue;
        }
    }
    inline bool operator!=(const Value& other) const {
        return !(*this == other);
    }
    inline bool operator<(const Value& other) const {
        if (type != other.type) {
            ESP_LOGW("Config", "Comparison between different types");
            return getFloat() < other.getFloat();
        }
        if (type == VT_Int) {
            return value.intValue < other.value.intValue;
        } else {
            return value.floatValue < other.value.floatValue;
        }
    }
    inline String toString(int decimalPlaces = -1) const {
        if (type == VT_Int) {
            return String(value.intValue);
        } else {
            char str[64];
            if (decimalPlaces < 0) {
                snprintf(str, sizeof(str), "%g", value.floatValue);
            } else {
                snprintf(str, sizeof(str), "%.*f", decimalPlaces, value.floatValue);
            }
            return String(str);
        }
    }
};
