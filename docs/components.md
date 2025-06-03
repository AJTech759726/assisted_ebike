# Components

## Microcontroller
- **ESP32 DevKit v1**
  - Dual-core processor with Wi-Fi and Bluetooth
  - Powered at 3.3V

## Sensors and Modules
- **Nokia 5110 LCD**
  - Interface: SPI
  - Pins: DIN (D23), CLK (D18), CE (D2), DC (D17), RST (D21)

- **RFID Reader (RC522)**
  - Interface: SPI
  - Pins: MOSI (D23), MISO (D19), SCK (D18), SDA (D5), RST (D4)

- **RCWL-0516 (x2)**
  - Motion detection module
  - Pins: OUT1 (D33), OUT2 (D32)

- **Assistance Potentiometer**
  - Reads analog voltage for pedal assist level
  - Pin: D26

- **Accelerator trigger**
  - Reads analog voltage for manual motor engine
  - Pin: D24

- **Cadence Transistor**
  - Detects pedal rotations
  - Pin: D27

- **Turn signals activation**
  - Used for the logical function of the blind spot sensors
  - Pins: D13, D14

- **Blindspot Light**
  - Controlled via GPIO
  - Pin: D35

- **Brushless Motor Driver**
  - Controlled via ESP32 PWM/GPIO
  - Pin: D25
