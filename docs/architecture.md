# System Architecture  

The smart bicycle assistance system consists of:  

- **Control Unit**: ESP32 DevKit  
- **Sensors**:  
  - RCWL-0516 x2 (blind spot detection)  
  - Transistor (pedal sensor)  
  - Potentiometer (assistance level)  
  - RFID (user identification)  
- **Actuators**:  
  - Brushless motor (assistance)  
  - LED lights (blind spot warning)  
  - Nokia 5110 screen (UI)  
- **Power Source**:  
  - 36V battery (bicycle)  
  - DC-DC BOOST converter (36V → 5V/3.3V)  

The modules communicate internally via SPI, ADC, and digital inputs.  
