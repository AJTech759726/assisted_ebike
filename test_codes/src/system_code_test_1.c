#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "display.h"
#include "motor_control.h"
#include "rfid.h"
#include "sensors.h"
#include "ui.h"

#define TAG "EBIKE_MAIN"

// Pin Definitions (adjust according to your wiring)
#define LEFT_RADAR_GPIO         GPIO_NUM_4
#define RIGHT_RADAR_GPIO        GPIO_NUM_5
#define LEFT_LIGHT_GPIO         GPIO_NUM_18
#define RIGHT_LIGHT_GPIO        GPIO_NUM_19
#define MODE_BUTTON_GPIO        GPIO_NUM_23
#define POTENTIOMETER_ADC_CH    ADC1_CHANNEL_0
#define EMERGENCY_GPIO          GPIO_NUM_22
#define DIRECTION_GPIO_1        GPIO_NUM_25
#define DIRECTION_GPIO_2        GPIO_NUM_26
#define DIRECTION_GPIO_3        GPIO_NUM_27
#define MOTOR_PWM_GPIO          GPIO_NUM_12
#define MOTOR_DIR_GPIO          GPIO_NUM_13
#define MOTOR_ENABLE_GPIO       GPIO_NUM_14
#define BATTERY_ADC_CH          ADC1_CHANNEL_3

// System parameters
#define BATTERY_DIVIDER_RATIO   7.2f  // (R1+R2)/R2 for voltage divider
#define PWM_FREQ                5000  // Hz
#define PWM_RESOLUTION          LEDC_TIMER_10_BIT

// Global system state
typedef struct {
    bool system_active;
    bool emergency_stop;
    uint8_t assistance_level;
    bool bike_mode; // false=manual, true=auto
    float battery_voltage;
    float speed;
    bool left_radar;
    bool right_radar;
    uint8_t direction; // 0=left, 1=center, 2=right
} system_state_t;

static system_state_t system_state = {0};
static QueueHandle_t ui_event_queue = NULL;

void system_init_hardware() {
    // Initialize GPIOs
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LEFT_LIGHT_GPIO) | (1ULL << RIGHT_LIGHT_GPIO) |
                       (1ULL << MOTOR_DIR_GPIO) | (1ULL << MOTOR_ENABLE_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Input GPIOs
    io_conf.pin_bit_mask = (1ULL << LEFT_RADAR_GPIO) | (1ULL << RIGHT_RADAR_GPIO) |
                          (1ULL << MODE_BUTTON_GPIO) | (1ULL << EMERGENCY_GPIO) |
                          (1ULL << DIRECTION_GPIO_1) | (1ULL << DIRECTION_GPIO_2) |
                          (1ULL << DIRECTION_GPIO_3);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    // ADC configuration
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(POTENTIOMETER_ADC_CH, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(BATTERY_ADC_CH, ADC_ATTEN_DB_11);

    // PWM configuration for motor
    ledc_timer_config_t pwm_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&pwm_timer);

    ledc_channel_config_t pwm_channel = {
        .gpio_num = MOTOR_PWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&pwm_channel);
}

void system_read_sensors() {
    system_state.left_radar = gpio_get_level(LEFT_RADAR_GPIO);
    system_state.right_radar = gpio_get_level(RIGHT_RADAR_GPIO);
    
    // Update warning lights
    gpio_set_level(LEFT_LIGHT_GPIO, system_state.left_radar);
    gpio_set_level(RIGHT_LIGHT_GPIO, system_state.right_radar);
    
    // Read direction switch
    if (!gpio_get_level(DIRECTION_GPIO_1)) {
        system_state.direction = 0;
    } else if (!gpio_get_level(DIRECTION_GPIO_2)) {
        system_state.direction = 1;
    } else if (!gpio_get_level(DIRECTION_GPIO_3)) {
        system_state.direction = 2;
    }
    
    // Read potentiometer (assistance level)
    uint32_t pot_value = adc1_get_raw(POTENTIOMETER_ADC_CH);
    system_state.assistance_level = (uint8_t)((pot_value * 100) / 4095);
    
    // Read battery voltage
    uint32_t bat_value = adc1_get_raw(BATTERY_ADC_CH);
    float voltage = (bat_value * 3.3f / 4095.0f) * BATTERY_DIVIDER_RATIO;
    system_state.battery_voltage = voltage;
    
    // Calculate speed (simplified)
    system_state.speed = system_state.assistance_level * 0.3f;
}

