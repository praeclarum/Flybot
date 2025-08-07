#include <WiFi.h>
#include <ESPmDNS.h>

#include "Config.h"
#include "OTA.h"

const char *hostName = "FlightController";
const char *serialNumber = "0000";

Config config;

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
}

void loop() {
    otaLoop();
}
