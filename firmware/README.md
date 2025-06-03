# Firmware v2.0

Second version of the smart bicycle assistance system firmware.  
Now includes full integration of all hardware, sensor fusion, PID motor control, and system state management.

## Objective

Implement a fully functional smart bicycle assistance system using the ESP32, capable of managing the following components:

- **Nokia 5110 LCD screen** (PCD8544 controller)
- **RFID RC522 module** for authentication
- **2 RCWL-0516 microwave sensors** for blind spot detection
- **Assistance potentiometer** to regulate motor power
- **Cadence sensor** (hall-effect transistor) for pedal detection
- **Brushless motor driver** with DAC output and PID speed control
- **Turn signals** with automatic timeout
- **Blind spot indicator LEDs**
- **Battery voltage monitoring**
- **Speed calculation** via 3 hall sensors on motor
- **System status LED**

## Structure

- `main/`: Main application logic and system loop.
- `components/`: Custom drivers and modules:
  - `display/`: LCD screen control
  - `rfid/`: RFID module interface
  - `motor_control/`: PID motor control logic
  - `blind_spot/`: RCWL-0516 sensor handling
- `include/`: Global headers for components and shared definitions.
- `sdkconfig`: ESP-IDF project configuration file.
- `CMakeLists.txt`: Project build instructions.

## New Features in v2.0

- ✅ **PID motor assistance** with dynamic speed input
- ✅ **Cadence timeout** to cut motor when user stops pedaling
- ✅ **Auto-resetting turn signals**
- ✅ **OLED screen** now displays real-time system status
- ✅ **Battery monitoring** through ADC voltage divider
- ✅ **RFID tag system** to enable or disable bike access
- ✅ **Blind spot detection** and LED indication
- ✅ **Speed calculation** using interrupts and hall sensor pulses

## Notes

- Uses **two SPI buses** for RFID and display.
- DAC output is used to generate analog signal for motor driver.
- All sensors and controls are **read and updated periodically** in the main loop.

---

> This version focuses on robustness, modularity, and clarity in control logic.  
> Ideal for testing full system behavior under real bicycle use conditions.