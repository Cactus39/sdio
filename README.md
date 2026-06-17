# RP2040 SDIO USB Mass Storage Card Reader (readonly)

A bare-metal C implementation of an SDIO SD card reader utilizing the **RP2040 (Raspberry Pi Pico)**. This project leverages the RP2040's unique **PIO (Programmable I/O)** state machines to handle the low-level, high-speed SDIO protocol natively, exposing the card as a standard USB Mass Storage device via **TinyUSB**.

---

## 🚀 Key Technical Features

* **Custom SDIO Driver via PIO:** Bypasses slower SPI modes by implementing native SDIO command and data transport layers using RP2040 PIO state machines (`cmd_asm.pio` and `dat_asm.pio`).
* **USB Mass Storage Class (MSC):** Integrates with the TinyUSB stack to enumerate the RP2040 as a standard USB flash drive.

---

## 🛠️ Hardware Stack

* **Microcontroller:** Raspberry Pi Pico (RP2040)
* **Storage Interface:** Micro SD Card Slot (wired for SDIO/1-bit data bus and open drain cmd line)
* **Debugger/Console:** Raspberry Pi Debug Probe (CMSIS-DAP + UART Bridge on pins gp0 and gp1)

---

## 💻 Software Prerequisites

Ensure you have the Raspberry Pi Pico SDK and ARM GCC toolchain installed. The project expects environment variables to point to your toolchain path, or it will default to the standard paths:
* Pico SDK v2.2.0
* ARM GNU Toolchain (GCC 14.2)

---

## 🔧 Building and Flashing

Management of the project is unified under a single `Makefile` for simplicity and speed.

### 1. Compile the Project
To generate the PIO headers, configure CMake, and build the binaries in parallel, run:
```
bash make
```
### 2. Flash via Debug Probe

To instantly flash the compiled .elf binary into the RP2040 using the Raspberry Pi Debug Probe (via OpenOCD), run:

```
make flash 
```

### 4. Clean Build Files

To wipe the temporary build/ directory and force a completely fresh cache remap:
```
make clean
```

## Project Structure
---

sdio_card_reader.c — Main application logic and TinyUSB MSC callbacks.

cmd_asm.pio / dat_asm.pio — PIO assembly scripts driving the hardware-level SDIO lines.

sdio_utils.c / sdio.h — Helper utilities for handling SD card initialization blocks.

usb_descriptors.c / tusb_config.h — USB configuration settings defining the Mass Storage device characteristics. 

Makefile — Automation script for compilation flashing, and debugging loops.