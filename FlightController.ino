#include <cmath>

#include <WiFi.h>
#include <ESPmDNS.h>

#include "Config.h"
#include "OTA.h"
#include "MPU6050.h"
#include "Geometry.h"

const char *hostName = "FlightController";
const char *serialNumber = "0000";

#define CONTROL_LOOP_HZ 100
#define CONTROL_LOOP_INTERVAL_MICROS (1000000 / CONTROL_LOOP_HZ)

using namespace std;

MPU6050 mpu;

unsigned long lastControlLoopMicros = 0;
unsigned long nextControlLoopMicros = 0;

AirframeConfig airframeConfig;

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("================================");
    Serial.println("FlightController");

    char accessPointName[128];
    sprintf(accessPointName, "%s-%s", hostName, serialNumber);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(accessPointName);
    const auto ip = WiFi.softAPIP();
    MDNS.begin(hostName);
    MDNS.addService("http", "tcp", 80);

    Serial.printf("Access Point: %s\n", accessPointName);
    Serial.printf("Host:         %s.local\n", hostName);
    Serial.printf("IP Address:   %s\n", ip.toString().c_str());

    otaSetup(hostName);

    Serial.println("================================");

    Wire.begin();
    mpu.begin();

    lastControlLoopMicros = micros();
}

const int numCalCount = 300;
int loopCounter = 0;

void loop() {
    otaLoop();
    unsigned long nowMicros = micros();
    const auto dtMicros = nowMicros - lastControlLoopMicros;
    if (nowMicros >= nextControlLoopMicros) {
        nextControlLoopMicros += CONTROL_LOOP_INTERVAL_MICROS;
        if (nextControlLoopMicros <= nowMicros) {
            nextControlLoopMicros = nowMicros + CONTROL_LOOP_INTERVAL_MICROS;
        }

        if (loopCounter == 0) {
            Serial.println("Starting calibration...");
            mpu.beginCalibration();
        }
        else if (loopCounter == numCalCount) {
            Serial.println("Calibration complete.");
            mpu.endCalibration();
        }

        // Read sensor data
        // const auto mpuData = mpu6050.read();
        // Serial.printf("%f,%f,%f\n", mpuData.accelX, mpuData.accelY, mpuData.accelZ);
        // Serial.printf("%f,%f,%f\n", mpuData.gyroX, mpuData.gyroY, mpuData.gyroZ);
        // Serial.println(dtMicros);
        mpu.update();
        const float pi = std::acos(-1.0f);
        const auto orientation = mpu.getOrientation();
        const auto euler = orientation.toEulerAngles();
        Serial.printf("%.3f,%.3f,%.3f\n", euler.x * 180.0f / pi, euler.y * 180.0f / pi, euler.z * 180.0f / pi);
        
        loopCounter++;
        lastControlLoopMicros = nowMicros;
    }

}
