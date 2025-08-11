#include <Arduino.h>

#include "RadioController.h"
#include "State.h"
#include "ConfigValue.h"
#include "Geometry.h"

HardwareSerial *serial = 0;

static uint8_t packet[25] = {0};
static uint8_t packetLen = 0;

ConfigValue rcPitchMaxDegrees("rc.pitch.max", "Maximum pitch angle (degrees)", Value::fromFloat(60.0f));
ConfigValue rcRollMaxDegrees("rc.roll.max", "Maximum roll angle (degrees)", Value::fromFloat(60.0f));

static void parsePacket() {
    bool failSafe = (packet[23] & 0x08) != 0; // True when no RC signal
    bool hasSignal = !failSafe;
    const auto ch1 = ((packet[2] & 0b00000111) << 8) | packet[1];
    const auto ch2 = ((packet[3] & 0b00111111) << 5) | (packet[2] >> 3);
    const auto ch3 = ((packet[5] & 0b00000001) << 10) | (packet[4] << 2) | (packet[3] >> 6);
    const auto ch4 = ((packet[6] & 0b00001111) << 7) | (packet[5] >> 1);
    // InMin = 172, InMax = 1811
    // OutMin = 0.0, OutMax = 1.0
    // o = m*i + b
    // 0.0 = m*172 + b
    const float m = (1.0f) / (1811.0f - 172.0f);
    const float b = -m * 172.0f;
    const float ch1f = (float)ch1 * m + b;
    const float ch2f = (float)ch2 * m + b;
    const float ch3f = (float)ch3 * m + b;
    const float ch4f = (float)ch4 * m + b;
    if (false) {
        for (size_t i=1; i<=11; i++) {
            for (int b=7; b >= 0; b--) {
                const auto v = (packet[i] >> b) & 0x1;
                Serial.printf("%d", v);
            }
            Serial.printf(" ");
        }
        Serial.println();
    } else {
        // Serial.printf("RC: Got valid packet: hasSignal: %s, ch1: %4d, ch2: %4d, ch3: %4d, ch4: %4d\n",
        //     hasSignal ? "true" : "false", ch1, ch2, ch3, ch4);
        // Serial.printf("RC: Got valid packet: hasSignal: %s, ch1: %.3f, ch2: %.3f, ch3: %.3f, ch4: %.3f\n",
        //     hasSignal ? "true" : "false", ch1f, ch2f, ch3f, ch4f);
    }
    const float thr = ch3f;
    const float yaw = ch4f;
    const float pitchDegrees = (ch2f * 2.0f - 1.0f) * rcPitchMaxDegrees.getFloat();
    const float rollDegrees = (ch1f * 2.0f - 1.0f) * rcRollMaxDegrees.getFloat();
    stateUpdateRC(
        pitchDegrees * DEG_TO_RAD_F,
        rollDegrees * DEG_TO_RAD_F,
        yaw,
        thr,
        hasSignal);
}

void rcBegin() {
    serial = &Serial2;
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
                    packetLen = 0;
                }
                else {
                    // Invalid packet, look for a new start byte within the packet
                    bool foundStartByte = false;
                    for (size_t i = 1; i < packetLen; i++) {
                        if (packet[i] == 0x0F) {
                            // Found a new start byte, shift the packet
                            for (size_t j = 0; j < packetLen - i; j++)
                                packet[j] = packet[i + j];
                            packetLen -= i;
                            foundStartByte = true;
                            break;
                        }
                    }
                    if (!foundStartByte) {
                        // No new start byte found, reset the packet
                        packetLen = 0;
                    }
                }
            }
        }
    }
}
