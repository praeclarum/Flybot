# ESP32-Based Configurable Flight Controller – Functional Spec

## Overview

This project defines a simple but capable **flight controller written in C++ for ESP32**, with clear separation of real-time control loops from non-critical tasks (web UI, I2C polling, logging). The controller is designed to run reliably on bare-metal or FreeRTOS with hardware in the loop and configurable PID-based attitude control.

This system avoids traditional ground stations entirely. Instead, all configuration and telemetry are served via **onboard web interface** hosted on the ESP32 itself over Wi-Fi.

---

## Goals

- Minimal, reliable flight stack using modern embedded practices
- Real-time flight control loop with precise timing (100–400 Hz)
- Background tasks (I2C, SPI, telemetry, web UI) **must not interfere** with control loop
- Full configurability via onboard web UI over Wi-Fi
- Plug-and-play support for basic sensors (IMU, baro, GPS)
- Easily extendable to quad/hex/VTOL vehicles

---

## Target Hardware

- **MCU**: ESP32 (dual-core, FreeRTOS)
- **Sensors**:
  - MPU6050 / MPU9250 via I2C
  - BMP280 or MS5611 (barometer)
  - Optional GPS via UART (e.g. u-blox)
- **Motor Outputs**: PWM via LEDC
- **Input**: SBUS RC decoding
- **Config Interface**: HTTP UI hosted onboard
- **Airframe**: Quad, Hexa, or custom (configurable motor mixer)

---

## Architecture

### Core Loop (Real-Time Task, Pinned to Core 0)

- Frequency: 250Hz default (configurable)
- Reads IMU sensor data
- Runs complementary/MADGWICK filter for attitude estimation
- Applies PID controller per axis (roll, pitch, yaw)
- Writes PWM to ESCs
- Uses hardware timers, no `delay()` or dynamic memory

### I/O & System Tasks (Non-RT, Core 1)

- **Wi-Fi server + web UI**
  - Static HTML/CSS/JS files embedded in flash
  - REST endpoints for config/state
- **Sensor polling (I2C/SPI)**
  - With task messaging RT-safe IMU, and compass access
- **RC input decoding**
  - SBUS via UART

### Optional

- OTA firmware update over Wi-Fi
- Write flight data to SPIFFS or SD card
- Background log upload via HTTP or WebSocket

---

## Configuration System

- Config stored in flash (JSON or compact binary format)
- Parameters:
  - PID gains for roll/pitch/yaw (P, I, D)
  - IMU orientation/mount offsets
  - RC input mapping and reversal
  - Arming conditions and failsafes
  - Motor mixer table (quad/hex/etc)
  - Loop rate
- REST API to GET/PUT config
- Simple local UI (single HTML/JS page) for editing params

---

## Control Model

- **Input**: RC sticks (mapped to roll/pitch/yaw/throttle)
- **Output**: Motor signals based on mixer + PID
- **Arming**:
  - Manual switch or stick gesture
  - Optional pre-arm checks (RC present, sensor OK)
- **Failsafe**:
  - RC loss triggers disarm or RTL (if GPS available)
  - Optional altitude hold via barometer

---

## Safety Features

- Hard real-time loop isolated from Wi-Fi and sensor lag
- Watchdog on flight loop
- Optional hard motor cutoff pin
- Configurable throttle ramp / soft start
- Optional flight mode switch support (manual, stabilized)

---

## Web UI Features

- Connect to ESP32 AP (`FC-XXXX`)
- Access via `http://192.168.4.1/`
- Display:
  - RC input values
  - Current attitude (live)
  - Battery voltage (if sensor present)
  - Arming status
- Editable configuration UI (PID sliders, channel maps, etc.)
- Save + reboot system
- Download logs

---

## Out of Scope (for V1)

- Autonomous missions
- Waypoints or complex GPS modes
- Computer vision or external SLAM
- Companion computer integration
- Mobile apps (web only)

---

## Future Enhancements

- Add magnetometer for heading hold
- A barometer for altitude hold
- Add GPS-based position hold
- Add basic RTL (Return-to-Launch)
- Implement scripting for flight behaviors
- Add software-in-loop simulator connector

---

## Your Strengths Mapped

- You are comfortable with:
  - Real-time system design (FreeRTOS, task separation)
  - C++ on embedded hardware
  - Sensor interfacing, motor control
  - Networking, HTTP/REST design
- So this project avoids:
  - Bloated ground stations
  - Deprecated legacy firmware
  - GUI toolchains that abstract away control

---

## Key Design Values

- Deterministic and readable C++
- Transparent timing model
- Minimal runtime dynamic allocation
- Fully testable flight loop in isolation
- Honest watchdog and failure handling
- Onboard config, not toolchain dependency
