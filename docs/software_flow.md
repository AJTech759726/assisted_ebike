# Software Flow

## Initialization
1. Configure GPIOs and SPI/I2C peripherals.
2. Initialize custom components (LCD, RFID, Sensors).

## Main Loop
1. Read sensor data:
   - Potentiometer level
   - Cadence
   - Motion detectors
   - RFID tag
2. Update display based on system state.
3. Control outputs:
   - Blindspot lights
   - Assist level
   - Motor power
