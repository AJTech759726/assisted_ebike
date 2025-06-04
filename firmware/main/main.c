// Full integrated code for ESP32 assisted electric bicycle system

#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "rc522.h" // External RC522 library

//-------------------------------- LCD Nokia 5110 --------------------------------

#define LCD_RST_PIN      GPIO_NUM_21
#define LCD_CE_PIN       GPIO_NUM_2
#define LCD_DC_PIN       GPIO_NUM_17
#define LCD_DIN_PIN      GPIO_NUM_23
#define LCD_CLK_PIN      GPIO_NUM_18
#define LCD_BL_PIN       GPIO_NUM_15  // Backlight
#define SPI_HOST_LCD     SPI2_HOST

#define LCD_CMD          0
#define LCD_DATA         1
#define LCD_DISPLAY_ON   0x0C

spi_device_handle_t lcd_spi;

void lcd_send(uint8_t data, uint8_t mode) {
    gpio_set_level(LCD_DC_PIN, mode);
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data
    };
    spi_device_polling_transmit(lcd_spi, &t);
}

void lcd_init() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LCD_RST_PIN) | (1ULL << LCD_CE_PIN) | (1ULL << LCD_DC_PIN),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&io_conf);

    gpio_set_level(LCD_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(LCD_RST_PIN, 1);

    spi_bus_config_t buscfg = {
        .mosi_io_num = LCD_DIN_PIN,
        .miso_io_num = -1,
        .sclk_io_num = LCD_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };
    spi_device_interface_config_t devcfg = {
        .mode = 0,
        .clock_speed_hz = 4000000,
        .spics_io_num = LCD_CE_PIN,
        .queue_size = 1
    };
    spi_bus_initialize(SPI_HOST_LCD, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI_HOST_LCD, &devcfg, &lcd_spi);

    lcd_send(0x21, LCD_CMD);
    lcd_send(0xBF, LCD_CMD);
    lcd_send(0x04, LCD_CMD);
    lcd_send(0x14, LCD_CMD);
    lcd_send(0x20, LCD_CMD);
    lcd_send(LCD_DISPLAY_ON, LCD_CMD);
}

void lcd_print(const char* text) {
    while (*text) {
        lcd_send(*text++, LCD_DATA);
    }
}

//-------------------------------- RC522 RFID --------------------------------

#define RC522_SCANNER_GPIO_RST     GPIO_NUM_4
#define RC522_SPI_BUS_GPIO_MISO    GPIO_NUM_19
#define RC522_SPI_BUS_GPIO_MOSI    GPIO_NUM_23
#define RC522_SPI_BUS_GPIO_SCLK    GPIO_NUM_18
#define RC522_SPI_SCANNER_GPIO_SDA GPIO_NUM_5
#define SPI_HOST_RFID              SPI3_HOST

rc522_driver_handle_t rfid_driver;
rc522_handle_t rfid;

static void on_picc_state_changed(void *arg, esp_event_base_t base, int32_t event_id, void *data) {
    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
    rc522_picc_t *picc = event->picc;

    if (picc->state == RC522_PICC_STATE_ACTIVE) {
        rc522_picc_print(picc);
        lcd_print("Authorized\n");
    } else if (picc->state == RC522_PICC_STATE_IDLE && event->old_state >= RC522_PICC_STATE_ACTIVE) {
        ESP_LOGI("RFID", "Card removed");
    }
}

void rfid_init() {
    rc522_spi_config_t driver_config = {
        .host_id = SPI_HOST_RFID,
        .bus_config = &(spi_bus_config_t){
            .miso_io_num = RC522_SPI_BUS_GPIO_MISO,
            .mosi_io_num = RC522_SPI_BUS_GPIO_MOSI,
            .sclk_io_num = RC522_SPI_BUS_GPIO_SCLK,
        },
        .dev_config = {
            .spics_io_num = RC522_SPI_SCANNER_GPIO_SDA,
        },
        .rst_io_num = RC522_SCANNER_GPIO_RST,
    };

    rc522_spi_create(&driver_config, &rfid_driver);
    rc522_driver_install(rfid_driver);

    rc522_config_t scanner_config = {.driver = rfid_driver};
    rc522_create(&scanner_config, &rfid);
    rc522_register_events(rfid, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed, NULL);
    rc522_start(rfid);
}

//-------------------------------- RCWL-0516 Sensor ------------------------------

#define RCWL_PIN_1      GPIO_NUM_33
#define RCWL_PIN_2      GPIO_NUM_32
#define LED_PIN         GPIO_NUM_35

void rcwl_init() {
    gpio_config_t rcwl_conf = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    rcwl_conf.pin_bit_mask = (1ULL << RCWL_PIN_1);
    gpio_config(&rcwl_conf);

    rcwl_conf.pin_bit_mask = (1ULL << RCWL_PIN_2);
    gpio_config(&rcwl_conf);

    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}

void rcwl_task(void *arg) {
    while (1) {
        int motion1 = gpio_get_level(RCWL_PIN_1);
        int motion2 = gpio_get_level(RCWL_PIN_2);
        gpio_set_level(LED_PIN, motion1 || motion2);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

//-------------------------------- Motor Control --------------------------------

#define MOTOR_PWM_PIN   GPIO_NUM_25
#define MOTOR_DIR_PIN   GPIO_NUM_26
#define MOTOR_EN_PIN    GPIO_NUM_27

#define PWM_CHANNEL     LEDC_CHANNEL_0
#define PWM_TIMER       LEDC_TIMER_0
#define PWM_FREQ        25000
#define PWM_RES         LEDC_TIMER_10_BIT

void motor_init() {
    ledc_timer_config_t pwm_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = PWM_RES,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&pwm_timer);

    ledc_channel_config_t pwm_channel = {
        .gpio_num = MOTOR_PWM_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = PWM_CHANNEL,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&pwm_channel);

    gpio_set_direction(MOTOR_DIR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(MOTOR_DIR_PIN, 1);

    gpio_set_direction(MOTOR_EN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(MOTOR_EN_PIN, 1);
}

void motor_set_speed(uint32_t speed) {
    speed = (speed > 1023) ? 1023 : speed;
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, speed);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
}

//-------------------------------- App Main --------------------------------

void app_main(void) {
    lcd_init();
    lcd_print("BICILOCOS SYSTEM\n");

    rfid_init();
    rcwl_init();
    motor_init();

    xTaskCreate(rcwl_task, "rcwl_task", 2048, NULL, 5, NULL);

    while (1) {
        if (gpio_get_level(RCWL_PIN_1) || gpio_get_level(RCWL_PIN_2)) {
            gpio_set_level(LED_PIN, 1);
        } else {
            gpio_set_level(LED_PIN, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
