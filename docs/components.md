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
  - Pins: OUT1 (D33), OUT2 (D25)

- **Assistance Potentiometer**
  - Reads analog voltage for pedal assist level
  - Pin: D27

- **Cadence Transistor**
  - Detects pedal rotations
  - Pin: D26

- **Voltage Dividers (x2)**
  - Used for battery monitoring or sensor logic level detection
  - Pins: D13, D14

- **Blindspot Lights**
  - Controlled via GPIO
  - Pins: D35, D34

- **Brushless Motor Driver**
  - Controlled via ESP32 PWM/GPIO
  - Pins available: D22, D1, D3, D16, D15, D32
