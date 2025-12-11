# WebAssembly (WASM) on ESP32 with WAMR / ESP-IDF

## 📌 Project Overview

This project integrates and executes **WebAssembly (WASM)** modules on an **ESP32**, using the **WebAssembly Micro Runtime (WAMR)** within the **ESP-IDF/FreeRTOS** environment. The goal is to isolate the business logic (Guest) from the firmware (Host), improving security, portability, and enabling OTA updates that replace only the `.wasm` module.

This repository is part of a **Scientific Initiation project**, focused on computational continuum and safe execution of code in embedded devices.

The project was initially created from the official Espressif example using the command:

```bash
idf.py create-project-from-example "espressif/wasm-micro-runtime=2.4.0~1:esp-idf"
```

The implementation uses:

* **ESP-IDF v5.5.1**
* **WAMR v2.4.0**

---

## 🎯 Project Goals

* **Sandboxing & Isolation:** Guest code cannot compromise the Host firmware.
* **Portability:** WASM binaries are CPU‑agnostic and not tied to the Xtensa architecture.
* **OTA-Friendly Architecture:** Only the `.wasm` module needs updating—no reflashing.
* **Clean Separation:** The Host manages hardware; the Guest requests actions through Host Functions.

---

## 🧱 Architecture Overview

### 🟦 Guest (WASM Module)

* Written in C.
* Compiled using LLVM/Clang for `wasm32-unknown-unknown-wasm`.
* Freestanding environment (no libc, no crt0, no syscalls).
* Uses imported Host Functions to interact with hardware.

### 🟧 Host (ESP-IDF + WAMR)

* Loads, validates, and runs the WASM module.
* Exposes hardware access APIs (sensor read, delay, random generation, etc.).
* Ensures memory isolation between Host and Guest.

---

## 🛠️ WASM Freestanding Toolchain

The Guest code is compiled using **LLVM/Clang** without standard libraries.

### 🔹 Compilation (produces test.o)

```bash
clang --target=wasm32-unknown-unknown-wasm \
      -nostdinc -nostdlib -ffreestanding \
      -O3 \
      -c test.c -o test.o
```

### 🔹 Linking (produces test.wasm)

```bash
clang --target=wasm32-unknown-unknown-wasm \
      -nostdlib \
      -nostartfiles \
      -Wl,--no-entry \
      -Wl,--allow-undefined \
      -Wl,--export=main \
      test.o -o test.wasm
```

### ℹ️ Key Flags Explained

* `-ffreestanding`: removes libc assumptions.
* `-nostdlib -nostartfiles`: prevents search for startup files (crt1.o) and libc.
* `--allow-undefined`: enables Guest code to call Host Functions.
* `--export=main`: exposes the entry point for WAMR.

---

## 📄 Example Guest Code (test.c)

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

        printf("WASM: Fixed Reading: %d | Random: %d
", leitura_fixa, valor_random);
        delay_ms(delay);
    }
    return 0;
}
```

---

## 🔗 Host Function Registration (ESP-IDF Host Side)

```c
static NativeSymbol extended_native_symbols[] = {
    {"sensor_read", (void*)sensor_read, "()i"},
    {"delay_ms",    (void*)wasm_delay,  "(i)"},
    {"get_random",  (void*)wasm_rand,   "()i"}
};
```

---

## 🐞 Troubleshooting — Errors & Solutions

### ❌ **1. Linker searching for crt1.o / libc**

```
wasm-ld: error: cannot open crt1.o
```

✔ Add `-nostdlib -nostartfiles`.

### ❌ **2. WAMR could not find main()**

```
lookup the entry point symbol failed
```

✔ Add `-Wl,--export=main`.

### ❌ **3. Missing extern declarations**

```
warning: implicit declaration of function 'printf'
```

✔ Declare all Host Functions explicitly with `extern`.

### ❌ **4. Incorrect Host Function signatures**

```
failed to check signature '(i)' for import
```

✔ Ensure signatures match exactly (e.g., `()i`, `(i)`).

### ❌ **5. WAMR memory allocation failure**

```
WASM module instantiate failed: allocate linear memory failed
```

✔ Reduce WASM module stack/heap (e.g., 8 KB + 8 KB).

---

## 📦 Integration Workflow

1. Compile Guest → `test.wasm`
2. Convert to C header:

```bash
xxd -i test.wasm > test_wasm.h
```

3. Register Host Functions.
4. Load and execute the WASM module safely under WAMR.

This enables:

* isolated business logic execution;
* OTA updates by swapping the `.wasm` file;
* hardware access only through validated Host APIs.

---

## 🚀 Next Steps in the Research Project

* Advanced WASM security model on embedded devices.
* Full OTA pipeline for module replacement.
* Deploying WASM across cloud–fog–edge layers.
* Benchmarking performance & memory usage.

---

## 📚 References

* WebAssembly Micro Runtime (WAMR)
* LLVM/Clang
* ESP-IDF / FreeRTOS

---

## 🧑‍💻 Author

**Pedro Henrique Silva Degan**
Scientific Initiation Project — UFABC

---

If this repository was helpful, ⭐ star it!