void system_control_motor() {
    if (system_state.emergency_stop || !system_state.system_active) {
        gpio_set_level(MOTOR_ENABLE_GPIO, 0);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        return;
    }
    
    // Set direction
    gpio_set_level(MOTOR_DIR_GPIO, system_state.direction == 0 ? 0 : 1);
    
    // Set PWM duty
    uint32_t duty = (system_state.assistance_level * (1 << PWM_RESOLUTION)) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    
    // Enable motor
    gpio_set_level(MOTOR_ENABLE_GPIO, 1);
}

void system_check_emergency() {
    if (!gpio_get_level(EMERGENCY_GPIO)) {
        system_state.emergency_stop = true;
        system_state.system_active = false;
        ESP_LOGE(TAG, "Emergency stop activated");
        
        // Send UI event
        ui_event_t event = {.type = UI_EVENT_EMERGENCY};
        xQueueSend(ui_event_queue, &event, portMAX_DELAY);
    }
}

void system_task(void *pvParameters) {
    while (1) {
        if (system_state.system_active && !system_state.emergency_stop) {
            system_read_sensors();
            system_control_motor();
            system_check_emergency();
            
            // Update UI
            ui_event_t event = {
                .type = UI_EVENT_UPDATE,
                .data = {
                    .battery_voltage = system_state.battery_voltage,
                    .speed = system_state.speed,
                    .assistance_level = system_state.assistance_level,
                    .direction = system_state.direction,
                    .left_radar = system_state.left_radar,
                    .right_radar = system_state.right_radar,
                    .mode = system_state.bike_mode
                }
            };
            xQueueSend(ui_event_queue, &event, portMAX_DELAY);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void rfid_callback(bool authenticated) {
    if (authenticated && !system_state.emergency_stop) {
        system_state.system_active = true;
        ESP_LOGI(TAG, "System activated by RFID");
        
        ui_event_t event = {.type = UI_EVENT_SYSTEM_ACTIVE};
        xQueueSend(ui_event_queue, &event, portMAX_DELAY);
    }
}

void button_isr_handler(void *arg) {
    // Debounce check would be needed in a real implementation
    system_state.bike_mode = !system_state.bike_mode;
    
    ui_event_t event = {
        .type = UI_EVENT_MODE_CHANGE,
        .data.mode = system_state.bike_mode
    };
    xQueueSend(ui_event_queue, &event, 0);
}

void app_main() {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize hardware
    system_init_hardware();
    
    // Create UI event queue
    ui_event_queue = xQueueCreate(10, sizeof(ui_event_t));
    
    // Initialize components
    display_init();
    motor_control_init();
    rfid_init(rfid_callback);
    ui_init(ui_event_queue);
    
    // Setup mode button interrupt
    gpio_install_isr_service(0);
    gpio_isr_handler_add(MODE_BUTTON_GPIO, button_isr_handler, NULL);
    gpio_set_intr_type(MODE_BUTTON_GPIO, GPIO_INTR_NEGEDGE);
    
    // Create system task
    xTaskCreate(system_task, "system_task", 4096, NULL, 5, NULL);
    
    // Show initial screen
    ui_event_t event = {.type = UI_EVENT_STARTUP};
    xQueueSend(ui_event_queue, &event, portMAX_DELAY);
    
    ESP_LOGI(TAG, "E-Bike system initialized");
}
