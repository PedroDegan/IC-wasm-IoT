# WebAssembly Across the Computational Continuum  
## Distributed WASM Execution on ESP32, Raspberry Pi, and Server

![ESP32](https://img.shields.io/badge/ESP32-IoT-red)
![WebAssembly](https://img.shields.io/badge/WebAssembly-WASM-blue)
![WAMR](https://img.shields.io/badge/WAMR-2.4.0-green)
![WASI](https://img.shields.io/badge/WASI-Portability-orange)
![MQTT](https://img.shields.io/badge/MQTT-Protocol-purple)
![Research](https://img.shields.io/badge/Research-Computational%20Continuum-black)

---

# 📌 Project Overview

This project investigates **WebAssembly (WASM) as a unified execution layer across the Computational Continuum (Edge–Fog–Cloud)**.

Instead of restricting WASM to embedded devices, this research evaluates the feasibility of deploying **the same WASM modules across heterogeneous nodes**, enabling portable, sandboxed, and OTA-friendly distributed IoT applications.

The system spans:

ESP32 (Edge) → Raspberry Pi (Fog) → Laptop Server (Cloud)

All layers are designed to support **WASM-based execution**, enabling comparative evaluation of different deployment strategies.

This repository is part of a **Scientific Initiation research project (UFABC)** focused on:

- Secure embedded execution
- Lightweight virtualization
- WebAssembly portability (WASI)
- Distributed IoT systems
- Computational continuum (IoTinuum)

---

# 🎯 Research Hypothesis

WebAssembly can act as a lightweight, secure, and portable execution layer capable of unifying distributed IoT application logic across Edge, Fog, and Cloud environments.

---

# 🧱 System Architecture

## 🔵 Edge Layer — ESP32 (WASM Runtime via WAMR)

- Executes irrigation logic inside WAMR sandbox
- Reads soil moisture sensor
- Controls irrigation hardware
- Publishes sensor data via MQTT
- Hardware abstraction through validated Host Functions
- Enables OTA replacement of `.wasm` modules

---

## 🟣 Fog Layer — Raspberry Pi (WASM-Enabled Processing)

- Orchestrates MQTT communication and acts as the central message broker between Edge and Cloud
- MQTT subscriber
- Applies temporal filtering and anomaly detection
- Performs smoothing (EMA / moving average)
- Aggregates sensor data
- Forwards structured data to Cloud layer
- Comparative evaluation: Native vs WASM execution

---

## 🟢 Cloud Layer — Server (WASM + Native Comparison)

- Executes real-time analytics and digital twin logic using Wasmtime (Python-based host). Implements dual-stream processing: Raw Data vs. WASM-Filtered Data
- Receives filtered data from Fog node
- Stores historical data
- Provides visualization endpoints
- Enables deployment strategy comparison:
  - Native server logic
  - WASM-based execution
  - Hybrid strategies

---

# 🔄 Data Flow

1. ESP32 reads sensor
2. WASM Guest applies irrigation logic
3. ESP32 publishes data via MQTT
4. Fog node processes data (WASM or native)
5. Fog forwards processed data
6. Cloud executes analytics (WASM or native)
7. Results are stored and visualized

---

# ✨ Key Features Demonstrated

- **True Cross-Architecture Portability:** The exact same `.wasm` binary, compiled from C, executes seamlessly on **Xtensa (ESP32)** and **x86_64 (Linux/Windows)** without a single line of code change or recompilation.
- **Remote Dynamic Calibration:** Implemented a bi-directional control loop where logic thresholds (e.g., irrigation triggers) are updated via MQTT and applied instantly to the WASM Guest memory without rebooting the node.
- **Hybrid Execution Logic:** Demonstration of a "Digital Twin" approach, where the same filtering algorithm runs on the Edge (for immediate action) and in the Cloud (for high-level analytics and UI visualization).
- **Thread-Safe Data Bridging:** A robust synchronization strategy using file-buffered I/O to bridge asynchronous MQTT network threads with the Streamlit reactive UI.

---

# 🧪 Experimental Focus

The project evaluates multiple deployment strategies:

- WASM only on Edge
- WASM on Edge + Fog
- WASM on Edge + Fog + Cloud
- Native Fog/Cloud + WASM Edge
- Fully native baseline

For each configuration, the following metrics are analyzed:

- End-to-end latency
- Memory usage per node
- Energy consumption (Edge)
- Sandbox overhead
- Portability constraints
- Security isolation guarantees

---

# ⚙️ Technologies Used

## Edge
- ESP32
- ESP-IDF
- WAMR
- LLVM/Clang (`wasm32` target)
- MQTT client

## Fog
- Raspberry Pi
- WASM runtime (planned: Wasmtime / WAMR)
- Python-based orchestration
- MQTT subscriber

## Cloud
- WASI-compatible runtime
- Python server
- Data logging and visualization

---

# 🔐 Security Model

- WASM sandbox enforces memory isolation
- Hardware access only through validated Host Functions
- MQTT communication extensible to TLS
- Isolation compared against container-based approaches
- Attack surface evaluated across continuum

---

# 🌱 Current Implementation Status

- [x] **WASM Edge Execution:** Irrigation logic running on ESP32 via WAMR.
- [x] **Cross-Continuum Portability:** Same `.wasm` binary executing on both ESP32 (Xtensa) and Laptop (x86_64).
- [x] **Bi-Directional MQTT Pipeline:** - `ic/esp32/data`: Telemetry from Edge to Cloud.
    - `ic/esp32/config`: Remote calibration from Cloud to Edge.
- [x] **Cloud Analytics Node:** High-level dashboard processing raw data via WASM filters in real-time.
- [x] **Concurrency Solution:** Implemented file-buffered data bridge to ensure thread-safe UI updates.

---

# 🔬 Research Contributions

This project aims to demonstrate:

- Feasibility of WASM on constrained microcontrollers
- Portability of identical WASM modules across heterogeneous nodes
- Secure Host–Guest isolation in embedded systems
- Lightweight alternative to container-based virtualization
- Experimental evaluation of WASM across the Computational Continuum
- Comparative analysis of distributed execution strategies

---

# 🚀 Next Steps

- Implement runtime comparison (Native vs WASM)
- Finalize MQTT structured protocol
- Enable TLS-secured communication
- Benchmark latency across deployment strategies
- Measure energy consumption on Edge
- Perform C vs Rust WASM comparison
- Document reproducible experiments

---

# 🧑‍💻 Author

Pedro Henrique Silva Degan  
Scientific Initiation — UFABC  
Embedded Systems & Distributed Execution Research

---

⭐ If this project contributes to your research, consider starring it.
