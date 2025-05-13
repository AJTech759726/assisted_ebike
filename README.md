# Project: Assisted e-bike - ESP32
## Description
This project involves developing a electric bicycle using an ESP32 microcontroller, sensors, and a Nokia 5110 LCD screen.
The system is designed to display bicycle status information, motor assistance, and safety sensors such as blind spot detection and RFID.

## Technologies and tools
- ESP-IDF (Official Espressif framework for ESP32).
- C Programming language.
- Git and GitHub.
- FreeRTOS (Real-time task management).
- SPI Master driver (for Nokia 5110 screen control).

## Electronic components
- ESP32-WROOM-32
- Nokia 5110 LCD screen
- SNR8503M driver for BLDC motor
- RCWL-0516 sensors (blind spot detection)
- RFID module
- BLDC motor

## Features
- BLDC motor control with SNR8503M driver.
- Blind spot detection using RCWL-0516 sensors.
- Visual interface on Nokia 5110 display.
- Low-level control using ESP-IDF in C/C++.

## GPIO connections – ESP32-WROOM-32

| Component              | Description                        | ESP32 pin (GPIO) | Notes                                  |
|------------------------|------------------------------------|------------------|----------------------------------------|
| 📟 Nokia 5110 display  | LCD SPI                            | GPIO 18 (CLK)    | SPI clock                              |
|                        |                                    | GPIO 23 (DIN)    | Data in (MOSI)                         |
|                        |                                    | GPIO 5 (CE)      | Chip enable (may vary)                 |
|                        |                                    | GPIO 2 (DC)      | Data/Command                           |
|                        |                                    | GPIO 4 (RST)     | Reset                                  |
| 🧿 RFID RC522          | RFID reader SPI                    | GPIO 18 (SCK)    | Share with display (revisar)           |
|                        |                                    | GPIO 23 (MOSI)   | Share with display (revisar)           |
|                        |                                    | GPIO 19 (MISO)   | Required for SPI communication         |
|                        |                                    | GPIO 21 (SDA/SS) | Slave select                           |
| 📡 RCWL-0516 (1)       | Blind spot sensor (left)           | GPIO 32          | Digital output                         |
| 📡 RCWL-0516 (2)       | Blind spot sensor (right)          | GPIO 33          | Digital output                         |
| 💡 Rear LED            | Brake light                        | GPIO 26          | Controlled via software (HIGH/LOW)     |
| 🔁 Turn signal (left)  | Left directional light             | GPIO 27          | PWM or digital                         |
| 🔁 Turn signal (right) | Right directional light            | GPIO 14          | PWM or digital                         |
| ⚙️  BLDC driver         | PWM signal input                   | GPIO 25          | Control signal to driver               |

## Project structure

```text
workspace/
├── main/                          	# Main project code
│   ├── main.c                     	# Application entry point (app_main)
│   ├── CMakeLists.txt             	# CMake configuration for this subfolder
│   └── ...                        	# Other source files
├── managed_components/                 # Custom libraries
│   └── lcd_nokia5110/             	# Library for Nokia 5110 LCD screen
│       ├── src/                   	# Library source code
│       │   ├── lcd_nokia5110.c
│       │   └── font_5x7.c
│       ├── private_include/       	# Internal headers (private use)
│       │   └── lcd_nokia5110_priv.h
│       ├── include/               	# Public headers (library API)
│       │   ├── lcd_nokia5110.h
│       │   └── font_5x7.h
│       └── CMakeLists.txt
├── sdkconfig                      	# Project configuration
├── CMakeLists.txt
├── .gitignore
└── README.md
```

## How to compile and upload
1. Set up the ESP-IDF environment.
2. Clone this repository:

```bash
git clone https://github.com/AJTech759726/assisted_ebike.git
```

3. Enter the project folder:

```bash
cd workspace
```

4. Compile the project:

```bash
idf.py build
```

5. Flash the firmware to the ESP32:

```bash
idf.py -p PORT flash
```

6. Monitor the output:

```bash
idf.py monitor
```

## Project status

```
- [x] ✅ Basic ESP32 connection
- [ ] 📘 Display text on Nokia 5110 screen
- [x] ✅ Integrate RCWL-0516 blind spot sensors
- [x] ✅ Read data from RFID RC522 reader
- [ ] 💡 Control rear LED lights
- [ ] 🔁 Control turn signals (two wires)
- [ ] ⚙️  Connect and test the BLDC driver
- [ ] 🧠 Develop assistance logic
- [ ] 🛠️ Integrate all modules into the system
- [ ] 🧪 Perform integration tests on the bicycle
- [ ] 📦 Document all physical connections
- [ ] 📝 Add connection diagram to README
```
```
✅: Task completed
🛠️: Technical implementation
📐: Sensors
💡: Actuators
📘: Communication
📦: Hardware
```

## Author
- Developed by Alan Joshua Jiménez Flores
- [GitHub Profile](https://github.com/AJTech759726/)
---

Developed with [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/).
