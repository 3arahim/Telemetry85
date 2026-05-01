# Telemetry85

[![Language](https://img.shields.io/badge/Language-C++11-blue.svg)](#)
[![Platform](https://img.shields.io/badge/Platform-Arduino%20%7C%20Linux-lightgrey.svg)](#)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](#)

A lightweight, zero-dependency C++ telemetry compression library optimized for high-frequency Vehicle Control Unit (VCU) data transmission.

## Overview

In motorsport telemetry (such as FSAE), transmitting standard JSON strings at high baud rates (e.g., 1,000,000 bps) frequently causes serial buffer overflows and memory fragmentation on microcontrollers with limited SRAM. 

Telemetry85 serves as a middleware layer between your sensor logic and your serialization library. It compresses high-resolution sensor data and boolean state arrays into compact, JSON-safe strings. This significantly reduces payload size, ensuring stable control loop frequencies without sacrificing the structural readability of JSON.

## Key Features

* **Bandwidth Optimization:** Reduces standard JSON payload sizes by over 80%.
* **O(1) Memory Footprint:** Operates entirely on stack memory using fixed-size buffers, eliminating the risk of heap fragmentation on 8-bit microcontrollers.
* **Zero Dependencies:** A single header file (`Telemetry85.h`) that relies only on standard C libraries.
* **Cross-Platform Compatibility:** Identical implementation for both embedded senders (Arduino) and POSIX receivers (Raspberry Pi/Linux).

## Architecture

Telemetry85 utilizes two core compression strategies:

1. **Fixed-Point Base85 Encoding:** 
   Standard 12-bit ADC sensor readings (0–4095) require up to 4 bytes in plaintext. Telemetry85 maps these values to an 85-character ASCII-safe alphabet. Because 85² = 7225, any 12-bit value is safely encoded into exactly 2 bytes.
2. **State Bitmasking:** 
   Dashboard switches and fault states are condensed from lengthy string descriptors into a single 16-bit integer via bitwise operations. This packed integer is then passed through the Base85 encoder.

---

## Performance Benchmarks

*Hardware profile: Arduino Mega 2560 transmitting to Raspberry Pi 4 via USB-Serial at 1,000,000 baud. Payload consists of 1x Float (scaled), 1x Long (truncated), and 11x Boolean states.*

| Metric | Standard JSON | Telemetry85 | Delta |
| :--- | :--- | :--- | :--- |
| **Average Payload Size** | ~180 bytes | 32 bytes | -82.2% |
| **Throughput Ceiling** | ~400 Hz | > 2000 Hz | +500% |
| **Dynamic Memory Allocation** | High | None | Stable |

---

## Integration Guide

Drop `Telemetry85.h` into your project includes.

### 1. Embedded Sender (Arduino)
```cpp
#include <ArduinoJson.h>
#include "Telemetry85.h"

void setup() {
  Serial.begin(1000000);
}

void loop() {
  JsonDocument doc;
  char buffer[3];

  // Scale float values to integers before encoding
  int engineTemp = 125; 
  Telemetry85::encode(engineTemp, buffer);
  doc["t1"] = buffer; 

  // Bitmask boolean states
  uint16_t dashStates = (1 << 0) | (0 << 1) | (1 << 2);
  Telemetry85::encode(dashStates, buffer);
  doc["d1"] = buffer;

  serializeJson(doc, Serial);
  Serial.print('\n');
  
  delay(10); // Enforce 100Hz transmission rate
}
