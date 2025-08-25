#include <FS.h>
#include <SPIFFS.h>
#include "ConfigValue.h"

#include <Arduino.h>
#include <vector>

using namespace std;

static void configValuesSave();

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
    ESP_LOGD("Config", "Registering config: %s = %s", value->getName().c_str(), value->getValue().toString().c_str());
    registry.push_back(value);
}

ConfigValue::ConfigValue(const String &name, const String &descriptionHtml, Value defaultValue)
    : name(name), descriptionHtml(descriptionHtml), defaultValue(defaultValue), value(defaultValue) {
    registerConfig(this);
    if (defaultValue.getType() != value.getType()) {
        ESP_LOGE("Config", "Default value type %d does not match value type %d", defaultValue.getType(), value.getType());
    }
}

void ConfigValue::setValue(const Value &newValue) {
    if (newValue.getType() != defaultValue.getType()) {
        ESP_LOGW("Config", "Attempt to set value of type %d, expected %d", newValue.getType(), defaultValue.getType());
        return;
    }
    const auto oldValue = value;
    value = newValue;
    if (value != oldValue) {
        ESP_LOGI("Config", "Config set %s = %s (was %s)", name.c_str(), value.toString().c_str(), oldValue.toString().c_str());
        configValuesSave();
    }
}

void ConfigValue::setValueString(const String &newValueString) {
    const auto oldValue = value;
    value.setToString(newValueString);
    if (value != oldValue) {
        ESP_LOGI("Config", "Config set %s = %s (was %s) (from \"%s\")", name.c_str(), value.toString().c_str(), oldValue.toString().c_str(), newValueString.c_str());
        configValuesSave();
    }
}

void configValuesIterate(const std::function<void(const String &, const Value &)> &callback) {
    for (const auto *value : registry) {
        callback(value->getName(), value->getValue());
    }
}

void configDefaultValuesIterate(const std::function<void(const String &, const Value &)> &callback) {
    for (const auto *value : registry) {
        callback(value->getName(), value->getDefaultValue());
    }
}

bool configValueSetString(const String &name, const String &valueString) {
    if (auto *value = findConfig(name)) {
        value->setValueString(valueString);
        return true;
    }
    return false;
}

bool configValueRestore(const String &name) {
    if (auto *value = findConfig(name)) {
        value->setValue(value->getDefaultValue());
        return true;
    }
    return false;
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                listDir(fs, file.path(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

enum JSONParseState {
    JS_NeedObject,
    JS_NeedKey,
    JS_InKey,
    JS_NeedColon,
    JS_NeedValue,
    JS_InValue
};

void configValuesLoad() {
    // listDir(SPIFFS, "/", 0);
    const char *path = "/config.json";

    File file = SPIFFS.open(path);
    if (!file || file.isDirectory()) {
        Serial.println("! Failed to open config.json for reading");
        return;
    }

    JSONParseState state = JS_NeedObject;
    String key;
    String valueString;
    while (file.available()) {
        const auto ch = file.read();
        if (ch <= 0)
            break;
        switch (state) {
            case JS_NeedObject:
                if (ch == '{') {
                    state = JS_NeedKey;
                }
                break;
            case JS_NeedKey:
                if (ch == '"') {
                    state = JS_InKey;
                    key = "";
                }
                break;
            case JS_InKey:
                if (ch == '"') {
                    state = JS_NeedColon;
                }
                else {
                    key += (char)ch;
                }
                break;
            case JS_NeedColon:
                if (ch == ':') {
                    state = JS_NeedValue;
                }
                break;
            case JS_NeedValue:
                if ((ch >= '0' && ch <= '9') || (ch == '-')) {
                    state = JS_InValue;
                    valueString = (char)ch;
                }
                break;
            case JS_InValue:
                if (isdigit(ch) || ch == '.' || ch == '-' || ch == 'e' || ch == 'E') {
                    valueString += (char)ch;
                } else if (ch == ',' || ch == '}' || ch == '\n') {
                    ConfigValue *config = findConfig(key);
                    if (!config) {
                        ESP_LOGE("Config", "Unknown config key: %s", key.c_str());
                    }
                    else {
                        ESP_LOGD("Config", "Loaded config: %s = %s", key.c_str(), valueString.c_str());
                        config->value.setToString(valueString);
                    }
                    state = (ch == ',') ? JS_NeedKey : JS_NeedObject;
                } else {
                    ESP_LOGW("Config", "Unexpected character in value: %c", ch);
                }
                break;
        }
    }
    file.close();
}

static void configValuesSave() {
    const char *path = "/config.json";
    const char *tmpPath = "/config.json.tmp";

    if (SPIFFS.exists(tmpPath)) {
        SPIFFS.remove(tmpPath);
    }

    File file = SPIFFS.open(tmpPath, FILE_WRITE);
    if (!file) {
        ESP_LOGE("Config", "Failed to open config.json for writing");
        return;
    }
    file.print("{");
    const char *head = "\n  ";
    configValuesIterate([&file, &head](const String &name, const Value &value) {
        file.printf("%s\"%s\": %s", head, name.c_str(), value.toString().c_str());
        head = ",\n  ";
    });
    const auto ok = file.printf("\n}\n") == 3;
    file.close();
    if (ok) {
        if (SPIFFS.remove(path)) {
            SPIFFS.rename(tmpPath, path);
            ESP_LOGI("Config", "Config saved to %s", path);
        }
        else {
            ESP_LOGE("Config", "Failed to remove old file: %s", path);
            SPIFFS.remove(tmpPath);
        }
    } else {
        ESP_LOGE("Config", "Write failed");
    }
}
