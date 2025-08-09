#include <WiFi.h>
#include <ESPmDNS.h>

#include "Config.h"
#include "OTA.h"
#include "MPU6050.h"

const char *hostName = "FlightController";
const char *serialNumber = "0000";

#define CONTROL_LOOP_HZ 10
#define CONTROL_LOOP_INTERVAL_MICROS (1000000 / CONTROL_LOOP_HZ)

MPU6050 mpu6050;

unsigned long lastControlLoopMicros = 0;

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
    mpu6050.begin();

    lastControlLoopMicros = micros();
}


void loop() {
    otaLoop();
    unsigned long now = micros();
    if (now - lastControlLoopMicros >= CONTROL_LOOP_INTERVAL_MICROS) {
        lastControlLoopMicros = now;

        // Read sensor data
        const auto mpuData = mpu6050.readData();
        // Serial.printf("%f,%f,%f\n", mpuData.accelX, mpuData.accelY, mpuData.accelZ);
        Serial.printf("%f,%f,%f\n", mpuData.gyroX, mpuData.gyroY, mpuData.gyroZ);
    }
}
