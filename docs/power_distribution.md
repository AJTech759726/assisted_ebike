# Power Distribution  

**Main Source**: 36V bicycle battery  
**Conversion**: DC-DC BOOST to 5V and then to 3.3V for logic  

| Line         | Connected to                                           |
|-------------|--------------------------------------------------------|
| 3.3V        | Potentiometer, Transistor, Screen, RFID, RCWLs         |
| GND         | All sensors, actuators, and the ESP32                  |
| DC BOOST OUT | BLDC motor, blind spot lights                         |
