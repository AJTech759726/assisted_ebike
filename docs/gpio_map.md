# GPIO Pin Mapping  

| Component                   | ESP32 GPIO | Description               |
|-----------------------------|------------|---------------------------|
| Nokia 5110 - DIN (MOSI)     | D23        | SPI Data                  |
| Nokia 5110 - RST            | D21        | Reset                     |
| Nokia 5110 - CLK (SCK)      | D18        | SPI Clock                 |
| Nokia 5110 - DC             | D17        | Data/Command              |
| Nokia 5110 - CE (CS)        | D2         | Chip Select               |
| RFID - MOSI                 | D23        | Shared SPI                |
| RFID - MISO                 | D19        | SPI Read                  |
| RFID - SCK                  | D18        | Shared SPI                |
| RFID - SDA (SS)             | D5         | RFID Chip Select          |
| RFID - RST                  | D4         | RFID Reset                |
| Assistance Potentiometer    | D26        | Analog input              |
| Pedal Transistor            | D27        | Digital input             |
| RCWL 1                      | D33        | Digital input             |
| RCWL 2                      | D32        | Digital input             |
| Blind Spot Light            | D35        | Digital output (3.3V)     |
| BLDC Motor                  | D25        | PWM and free control      |
