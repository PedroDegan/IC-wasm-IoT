# WebAssembly (WASM) on ESP32 with WAMR / ESP-IDF

![ESP32](https://img.shields.io/badge/ESP32-IoT-red)
![WASM](https://img.shields.io/badge/WebAssembly-WASM-blue)
![WAMR](https://img.shields.io/badge/WAMR-2.4.0-green)
![ESP-IDF](https://img.shields.io/badge/ESP--IDF-5.5.1-orange)
![Research](https://img.shields.io/badge/Research-Embedded%20Systems-purple)

---

## 📌 Project Overview

This project demonstrates the execution of **WebAssembly (WASM)** modules on an **ESP32** microcontroller using **WebAssembly Micro Runtime (WAMR)** integrated with **ESP-IDF / FreeRTOS**.

The core idea is to separate:

- 🔵 **Host (Firmware / Hardware Control)**
- 🟠 **Guest (Business Logic in WASM)**

This architecture enables:

- Memory isolation (sandboxing)
- OTA updates replacing only the `.wasm` module
- Hardware abstraction
- CPU-agnostic logic execution
- Safer embedded software architectures

This repository is part of a **Scientific Initiation research project (UFABC)** focused on:

- Computational continuum (cloud–fog–edge)
- Secure execution models
- Embedded virtualization strategies

---

## 🎯 Why WebAssembly on Microcontrollers?

Traditional embedded systems tightly couple firmware and logic.

With WASM:

- Logic becomes portable
- Firmware remains stable
- OTA becomes lightweight
- Risk surface is reduced
- Cloud/Edge logic reuse becomes feasible

This project validates that **WASM is viable for real-time sensor-driven control loops on constrained devices**.

---

## 🧱 System Architecture

### 🔵 Host (ESP-IDF + WAMR)

- Runs on ESP32 (Xtensa architecture)
- Loads and validates `.wasm` module
- Registers Host Functions
- Controls hardware (ADC, GPIO, relay, pump)
- Enforces memory isolation

### 🟠 Guest (WASM Module)

- Written in C
- Compiled for `wasm32-unknown-unknown-wasm`
- Freestanding (no libc, no syscalls)
- Calls hardware indirectly via imported Host Functions

---

## 🛠️ Toolchain & Versions

- **ESP-IDF v5.5.1**
- **WAMR v2.4.0**
- **LLVM/Clang (wasm32 target)**

Project originally created from:

```bash
idf.py create-project-from-example "espressif/wasm-micro-runtime=2.4.0~1:esp-idf"
```

---

## ⚙️ Building the WASM Guest (Freestanding)

### 🔹 Step 1 — Compile

```bash
clang --target=wasm32-unknown-unknown-wasm \
      -nostdinc -nostdlib -ffreestanding \
      -O3 \
      -c test.c -o test.o
```

### 🔹 Step 2 — Link

```bash
clang --target=wasm32-unknown-unknown-wasm \
      -nostdlib \
      -nostartfiles \
      -Wl,--no-entry \
      -Wl,--allow-undefined \
      -Wl,--export=main \
      test.o -o test.wasm
```

### 🔎 Important Flags

- `-ffreestanding` → removes libc assumptions
- `-nostdlib -nostartfiles` → avoids crt1.o and libc
- `--allow-undefined` → allows Host Function imports
- `--export=main` → exposes entrypoint to WAMR

---

## 📄 Example Guest Code

```c
extern int printf(const char *format, ...);
extern int sensor_read(void);
extern void delay_ms(int ms);
extern int get_random(void);

int main() {
    int delay = 5000;

    while (1) {
        int leitura_fixa = sensor_read();
        int valor_random = get_random() & 0xFFF;

        printf("WASM: Fixed Reading: %d | Random: %d\n",
               leitura_fixa,
               valor_random);

        delay_ms(delay);
    }

    return 0;
}
```

---

## 🔗 Host Function Registration

```c
static NativeSymbol extended_native_symbols[] = {
    {"sensor_read", (void*)sensor_read, "()i"},
    {"delay_ms",    (void*)wasm_delay,  "(i)"},
    {"get_random",  (void*)wasm_rand,   "()i"}
};
```

Signature format examples:

- `()i` → returns int
- `(i)` → receives int, returns void

---

## 🌱 Current Implementation — Smart Irrigation Prototype

This stage implements a **real smart irrigation system** using:

### 🔧 Hardware

- ESP32
- Capacitive Soil Moisture Sensor (analog)
- 2-channel 5V Relay Module
- Mini Submersible Pump (2.5–6V, ~200mA)
- Status LEDs (Red / Green / Blue)

---

### 🧠 Software Flow

**Host (ESP-IDF)**

- Configures ADC (oneshot mode)
- Reads soil moisture
- Exposes:
  - `read_sensor()`
  - `set_output()`
  - `delay_ms()`
  - `wasm_log()`
- Instantiates WAMR runtime
- Executes Guest logic in isolation

**Guest (WASM)**

- Applies irrigation threshold logic
- Controls pump via Host calls
- Runs periodic loop (low-frequency sampling)

---

## ⚠️ Hardware Lessons Learned

The relay module requires **5V logic**, while ESP32 outputs **3.3V**.

Observed issue:
- Relay LED activates weakly
- Relay does not switch reliably

Implication:
- Need logic-level relay module
- OR transistor/MOSFET driver stage
- OR opto-isolated interface

Alternative approach under evaluation:
- Logic-level N-MOSFET
- Dedicated external power rail

---

## 🐞 Troubleshooting

### Linker Error — crt1.o

```
wasm-ld: error: cannot open crt1.o
```

✔ Add `-nostdlib -nostartfiles`

---

### Entry Point Not Found

```
lookup the entry point symbol failed
```

✔ Add `-Wl,--export=main`

---

### Signature Mismatch

```
failed to check signature '(i)' for import
```

✔ Ensure exact signature match

---

### Memory Allocation Failure

```
WASM module instantiate failed
```

✔ Reduce WASM stack/heap allocation

---

## 📦 Integration Workflow

1. Compile Guest → `test.wasm`
2. Convert to header:

```bash
xxd -i test.wasm > test_wasm.h
```

3. Register native symbols
4. Instantiate WAMR
5. Execute safely under sandbox

---

## 🔬 Research Impact

This project validates:

- WASM feasibility in microcontrollers
- Safe execution of dynamic logic
- OTA replacement of behavior without reflashing firmware
- Hardware abstraction through controlled imports
- Foundations for cloud–edge logic portability

---

## 🚀 Next Research Steps

- Implement hysteresis and temporal filtering
- Apply EMA / moving average filtering
- Design safe MOSFET-based driver stage
- Develop real OTA pipeline for WASM swapping
- Benchmark latency, memory footprint and energy consumption
- Deploy same WASM logic across edge, fog and cloud

---

## 📚 References

- WebAssembly Micro Runtime (WAMR)
- LLVM / Clang
- ESP-IDF / FreeRTOS

---

## 🧑‍💻 Author

**Pedro Henrique Silva Degan**  
Scientific Initiation Project — UFABC  
Embedded Systems & Secure Execution Research

---

⭐ If this project contributes to your research, consider starring it.
