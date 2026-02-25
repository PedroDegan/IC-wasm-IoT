# WebAssembly (WASM) on ESP32 with WAMR / ESP-IDF  
## Edge–Fog–Cloud Architecture with MQTT

![ESP32](https://img.shields.io/badge/ESP32-IoT-red)
![WASM](https://img.shields.io/badge/WebAssembly-WASM-blue)
![WAMR](https://img.shields.io/badge/WAMR-2.4.0-green)
![ESP-IDF](https://img.shields.io/badge/ESP--IDF-5.5.1-orange)
![MQTT](https://img.shields.io/badge/MQTT-Protocol-purple)
![Research](https://img.shields.io/badge/Research-Embedded%20Systems-black)

---

## 📌 Project Overview

This project explores secure execution of **WebAssembly (WASM)** modules on an **ESP32**, integrated with **WAMR** and **ESP-IDF**, within a distributed **Edge–Fog–Cloud architecture**.

The system is structured as:

ESP32 (Edge) → MQTT → Raspberry Pi (Fog Node) → Laptop Server (Cloud Layer)

The goal is to:

- Isolate business logic using WASM sandboxing
- Enable OTA updates via dynamic `.wasm` replacement
- Implement distributed data processing
- Validate computational continuum concepts

This repository is part of a **Scientific Initiation research project (UFABC)** focused on:

- Secure embedded execution
- Embedded virtualization
- Computational continuum (cloud–fog–edge)
- Safe OTA architectures

---

## 🧱 System Architecture

### 🔵 Edge Layer — ESP32 (WASM Execution)

- Executes Guest logic inside WAMR sandbox
- Reads soil moisture sensor
- Controls irrigation hardware
- Publishes sensor data via MQTT
- Hardware abstraction through Host Functions

### 🟣 Fog Layer — Raspberry Pi

- MQTT subscriber (broker client)
- Applies filtering and temporal validation
- Performs smoothing (EMA / moving average)
- Aggregates sensor data
- Forwards processed data to central server

### 🟢 Cloud Layer — Laptop Server

- Receives filtered data from Raspberry Pi
- Acts as monitoring server
- Stores historical data
- Enables visualization and analysis
- Potential integration with dashboards

---

## 🔄 Data Flow

1. ESP32 reads sensor
2. WASM Guest applies irrigation logic
3. ESP32 publishes data via MQTT
4. Raspberry Pi subscribes to topic
5. Raspberry applies filtering & validation
6. Raspberry forwards structured data to Laptop server
7. Laptop stores and visualizes results

---

## ⚙️ Technologies Used

### Edge
- ESP32
- ESP-IDF
- WAMR
- LLVM/Clang (wasm32 target)
- MQTT client

### Fog
- Raspberry Pi
- Python MQTT subscriber
- Data filtering algorithms
- TCP/HTTP forwarding

### Cloud
- Laptop server (Python-based)
- Data logging
- Future dashboard integration

---

## 🔐 Security Model

- WASM sandbox enforces memory isolation on ESP32
- Hardware access only through validated Host Functions
- MQTT communication can be extended to TLS
- Data validation performed at Fog layer

---

## 🌱 Current Implementation

- WASM execution on ESP32 validated
- Smart irrigation prototype functional
- MQTT communication under integration
- Raspberry Pi filtering layer under development
- Laptop server receiving structured data

---

## 🔬 Research Contributions

This project demonstrates:

- Viability of WASM in constrained microcontrollers
- Safe hardware abstraction via Host–Guest separation
- Distributed edge–fog–cloud processing
- OTA-friendly embedded behavior updates
- Layered filtering architecture

---

## 🚀 Next Steps

- Finalize MQTT pipeline
- Implement filtering algorithms on Raspberry Pi
- Develop structured JSON data protocol
- Add TLS to MQTT communication
- Benchmark latency across layers
- Implement OTA `.wasm` replacement

---

## 🧑‍💻 Author

Pedro Henrique Silva Degan  
Scientific Initiation — UFABC  
Embedded Systems & Secure Execution Research

---

⭐ If this project contributes to your research, consider starring it.
