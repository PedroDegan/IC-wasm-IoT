# 🚀 Research Roadmap — WebAssembly Across the Computational Continuum (IoTinuum)

This project investigates WebAssembly (WASM) as a **unified execution layer across the entire Computational Continuum (Edge–Fog–Cloud)**.

Instead of restricting WASM to embedded devices, this research evaluates the feasibility, portability, performance, and security implications of deploying the same WebAssembly modules across heterogeneous nodes.

---

# 🎯 Research Hypothesis

WebAssembly can act as a lightweight, secure, and portable execution layer capable of unifying distributed IoT application logic across Edge, Fog, and Cloud environments.

---

# 🎯 Research Objectives

## General Objective

Investigate the use of WebAssembly as a portable distributed execution model for smart irrigation systems deployed across the Computational Continuum (IoTinuum).

## Specific Objectives

- Deploy identical WASM modules across heterogeneous nodes
- Evaluate WASM portability via WASI
- Compare distributed execution strategies
- Analyze performance, memory, energy, and security trade-offs
- Compare C and Rust WASM implementations
- Provide reproducible experimental results

---

# 📚 Phase 0 — Research & Conceptual Foundation

- [X] Study IoT and the Computational Continuum (IoTinuum)
- [X] Analyze smart irrigation system requirements
- [X] Study WebAssembly architecture and sandbox model
- [X] Study WASI portability layer
- [ ] Compare WASM vs container-based virtualization
- [ ] Define system and threat model

---

# 🧩 Phase 1 — WASM Execution Layer on Edge (ESP32)

- [x] Integrate WAMR into ESP-IDF
- [x] Compile freestanding WASM module (C)
- [x] Register Host Functions
- [x] Execute sandboxed irrigation logic
- [ ] Measure memory footprint
- [ ] Measure execution latency
- [ ] Evaluate sandbox overhead

---

# 🌫️ Phase 2 — WASM Execution on Fog (Raspberry Pi)

- [X] Deploy WASM irrigation module on Raspberry Pi
- [X] Implement host runtime for Fog environment
- [X] Execute filtering (EMA) and anomaly detection inside WASM
- [ ] Compare native vs WASM execution
- [ ] Measure latency and memory usage
- [ ] Analyze portability constraints

---

# ☁️ Phase 3 — WASM Execution on Cloud / Server

- [X] Deploy identical WASM module on server environment
- [X] Implement WASI-based runtime (e.g., Wasmtime)
- [X] Execute cloud-side logic within WASM sandbox
- [ ] Compare native vs WASM server execution
- [ ] Evaluate scalability implications

---

# 🔄 Phase 4 — Distributed Communication Layer

- [X] Implement MQTT client on Edge
- [X] Define structured topic architecture
- [X] Transmit data between WASM-enabled nodes
- [X] Evaluate QoS reliability
- [ ] Measure end-to-end pipeline latency
- [ ] Analyze bandwidth consumption

---

# 🔒 Phase 5 — Security & Isolation Analysis

- [ ] Evaluate Host–Guest isolation guarantees
- [ ] Analyze sandboxing across all nodes
- [ ] Enable MQTT over TLS
- [ ] Compare WASM isolation vs container isolation
- [ ] Document attack surface across continuum

---

# 🌍 Phase 6 — Deployment Strategy Comparison

Evaluate different WASM placement strategies across the continuum:

- [ ] Strategy A: WASM only on Edge
- [ ] Strategy B: WASM on Edge + Fog
- [ ] Strategy C: WASM on Edge + Fog + Cloud (Full Continuum)
- [ ] Strategy D: Native Fog/Cloud + WASM Edge
- [ ] Strategy E: Fully native baseline

For each strategy:

- [ ] Measure end-to-end latency
- [ ] Measure memory usage per node
- [ ] Measure energy consumption (Edge)
- [ ] Measure processing distribution efficiency
- [ ] Analyze fault tolerance behavior

---

# 🦀 Phase 7 — Rust WASM Comparative Experiment

- [ ] Compile Rust to `wasm32-unknown-unknown`
- [ ] Deploy Rust WASM across Edge, Fog, Cloud
- [ ] Compare binary size (C vs Rust)
- [ ] Compare memory footprint
- [ ] Compare performance impact
- [ ] Evaluate safety and maintainability

---

# 📈 Phase 8 — Experimental Evaluation

- [ ] Define evaluation metrics:
  - Latency
  - Memory usage
  - Energy consumption
  - Reliability
  - Security overhead
- [ ] Benchmark full distributed pipeline
- [ ] Analyze sandbox overhead across environments
- [ ] Evaluate portability limits of WASI
- [ ] Perform qualitative and quantitative analysis
- [ ] Document experimental methodology
- [ ] Analyze synchronization overhead between network threads (MQTT) and WASM runtimes.
- [ ] Evaluate data consistency in multi-threaded host environments.

---

# 🔁 Phase 9 — Reproducibility & Open Science

- [ ] Publish source code
- [ ] Provide deployment scripts
- [ ] Document hardware configurations
- [ ] Provide replication guide
- [ ] Release experimental datasets
- [ ] Provide architecture diagrams

---

# 📊 Phase 10 — Research Output

- [ ] Write IC technical reports
- [ ] Draft conference paper
- [ ] Prepare academic presentation
- [ ] Prepare public demonstration
- [ ] Document lessons learned

---

# 🌟 Long-Term Vision

- Establish WebAssembly as a unified execution layer for the Computational Continuum
- Provide a reference architecture for WASM-based IoT systems
- Contribute to research on lightweight embedded virtualization
- Enable secure, portable distributed execution without heavy containerization
