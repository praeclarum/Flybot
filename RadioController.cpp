#include "HardwareSerial.h"
#include <Arduino.h>

HardwareSerial *serial = 0;

static uint8_t packet[25] = {0};
static uint8_t packetLen = 0;

static void parsePacket() {
    bool failSafe = (packet[23] & 0x08) != 0; // True when no RC signal
    bool hasSignal = !failSafe;
    Serial.printf("RC: Got valid packet: hasSignal: %s\n", hasSignal ? "true" : "false");
}

void rcBegin() {
    serial = &Serial1;
    serial->begin(100000, SERIAL_8E2, 16, 17, false);
}

void rcUpdate() {
    while (serial->available()) {
        const auto c = serial->read();
        if (packetLen == 0) {
            if (c == 0x0F) {
                packet[0] = 0x0F;
                packetLen = 1;
            }
        }
        else {
            packet[packetLen] = c;
            packetLen++;
            if (packetLen >= 25) {
                if (c == 0x00) {
                    parsePacket();
                }
                packetLen = 0;
            }
        }
    }
}
