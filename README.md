# Project: Assisted e-bike - ESP32

## Description

This project involves developing a electric bicycle using an ESP32 microcontroller, sensors, and a Nokia 5110 LCD screen.
The system is designed to display bicycle status information, motor assistance, and safety sensors such as blind spot detection and RFID.

---

## Technologies and tools

- ESP-IDF (Official Espressif framework for ESP32).
- C Programming language.
- Git and GitHub.
- FreeRTOS (Real-time task management).
- SPI Master driver (for Nokia 5110 screen control).

---

## Electronic components

- ESP32-WROOM-32
- Nokia 5110 LCD screen
- SNR8503M driver for BLDC motor
- RCWL-0516 sensors (blind spot detection)
- RFID module
- BLDC motor

---

## Features

- BLDC motor control with SNR8503M driver.
- Blind spot detection using RCWL-0516 sensors.
- Visual interface on Nokia 5110 display.
- Low-level control using ESP-IDF in C/C++.

---

## Project Documentation  

The detailed documentation is located in the [`docs/`](./docs/) folder:  

| File                      | Description |
|---------------------------|-------------|
| [`architecture.md`](./docs/architecture.md)        | General architecture of the assistance system. |
| [`components.md`](./docs/components.md)            | Electronic components used. |
| [`gpio_map.md`](./docs/gpio_map.md)                | GPIO pin mapping between modules and the ESP32. |
| [`power_distribution.md`](./docs/power_distribution.md) | Power distribution from the 36V battery to various modules. |
| [`software_flow.md`](./docs/software_flow.md)      | Software logic flow (state and events). |
| [`roadmap.md`](./docs/roadmap.md)                  | Development roadmap for software and hardware. |
| [`changelog.md`](./docs/changelog.md)              | Version change log. |

---

## Diagrams

Located in the [`hardware/`](./hardware/) directory:

| Diagram                                 | Description                        |
|-----------------------------------------|------------------------------------|
| ![Schematic](./hardware/schematic_diagram.png) | Logical schematic of the circuit   |
| ![Wiring](./hardware/wiring_diagram.png)       | Wiring diagram for physical setup  |

---

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

---

## Project structure

```text
assisted_ebike_project/
├── docs/                       # Project documentation in Markdown
│   ├── architecture.md
│   ├── components.md
│   └── ...
├── firmware/
│   └── v1.0/
│       ├── main/               # Main ESP32 application source code
│       │   ├── main.c
│       │   └── CMakeLists.txt
│       ├── include/           # Configuration and public headers
│       │   └── config.h
│       └── CMakeLists.txt
├── hardware/                  # Hardware diagrams and schematics
│   ├── schematic_diagram.png
│   └── wiring_diagram.png
├── test/                      # Unit or integration testing files
├── .gitignore
├── LICENSE
└── README.md
```

---

## How to compile and upload

1. **Set up the ESP-IDF environment** (see [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)).

2. **Clone the repository**:

```bash
git clone https://github.com/AJTech759726/assisted_ebike.git
```

3. **Navigate to the firmware directory**:

```bash
cd assisted_ebike/firmware/v...
```

4. *(Optional)* **Set ESP32 as the target**:

```bash
idf.py set-target esp32
```

5. **Build the firmware**:

```bash
idf.py build
```

6. **Flash to ESP32** (replace ```<your_port>``` with your actual port, e.g., ```/dev/ttyUSB0``` or ```COM3```):

```bash
idf.py -p <your_port> flash
```

7. **Monitor output**:

```bash
idf.py monitor
```

You can combine flashing and monitoring in one command:

```bash
idf.py -p <your_port> flash monitor
```

Press ```Ctrl+]``` to exit monitor mode.

---

## Project status

```
- [x] 📦 Basic ESP32 connection
- [x] 📘 Display text on Nokia 5110 screen
- [x] 📐 Integrate RCWL-0516 blind spot sensors
- [x] 📐 Read data from RFID RC522 reader
- [ ] 💡 Control rear LED lights
- [ ] 🔁 Control turn signals (two wires)
- [x] ⚙️  Connect and test the BLDC driver
- [ ] 🧠 Develop assistance logic
- [ ] 🛠️ Integrate all modules into the system
- [ ] 🧪 Perform integration tests on the bicycle
- [ ] 📦 Document all physical connections
- [ ] 📝 Add connection diagram to README
```

---

**Legend**:

```
🛠️: Technical implementation
📐: Sensors
💡: Actuators
📘: Communication
📦: Hardware
```

---

## Author

- Developed by **Alan Joshua Jiménez Flores**

- [GitHub Profile](https://github.com/AJTech759726/)

---

Developed with [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/).
