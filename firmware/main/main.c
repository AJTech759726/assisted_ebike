#include <esp_log.h>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/dac.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "rc522.h"
#include "driver/rc522_spi.h"
#include "rc522_picc.h"

// Hardware configuration
#define RCWL_LEFT_GPIO       GPIO_NUM_33
#define RCWL_RIGHT_GPIO      GPIO_NUM_32
#define BLIND_SPOT_LIGHT_GPIO GPIO_NUM_25
#define SYSTEM_ACTIVE_LED    GPIO_NUM_2

// Motor control
#define DAC_OUTPUT_PIN       GPIO_NUM_25  // Connected to VSP of driver
#define PEDAL_HALL_PIN       GPIO_NUM_27
#define POTENTIOMETER_PIN    GPIO_NUM_26
#define ACCELERATOR_PIN      GPIO_NUM_34
#define HALL1_PIN            GPIO_NUM_3
#define HALL2_PIN            GPIO_NUM_1
#define HALL3_PIN            GPIO_NUM_22

// Display configuration
#define DISPLAY_RST_PIN      GPIO_NUM_21
#define DISPLAY_CE_PIN       GPIO_NUM_2
#define DISPLAY_DC_PIN       GPIO_NUM_17
#define DISPLAY_DIN_PIN      GPIO_NUM_23
#define DISPLAY_CLK_PIN      GPIO_NUM_18
#define DISPLAY_SPI_HOST     SPI2_HOST

// RFID configuration
#define RC522_SPI_HOST       SPI3_HOST
#define RC522_MISO_GPIO      GPIO_NUM_19
#define RC522_MOSI_GPIO      GPIO_NUM_23
#define RC522_SCLK_GPIO      GPIO_NUM_18
#define RC522_SDA_GPIO       GPIO_NUM_5
#define RC522_RST_GPIO       GPIO_NUM_4

// System parameters
#define BATTERY_DIVIDER_RATIO 7.2f
#define WHEEL_CIRCUMFERENCE   2.1f  // meters
#define HALL_SENSORS_PER_REV  6
#define MAX_SPEED_RPM         300   // Maximum expected motor RPM
#define PEDAL_TIMEOUT_MS      2000  // 2 seconds without pedaling cuts assist

// PID parameters
#define PID_UPDATE_MS         50
#define KP                    1.0
#define KI                    0.1
#define KD                    0.05

static bool system_activated = false;
static float current_speed = 0.0f;
static uint8_t assistance_level = 0;
static float battery_voltage = 0.0f;

// Motor control variables
static volatile int64_t last_hall_time = 0;
static volatile int64_t hall_period = 0;
static volatile bool hall_updated = false;
static volatile bool pedaling = false;
static volatile int64_t last_pedal_time = 0;
static float pid_integral = 0;
static float last_error = 0;

// RFID variables
static rc522_handle_t scanner;
static rc522_driver_handle_t driver;
static bool waiting_tag = true;

// Display variables
static spi_device_handle_t display_spi;

// LCD Commands
#define LCD_CMD     0
#define LCD_DATA    1
#define LCD_SETY    0x40
#define LCD_SETX    0x80
#define LCD_DISPLAY_ON 0x0C

// Hall sensor ISR
static void IRAM_ATTR hall_isr_handler(void* arg) {
    int64_t now = esp_timer_get_time();
    if (last_hall_time != 0) {
        hall_period = now - last_hall_time;
        hall_updated = true;
    }
    last_hall_time = now;
}

// Pedal sensor ISR
static void IRAM_ATTR pedal_isr_handler(void* arg) {
    pedaling = true;
    last_pedal_time = esp_timer_get_time();
}

// Display functions
void lcd_send(uint8_t data, uint8_t mode) {
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data
    };
    gpio_set_level(DISPLAY_DC_PIN, mode);
    spi_device_polling_transmit(display_spi, &t);
}

