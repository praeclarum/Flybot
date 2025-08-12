#include <cmath>

#include <WiFi.h>
#include <ESPmDNS.h>

#include "Config.h"
#include "OTA.h"
#include "MPU6050.h"
#include "Geometry.h"
#include "RadioController.h"

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
    MDNS.begin(hostName);
    MDNS.addService("http", "tcp", 80);

    otaSetup(hostName);

    Serial.println("================================");

    Wire.begin();
    mpu.begin();
    rcBegin();

    webServerBegin();
}

void loop() {
    otaLoop();
    rcUpdate();
    controlLoop(mpu);
}
