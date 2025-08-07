#include "ConfigValue.h"

#include <Arduino.h>
#include <vector>

using namespace std;

static vector<ConfigValue*> registry;

static ConfigValue* findConfig(const String &name) {
    for (auto *value : registry) {
        if (value->getName() == name) {
            return value;
        }
    }
    return nullptr;
}

static void registerConfig(ConfigValue *value) {
    if (findConfig(value->getName()) != nullptr) {
        ESP_LOGE("Config", "Duplicate config name: %s", value->getName().c_str());
        return;
    }
    ESP_LOGI("Config", "Registering config: %s = %s", value->getName().c_str(), value->getValue().toString().c_str());
    registry.push_back(value);
}

ConfigValue::ConfigValue(const String &name, const String &descriptionHtml, Value defaultValue)
    : name(name), descriptionHtml(descriptionHtml), defaultValue(defaultValue), value(defaultValue) {
    registerConfig(this);
    if (defaultValue.getType() != value.getType()) {
        ESP_LOGW("Config", "Default value type %d does not match value type %d", defaultValue.getType(), value.getType());
    }
}

void ConfigValue::setValue(const Value &newValue) {
    if (newValue.getType() != defaultValue.getType()) {
        ESP_LOGW("Config", "Attempt to set value of type %d, expected %d", newValue.getType(), defaultValue.getType());
        return;
    }
    value = newValue;
}