void lcd_init() {
    // Initialize GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DISPLAY_RST_PIN) | (1ULL << DISPLAY_CE_PIN) | (1ULL << DISPLAY_DC_PIN),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&io_conf);

    // Reset sequence
    gpio_set_level(DISPLAY_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(DISPLAY_RST_PIN, 1);
   
    // Initialize SPI
    spi_bus_config_t buscfg = {
        .mosi_io_num = DISPLAY_DIN_PIN,
        .miso_io_num = -1,
        .sclk_io_num = DISPLAY_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0
    };
    spi_device_interface_config_t devcfg = {
        .mode = 0,
        .clock_speed_hz = 4 * 1000 * 1000,
        .spics_io_num = DISPLAY_CE_PIN,
        .queue_size = 1
    };
    spi_bus_initialize(DISPLAY_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(DISPLAY_SPI_HOST, &devcfg, &display_spi);

    // LCD initialization sequence
    lcd_send(0x21, LCD_CMD);  // Extended instruction set
    lcd_send(0xBF, LCD_CMD);  // Set contrast (VOP)
    lcd_send(0x04, LCD_CMD);  // Temp coefficient
    lcd_send(0x14, LCD_CMD);  // Bias mode
    lcd_send(0x20, LCD_CMD);  // Basic instruction set
    lcd_send(LCD_DISPLAY_ON, LCD_CMD);
    lcd_send(0x0C, LCD_CMD);  // Normal display mode
}

void lcd_clear() {
    for (int i = 0; i < 504; i++) {
        lcd_send(0x00, LCD_DATA);
    }
}

void lcd_set_position(uint8_t x, uint8_t y) {
    lcd_send(LCD_SETX | x, LCD_CMD);
    lcd_send(LCD_SETY | y, LCD_CMD);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_send(*str++, LCD_DATA);
    }
}

void lcd_print_number(uint32_t num) {
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%lu", num);
    lcd_print(buffer);
}

void lcd_print_float(float num, uint8_t decimals) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.*f", decimals, num);
    lcd_print(buffer);
}

void update_display() {
    lcd_clear();
    lcd_set_position(0, 0);
    
    // Battery level
    lcd_print("Batt: ");
    lcd_print_float(battery_voltage, 1);
    lcd_print("V");
    
    // Speed
    lcd_set_position(0, 1);
    lcd_print("Speed: ");
    lcd_print_float(current_speed, 1);
    lcd_print("km/h");
    
    // Assistance level
    lcd_set_position(0, 2);
    lcd_print("Assist: ");
    lcd_print_number(assistance_level);
    lcd_print("%");
}

// Motor control functions
void motor_control_init() {
    // Configure DAC for motor control
    dac_output_enable(DAC_CHANNEL_1);
    
    // Setup GPIO interrupts for hall sensors and pedal
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << HALL1_PIN) | (1ULL << HALL2_PIN) | (1ULL << HALL3_PIN) | (1ULL << PEDAL_HALL_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    gpio_config(&io_conf);
    
    gpio_install_isr_service(0);
    gpio_isr_handler_add(HALL1_PIN, hall_isr_handler, NULL);
    gpio_isr_handler_add(HALL2_PIN, hall_isr_handler, NULL);
    gpio_isr_handler_add(HALL3_PIN, hall_isr_handler, NULL);
    gpio_isr_handler_add(PEDAL_HALL_PIN, pedal_isr_handler, NULL);
    
    // Configure ADC for potentiometer and accelerator
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);  // GPIO26 (potentiometer)
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);  // GPIO34 (accelerator)
}

void set_motor_output(float output) {
    // Constrain output to 0-1 range
    if (output < 0) output = 0;
    else if (output > 1) output = 1;
    
    // Convert to 8-bit DAC value (0-255)
    uint8_t dac_value = (uint8_t)(output * 255);
    dac_output_voltage(DAC_CHANNEL_1, dac_value);
}

float calculate_motor_speed() {
    float motor_speed = 0;
    if (hall_updated && hall_period > 0) {
        float frequency_hz = 1000000.0f / (float)hall_period;
        motor_speed = (frequency_hz / HALL_SENSORS_PER_REV) * 60.0f; // RPM
        hall_updated = false;
        
        // Convert RPM to km/h for display
        current_speed = (motor_speed * WHEEL_CIRCUMFERENCE) / 60.0f;
    }
    return motor_speed;
}

