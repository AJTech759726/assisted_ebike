#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Nokia display pins 5110
#define LCD_CLK   18
#define LCD_DIN   23
#define LCD_DC    16
#define LCD_CS    5
#define LCD_RST   17

Adafruit_PCD8544 display = Adafruit_PCD8544(LCD_CLK, LCD_DIN, LCD_DC, LCD_CS, LCD_RST);

// RFID pins (RC522)
#define RST_PIN         22
#define SS_PIN          21

// Hall and pedal pins
#define HALL_SENSOR_PIN     36
#define PEDAL_SENSOR_PIN    39

// Motor pins
#define MOTOR_DAC_PIN       25  // PWM

// LED pins
#define LEFT_SIGNAL_PIN     14
#define RIGHT_SIGNAL_PIN    27
#define SYSTEM_LED_PIN      2

// Blind spot sensors pins
#define BLIND_LEFT_PIN      32
#define BLIND_RIGHT_PIN     33

// Accelerator and potentiometer
#define ACCEL_PIN           34
#define POT_PIN             35

// Battery voltage
#define BATTERY_ADC_PIN     4
#define BATTERY_DIVIDER_RATIO 2.0

// PID
float kp = 0.1, ki = 0.01, kd = 0.05;
float targetSpeed = 0;
float currentSpeed = 0;
float lastSpeedError = 0;
float speedIntegral = 0;

// State
bool systemActivated = false;
unsigned long lastHallTime = 0;
volatile unsigned long hallInterval = 1000000;  // µs

MFRC522 rfid(SS_PIN, RST_PIN);

// ISR for Hall sensor
void IRAM_ATTR onHall() {
  unsigned long now = micros();
  hallInterval = now - lastHallTime;
  lastHallTime = now;
}

// RFID autentication
bool checkRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return false;

  byte expectedUID[4] = {0xDE, 0xAD, 0xBE, 0xEF}; // Ejemplo
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != expectedUID[i]) return false;
  }

  return true;
}

// Initialization
void setup() {
  Serial.begin(115200);

  pinMode(SYSTEM_LED_PIN, OUTPUT);
  pinMode(MOTOR_DAC_PIN, OUTPUT);
  pinMode(LEFT_SIGNAL_PIN, OUTPUT);
  pinMode(RIGHT_SIGNAL_PIN, OUTPUT);
  pinMode(BLIND_LEFT_PIN, INPUT);
  pinMode(BLIND_RIGHT_PIN, INPUT);
  pinMode(ACCEL_PIN, INPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(BATTERY_ADC_PIN, INPUT);

  pinMode(HALL_SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALL_SENSOR_PIN), onHall, RISING);

  SPI.begin();
  rfid.PCD_Init();

  // PWM for motor
  ledcSetup(0, 5000, 8); // Canal 0, 5kHz, 8 bits
  ledcAttachPin(MOTOR_DAC_PIN, 0);

  // Initialize display
  display.begin();
  display.setContrast(60);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0, 0);
  display.println("Insert RFID...");
  display.display();
}

// PID and speed
void updateSpeedControl() {
  if (hallInterval > 0) {
    currentSpeed = 1000000.0 / hallInterval; // Hz ~ proportional to speed
  }

  float error = targetSpeed - currentSpeed;
  speedIntegral += error;
  float derivative = error - lastSpeedError;

  float output = kp * error + ki * speedIntegral + kd * derivative;
  lastSpeedError = error;

  output = constrain(output, 0, 255);
  ledcWrite(0, output);
}

// Turn signals
void checkTurnSignals() {
  digitalWrite(LEFT_SIGNAL_PIN, digitalRead(BLIND_LEFT_PIN));
  digitalWrite(RIGHT_SIGNAL_PIN, digitalRead(BLIND_RIGHT_PIN));
}

// Nokia display
void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("SPD:");
  display.print(currentSpeed, 1);

  float battVoltage = analogRead(BATTERY_ADC_PIN) * (3.3 / 4095.0) * BATTERY_DIVIDER_RATIO;
  display.setCursor(0, 10);
  display.print("BAT:");
  display.print(battVoltage, 2);
  display.print("V");

  display.display();
}

// Main loop
void loop() {
  if (!systemActivated) {
    if (checkRFID()) {
      systemActivated = true;
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Welcome!");
      display.display();
      digitalWrite(SYSTEM_LED_PIN, HIGH);
      delay(1000);
    }
    delay(100);
    return;
  }

  float accelInput = analogRead(ACCEL_PIN) / 4095.0;
  float potInput = analogRead(POT_PIN) / 4095.0;
  targetSpeed = accelInput * potInput * 100;

  updateSpeedControl();
  checkTurnSignals();
  updateDisplay();

  delay(100);
}
