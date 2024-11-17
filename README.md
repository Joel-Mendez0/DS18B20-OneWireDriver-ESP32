# DS18B20-OneWireDriver-ESP32

This is a DS18B20 Driver written entirely from scratch in C for the ESP32. It provides low-level functionality for communicating with DS18B20 temperature sensors over the 1-Wire protocol.

## Features

- **1-Wire Protocol**: Directly interfaces with DS18B20 temperature sensors over the 1-Wire bus.
- **No External Libraries**: Written entirely from scratch without relying on external libraries for DS18B20 communication.
- **Multiple Sensor Support**: Can detect multiple devices on the same bus.
- **Temperature Reading**: Supports reading temperature data from the connected DS18B20 sensors.

## Hardware Requirements

- **ESP32 Development Board**: Any board with an ESP32 microcontroller.
- **DS18B20 Temperature Sensor(s)**: 1 or more DS18B20 sensors.
- **Resistor**: A 5kΩ pull-up resistor connected between the data pin and VCC (5V).

## Pin Configuration

- **DQ Pin**: The data pin for communication with DS18B20. In the provided code, this is set to GPIO 27 (`#define DQ_PIN GPIO_NUM_27`).
- **External Pull-up Resistor**: A 5kΩ resistor should be placed between the DQ pin and VCC (5V) to ensure proper communication.

## Installation and Setup

1. **Install ESP32 Toolchain**:
   Follow the instructions to set up the ESP32 toolchain. You can use [Espressif's official guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) for this.
   
2. **Clone the Repository**:
   Clone this repository to your local machine.

   ```bash
   git clone https://github.com/Joel-Mendez0/DS18B20-OneWireDriver-ESP32.git
   cd DS18B20-OneWireDriver-ESP32
3. **Configure the Project**: Open the project in your IDE or editor of choice. Make sure to configure the GPIO pin according to your hardware setup by modifying the DQ_PIN in DS18B20.h.
4. **Build the Project**: You can use the idf.py build system to build the project for your ESP32.
    ```bash
        idf.py set-target esp32
        idf.py build
5. **Flash the Firmware and Monitor**: After building the project, you can flash the firmware to your ESP32 board. Once the firmware is flashed, you can monitor the output via the serial port:
    ``` bash
        idf.py flash
        idf.py monitor

## How It Works

### Initialization
The `DS18B20_INIT()` function configures the data pin (DQ) for 1-Wire communication. It initializes the GPIO pin in input-output open-drain mode, which is required for communication with the DS18B20 sensor over the 1-Wire bus.

### Device Discovery
The `SEARCH_ROM()` function is used to search for all connected DS18B20 devices on the 1-Wire bus. It uses the 1-Wire search algorithm to detect up to 10 devices. The ROM codes of all found devices are stored in the `FOUND_ROMS` array.

### Temperature Conversion
The `CONVERT_T()` function is used to initiate the temperature conversion process on the DS18B20. After the conversion is complete, the sensor's scratchpad memory holds the temperature data, which can then be read using the `READ_SCRATCHPAD()` function.

### Reading Temperature
The `GetTemperature()` function reads the scratchpad data from the first detected DS18B20 device and calculates the temperature in Celsius. The function converts the raw data from the scratchpad into a usable temperature value.

## Code Overview

### `DS18B20.h`
This header file defines the functions and constants required for 1-Wire communication and for sending specific commands to the DS18B20 sensor. These commands include device discovery, reading scratchpad memory, starting temperature conversion, and more.

### `DS18B20.c`
This source file implements the functions declared in `DS18B20.h`. It handles the low-level bit-banging operations necessary for 1-Wire communication. It also implements the ROM command processing (such as search, match, and skip) and DS18B20 function commands (like reading scratchpad data and converting temperature).

## Functions

- `DS18B20_INIT()`: Initializes the 1-Wire bus by configuring the GPIO pin.
- `SEARCH_ROM()`: Searches for all connected DS18B20 devices on the 1-Wire bus and stores their ROM codes.
- `GET_TEMPERATURE()`: Retrieves the current temperature reading from the first DS18B20 sensor.
- `READ_ROM()`: Reads the ROM code of the first detected DS18B20 device.
- `MATCH_ROM()`: Matches the ROM code of a specific DS18B20 device to communicate with it.
- `SKIP_ROM()`: Skips the ROM code, allowing all devices on the bus to respond to the next command simultaneously.
- `READ_SCRATCHPAD()`: Reads the scratchpad memory of the DS18B20, which contains sensor data including the temperature.
- `CONVERT_T()`: Starts a temperature conversion process on the DS18B20.
- `DS18B20_CalculateCRC()`: A placeholder function for CRC calculation (currently not implemented).

## Example Usage

Once the setup is complete, you can use the following code to initialize the sensor and retrieve the temperature:

```c
#include "DS18B20.h"

void app_main() {
    DS18B20_INIT();
    
    // Search for DS18B20 devices
    uint8_t deviceCount = SEARCH_ROM();
    printf("Found %d devices\n", deviceCount);

    if (deviceCount > 0) {
        // Get temperature from the first detected device
        float temperature = GetTemperature();
        printf("Temperature: %.2f°C\n", temperature);
    } else {
        printf("No devices found.\n");
    }
}
```
## License

This project is licensed under the MIT License

## Acknowledgements

The DS18B20 sensor is a popular digital temperature sensor from Maxim Integrated. You can find more information in the official DS18B20 datasheet here. https://cdn.sparkfun.com/datasheets/Sensors/Temp/DS18B20.pdf