void motor_control_task(void *pvParameters) {
    float target_speed = 0;
    float current_speed_rpm = 0;
    
    while (1) {
        // Read sensors
        current_speed_rpm = calculate_motor_speed();
        int pot_value = adc1_get_raw(ADC1_CHANNEL_7);
        int accel_value = adc1_get_raw(ADC1_CHANNEL_6);
        
        // Calculate assistance level (30-80%)
        assistance_level = 30 + (pot_value * 50) / 4095;
        
        // Check if pedaling recently
        bool active_pedaling = (esp_timer_get_time() - last_pedal_time) < (PEDAL_TIMEOUT_MS * 1000);
        
        // Calculate motor output
        float motor_output = 0;
        
        // Direct accelerator override
        float accelerator = (float)accel_value / 4095.0f;
        if (accelerator > 0.1f) {
            motor_output = accelerator;
            pid_integral = 0; // Reset PID on direct accelerator use
        } 
        // PID control when pedaling
        else if (active_pedaling && pedaling) {
            target_speed = (MAX_SPEED_RPM * assistance_level) / 100.0f;
            float error = target_speed - current_speed_rpm;
            
            // PID calculation
            pid_integral += error * (PID_UPDATE_MS / 1000.0f);
            float derivative = (error - last_error) / (PID_UPDATE_MS / 1000.0f);
            
            motor_output = KP * error + KI * pid_integral + KD * derivative;
            motor_output = motor_output / MAX_SPEED_RPM; // Normalize
            
            last_error = error;
        }
        
        // Apply motor output
        set_motor_output(motor_output);
        pedaling = false;
        
        vTaskDelay(pdMS_TO_TICKS(PID_UPDATE_MS));
    }
}

// RFID callback
static void on_rfid_detection(void *arg, esp_event_base_t base,
                int32_t event_id, void *data) {
    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
    rc522_picc_t *picc = event->picc;

    if (waiting_tag && picc->state == RC522_PICC_STATE_ACTIVE) {
        ESP_LOGI("RFID", "Authorized TAG detected - Activating system");
        system_activated = true;
        waiting_tag = false;
        gpio_set_level(SYSTEM_ACTIVE_LED, 1);
    }
}

void setup_gpio() {
    // INPUT Configuration
    gpio_config_t input_conf = {
        .pin_bit_mask = (1ULL << RCWL_LEFT_GPIO) | (1ULL << RCWL_RIGHT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&input_conf);

    // OUTPUT Configuration
    gpio_config_t output_conf = {
        .pin_bit_mask = (1ULL << BLIND_SPOT_LIGHT_GPIO) | (1ULL << SYSTEM_ACTIVE_LED),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&output_conf);
}

void shutdown_system() {
    ESP_LOGI("SYSTEM", "Shutting down system");
    system_activated = false;
    waiting_tag = true;
    set_motor_output(0);
    gpio_set_level(SYSTEM_ACTIVE_LED, 0);
    gpio_set_level(BLIND_SPOT_LIGHT_GPIO, 0);
}

void app_main(void) {
    ESP_LOGI("SYSTEM", "E-Bike system initializing");

    // Initialize hardware
    setup_gpio();
    motor_control_init();
    lcd_init();

    // Initialize RFID
    rc522_spi_config_t driver_config = {
        .host_id = RC522_SPI_HOST,
        .bus_config = &(spi_bus_config_t) {
            .miso_io_num = RC522_MISO_GPIO,
            .mosi_io_num = RC522_MOSI_GPIO,
            .sclk_io_num = RC522_SCLK_GPIO,
        },
        .dev_config = {
            .spics_io_num = RC522_SDA_GPIO,
        },
        .rst_io_num = RC522_RST_GPIO,
    };
    rc522_spi_create(&driver_config, &driver);
    rc522_driver_install(driver);

    rc522_config_t scanner_config = {
        .driver = driver,
    };
    rc522_create(&scanner_config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, on_rfid_detection, NULL);
    rc522_start(scanner);

    // Initial state
    shutdown_system();
    lcd_clear();
    lcd_set_position(0, 0);
    lcd_print("Waiting for RFID");
    lcd_set_position(0, 1);
    lcd_print("Scan to activate");

    // Create motor control task
    xTaskCreate(motor_control_task, "motor_control", 4096, NULL, 5, NULL);

    while(1) {
        if (system_activated) {
            // Blind spot detection
            bool left_detected = gpio_get_level(RCWL_LEFT_GPIO);
            bool right_detected = gpio_get_level(RCWL_RIGHT_GPIO);
            gpio_set_level(BLIND_SPOT_LIGHT_GPIO, left_detected || right_detected);

            // Update display
            update_display();
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
