# Change Log (Changelog)  

## v2.0.0 - June 3rd, 2025
- Fully integrated system firmware for smart bicycle assistance
- Added PID-based motor control for smoother assistance
- Switched from Nokia 5110 (SPI) to OLED display (I2C) for better visuals
- Display now shows real-time speed, motor status, battery level
- Added:
  - RPM calculation based on cadence input
  - Motor enable logic with override via RFID
  - Battery level reading using ADC
  - OLED screen update loop with interface elements
- Improved structure and modularization of code
- Updated wiring and logic diagrams for new components

## v1.0.0 - June 1st, 2025
- First functional version of the smart bicycle assistance system
- Integrated and controlled components:
  - Nokia 5110 LCD screen
  - RFID RC522 module
  - 2 RCWL-0516 sensors
  - Assistance potentiometer
  - Cadence detection input
  - Blind spot LED warning outputs
  - Turn signals
  - Brushless motor driver
- Displayed system status on screen
- Assisted pedaling enabled based on cadence and potentiometer
- LED warning triggered when motion is detected

## v0.1.0 - May 27th, 2025  
- Project reorganized with folders: firmware, test, docs, versions  
- Initial documentation created (architecture, pins, power)  
- First tests: RFID reader and Nokia 5110 screen  
