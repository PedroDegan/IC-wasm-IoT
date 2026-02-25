# 🚀 Project Roadmap — Distributed WASM Edge Architecture

This roadmap defines the structured evolution of a distributed secure embedded execution system.

---

## ✅ Phase 1 — Edge WASM Foundation (Completed)

- [x] Integrate WAMR into ESP-IDF
- [x] Compile freestanding WASM module
- [x] Register Host Functions
- [x] Execute sandboxed irrigation logic
- [x] Validate hardware interaction

---

## 🔄 Phase 2 — MQTT Communication Layer

- [ ] Implement MQTT client on ESP32
- [ ] Publish structured JSON sensor data
- [ ] Define topic architecture
- [ ] Validate message reliability
- [ ] Measure transmission latency

---

## 🟣 Phase 3 — Fog Node (Raspberry Pi)

- [ ] Implement MQTT subscriber (Python)
- [ ] Apply temporal filtering
- [ ] Implement EMA / moving average smoothing
- [ ] Detect anomalous sensor readings
- [ ] Forward processed data to server

---

## 🟢 Phase 4 — Cloud / Laptop Server

- [ ] Implement TCP or HTTP ingestion server
- [ ] Store historical sensor data
- [ ] Add structured logging
- [ ] Build visualization endpoint
- [ ] Design monitoring dashboard prototype

---

## 🔒 Phase 5 — Security Enhancements

- [ ] Enable MQTT over TLS
- [ ] Validate integrity of transmitted data
- [ ] Analyze attack surface
- [ ] Formalize Host–Guest isolation guarantees

---

## 🌍 Phase 6 — Computational Continuum Validation

- [ ] Deploy identical logic across:
  - ESP32 (Edge)
  - Raspberry Pi (Fog)
  - Laptop (Cloud)
- [ ] Compare execution models
- [ ] Measure energy consumption
- [ ] Analyze distributed processing trade-offs

---

## 🦀 Phase 7 — Rust WASM Experiment

- [ ] Compile Rust to `wasm32-unknown-unknown`
- [ ] Compare memory footprint vs C
- [ ] Evaluate safety guarantees
- [ ] Benchmark performance impact

---

## 📊 Phase 8 — Research Output

- [ ] Benchmark full pipeline latency
- [ ] Produce architecture diagrams
- [ ] Write conference paper draft
- [ ] Document reproducibility steps
- [ ] Prepare academic presentation

---

# 🎯 Long-Term Vision

Transform this into:

- A secure distributed embedded execution framework
- A reference architecture for edge–fog–cloud WASM systems
- A research foundation for embedded virtualization
