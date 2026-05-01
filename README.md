<div align="center">

# Telemetry85

**A high-performance C++ telemetry compression library for Vehicle Control Units.**

[![Language](https://img.shields.io/badge/Language-C++11-blue.svg)](#)
[![Platform](https://img.shields.io/badge/Platform-Arduino%20%7C%20Linux-lightgrey.svg)](#)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](#)

</div>

<br>

**The Problem:** Transmitting standard JSON telemetry at high baud rates (e.g., 1,000,000 bps) frequently causes serial buffer overflows and memory fragmentation on microcontrollers with limited SRAM, such as the Arduino Mega. 

**The Solution:** Telemetry85 acts as a middleware layer between your sensor logic and your serialization library. It compresses high-resolution sensor data and boolean state arrays into compact, JSON-safe strings. This significantly reduces payload size, ensuring stable control loop frequencies without sacrificing the structural readability of JSON.

---

## Architecture

Telemetry85 utilizes two core compression strategies:

1. **Fixed-Point Base85 Encoding:** 
   Standard 12-bit ADC sensor readings (0–4095) require up to 4 bytes in plaintext. Telemetry85 maps these values to an 85-character ASCII-safe alphabet. Because 85² = 7225, any 12-bit value safely encodes into exactly 2 bytes.
2. **State Bitmasking:** 
   Dashboard switches and fault states are condensed from lengthy string descriptors into a single 16-bit integer via bitwise operations. This packed integer is then passed through the Base85 encoder.

<details>
<summary><b>View Performance Benchmarks</b></summary>

<br>

*Hardware profile: Arduino Mega 2560 transmitting to Raspberry Pi 4 via USB-Serial at 1,000,000 baud. Payload consists of 1x Float (scaled), 1x Long (truncated), and 11x Boolean states.*

| Metric | Standard JSON | Telemetry85 | Delta |
| :--- | :--- | :--- | :--- |
| **Average Payload Size** | ~180 bytes | 32 bytes | **-82.2%** |
| **Throughput Ceiling** | ~400 Hz | > 2000 Hz | **+500%** |
| **Dynamic Memory Allocation**| High | None | **Stable** |

</details>

---

## Integration Guide

Drop `Telemetry85.h` into your project includes on both your transmitting and receiving hardware. It requires zero external dependencies.

### 1. Embedded Sender (Arduino Mega)

This implementation reads sensor data, compresses it, and transmits it via USB-Serial.
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

  // Bitmask boolean dashboard states
  uint16_t dashStates = (1 << 0) | (0 << 1) | (1 << 2);
  Telemetry85::encode(dashStates, buffer);
  doc["d1"] = buffer;

  serializeJson(doc, Serial);
  Serial.print('\n');
  
  delay(10); // Enforce 100Hz transmission rate
}
```

### 2. POSIX Receiver (Raspberry Pi / Linux)

This implementation opens the hardware serial port, buffers the incoming 1,000,000 baud stream, and decodes the Base85 payload back into raw integers.
```cpp
#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <ArduinoJson.h>
#include "Telemetry85.h"

int main() {
    // 1. Open the hardware serial port
    int fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        std::cerr << "Error opening serial port.\n";
        return 1;
    }

    // 2. Configure for 1,000,000 baud
    struct termios tty;
    tcgetattr(fd, &tty);
    cfsetispeed(&tty, B1000000);
    cfsetospeed(&tty, B1000000);
    tcsetattr(fd, TCSANOW, &tty);

    std::string line = "";
    
    // 3. Continuous Read Loop
    while (true) {
        char c;
        if (read(fd, &c, 1) > 0) {
            if (c == '\n') {
                // Parse the complete JSON frame
                JsonDocument doc;
                DeserializationError err = deserializeJson(doc, line);
                
                if (!err) {
                    // Decode metric
                    const char* encodedTemp = doc["t1"];
                    uint16_t decodedTemp = Telemetry85::decode(encodedTemp);

                    // Decode bitmask
                    const char* encodedStates = doc["d1"];
                    uint16_t decodedStates = Telemetry85::decode(encodedStates);

                    std::cout << "Engine Temp: " << decodedTemp << "°C\n";
                    std::cout << "Traction Control Enabled: " << (decodedStates & (1 << 0)) << "\n";
                }
                line.clear();
            } else {
                line += c;
            }
        }
    }

    close(fd);
    return 0;
}
```

````</ArduinoJson.h></ArduinoJson.h>
