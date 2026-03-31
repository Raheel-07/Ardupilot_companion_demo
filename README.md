# ArduPilot Companion Health Demo

This repository contains the pre-GSoC 2025 implementation work for the "Real-Time Companion Computer Health Monitoring & Failsafe" project for ArduPilot. It serves as a proof-of-concept for a hardware-agnostic C++ framework designed to monitor companion computer health and communicate it directly to the flight controller.

## Features

* **System Metrics Daemon (`health_probe.cpp`)**: A working C++ probe that reads CPU percentage, memory percentage, and `systemd` service status directly from `/proc` and `sysfs`.
* **Bidirectional MAVLink Communication**: A C++ application that sends `HEARTBEAT` messages to an ArduPilot SITL instance at 1 Hz via a UDP socket.
* **Telemetry Parsing**: Successfully receives and parses live MAVLink telemetry back from the SITL, recognizing multiple message IDs.

## Environment & Prerequisites

This project was developed and verified in the following environment:
* Windows 11 using WSL2 (Ubuntu 22.04).
* ArduPilot SITL (ArduCopter) built from source.
* Mission Planner connected via UDP.
* `mavlink/c_library_v2` headers for C++ MAVLink integration.

## Core Components

### 1. Health Probe (`health_probe.cpp`)
The system monitor collects crucial metrics to determine the companion computer's operational health:
* **CPU Utilisation**: Calculated using a dual-sample delta method from `/proc/stat`.
* **Memory Usage**: Derived from the `MemAvailable` to `MemTotal` ratio in `/proc/meminfo`.
* **Service Status**: Checks if critical system services are active via `systemctl is-active` exit codes.

### 2. MAVLink Node Registration
The application successfully registers the companion process as a valid MAVLink node on the network:
* Broadcasts heartbeats using `SysID:42` and `CompID:191` (`MAV_COMP_ID_ONBOARD_COMPUTER`).
* Validates the bidirectional link by capturing and parsing real telemetry from the flight controller (`SysID:1`), including `SYS_STATUS` (MsgID:1), `ATTITUDE` (MsgID:30), `GLOBAL_POSITION_INT` (MsgID:33), and `VFR_HUD` (MsgID:74).
