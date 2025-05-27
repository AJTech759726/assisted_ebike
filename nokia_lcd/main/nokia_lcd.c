#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// LCD Pin Definitions
#define RST_PIN  4
#define CE_PIN   5
#define DC_PIN   2
#define DIN_PIN 23
#define CLK_PIN 18

// FreeRTOS Objects
SemaphoreHandle_t dataMutex;

// LCD Object
Adafruit_PCD8544 display = Adafruit_PCD8544(CLK_PIN, DIN_PIN, DC_PIN, CE_PIN, RST_PIN);

// Shared Data Structure
typedef struct {
  int counter;
  float temperature;
  char status[14];
} DisplayData;

DisplayData displayData = {0, 25.0, "Status: OK"};

// LCD Task (Updates Display)
void lcdTask(void *pvParameters) {
  display.begin();
  display.setContrast(100);
  display.clearDisplay();
  display.display();

  while(1) {
    // Take mutex to safely copy data
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    DisplayData localData = displayData;
    xSemaphoreGive(dataMutex);

    // Update display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf("Count: %d", localData.counter);
    display.setCursor(0, 10);
    display.printf("Temp: %.1fC", localData.temperature);
    display.setCursor(0, 20);
    display.println(localData.status);
    display.display();

    vTaskDelay(500 / portTICK_PERIOD_MS); // Refresh every 500ms
  }
}

// Sensor Task
void sensorTask(void *pvParameters) {
  while(1) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    displayData.temperature += 0.5;
    if(displayData.temperature > 30.0) displayData.temperature = 25.0;
    snprintf(displayData.status, sizeof(displayData.status), "Status: OK");
    xSemaphoreGive(dataMutex);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

// Counter Task
void counterTask(void *pvParameters) {
  while(1) {
    xSemaphoreTake(dataMutex, portMAX_DELAY);
    displayData.counter++;
    xSemaphoreGive(dataMutex);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
 
  // Initialize mutex
  dataMutex = xSemaphoreCreateMutex();

  // Create tasks
  xTaskCreatePinnedToCore(lcdTask, "LCD Task", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(sensorTask, "Sensor Task", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(counterTask, "Counter Task", 2048, NULL, 1, NULL, 0);

  Serial.println("FreeRTOS LCD System Started!");
}

void loop() {}
