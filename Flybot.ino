#include <cmath>

#include <WiFi.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <SPIFFS.h>
#include <EEPROM.h>

#include "Config.h"
#include "OTA.h"
#include "MPU6050.h"
#include "Geometry.h"
#include "RadioController.h"
#include "Motors.h"
#include "State.h"

const char *hostName = "flybot";
const char *serialNumber = "0000";

MPU6050 mpu;
AirframeConfig airframeConfig;

void controlLoop(MPU &mpu);
void webServerBegin();

#if __has_include("WiFiJoin.h")
#define WIFI_JOIN
#include "WiFiJoin.h"
#endif

enum CalMotorsMode {
    CMM_NotCalibrating = 0,
    CMM_CalibrationRequested = 1,
    CMM_CalibrationInProgress = 2
};

static uint8_t calMotorsMode = CMM_NotCalibrating;
static bool calMotorsModeDetermined = false;

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("================================");
    Serial.println("Flybot");

#ifdef WIFI_JOIN
#else
    char accessPointName[128];
    sprintf(accessPointName, "%s-%s", hostName, serialNumber);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 112, 1), IPAddress(192, 168, 112, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(accessPointName);
    Serial.printf("Access Point: %s\n", accessPointName);
    Serial.printf("IP Address:   %s\n", WiFi.softAPIP().toString().c_str());
    Serial.printf("Broadcast IP: %s\n", WiFi.softAPBroadcastIP().toString().c_str());
    Serial.printf("Network ID:   %s\n", WiFi.softAPNetworkID().toString().c_str());
    Serial.printf("Subnet Mask:  %s\n", WiFi.softAPSubnetMask().toString().c_str());
#endif
    Serial.printf("Host:         %s.local\n", hostName);
#ifdef LED_BUILTIN
    Serial.printf("LED Pin:      %d\n", LED_BUILTIN);
#endif
    MDNS.begin(hostName);
    MDNS.addService("http", "tcp", 80);

    otaSetup(hostName);

    Serial.println("================================");

    EEPROM.begin(512);
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
    }
    configValuesLoad();

    rcBegin();
    ledcSetClockSource(LEDC_AUTO_CLK);
    motorsSetup();
    Wire.begin();
    mpu.begin();

    webServerBegin();
}

static unsigned long lastCalMotorsPrintMillis = 0;

static void determineCalMotorsMode() {
    if (!calMotorsModeDetermined && rcDidReceiveData()) {
        calMotorsModeDetermined = true;
        EEPROM.get(0, calMotorsMode);
        const bool throttleHigh = rcGetInitialThrottle() > 0.98f;
        switch (calMotorsMode) {
        case CMM_NotCalibrating:
            if (throttleHigh) {
                calMotorsMode = CMM_CalibrationRequested;
                EEPROM.put(0, calMotorsMode);
                EEPROM.commit();
                Serial.println("Motor calibration requested. Please disconnect power and reconnect.");
            }
            break;
        case CMM_CalibrationRequested:
            if (throttleHigh) {
                calMotorsMode = CMM_CalibrationInProgress;
                EEPROM.put(0, calMotorsMode);
                EEPROM.commit();
                Serial.println("Motor calibration in progress. Decrease throttle after ESCs are calibrated.");
            }
            break;
        default:
            // Calibration is actually complete, reset back to not calibrating
            calMotorsMode = CMM_NotCalibrating;
            EEPROM.put(0, calMotorsMode);
            EEPROM.commit();
            Serial.println("Motor calibration complete.");
            break;
        }
    }
    const auto now = millis();
    if (calMotorsModeDetermined && (calMotorsMode != CMM_NotCalibrating) && (now - lastCalMotorsPrintMillis > 1000)) {
        lastCalMotorsPrintMillis = now;
        Serial.printf("Motor calibration mode: %d\n", calMotorsMode);
    }
}

void loop() {
    otaLoop();
    rcUpdate();
    determineCalMotorsMode();
    if (calMotorsMode == CMM_CalibrationInProgress) {
        const float thr = getState().rcThrottle;
        motorsSendCommands(thr, thr, thr, thr, thr, thr);
    }
    else {
        controlLoop(mpu);
    }
}
