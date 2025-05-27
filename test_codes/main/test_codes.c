#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

// Pin Configuration
#define RST_PIN    GPIO_NUM_21
#define CE_PIN     GPIO_NUM_2
#define DC_PIN     GPIO_NUM_17
#define DIN_PIN    GPIO_NUM_23
#define CLK_PIN    GPIO_NUM_18
#define SPI_HOST   SPI2_HOST  // Use HSPI (SPI2)

// LCD Commands
#define LCD_CMD    0
#define LCD_DATA   1
#define LCD_SETY   0x40
#define LCD_SETX   0x80
#define LCD_DISPLAY_ON  0x0C

spi_device_handle_t spi;

void lcd_send(uint8_t data, uint8_t mode) {
    esp_err_t ret;
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data
    };
   
    gpio_set_level(DC_PIN, mode);  // Set DC pin: command/data
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

void lcd_init() {
    // Initialize GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RST_PIN) | (1ULL << CE_PIN) | (1ULL << DC_PIN),
        .mode = GPIO_MODE_OUTPUT
    };
    gpio_config(&io_conf);

    // Reset sequence
    gpio_set_level(RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(RST_PIN, 1);
   
    // Initialize SPI
    spi_bus_config_t buscfg = {
        .mosi_io_num = DIN_PIN,
        .miso_io_num = -1,
        .sclk_io_num = CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0
    };
    spi_device_interface_config_t devcfg = {
        .mode = 0,  // SPI mode 0
        .clock_speed_hz = 4 * 1000 * 1000,  // 4MHz
        .spics_io_num = CE_PIN,
        .queue_size = 1
    };
    spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI_HOST, &devcfg, &spi);

    // LCD initialization sequence
    lcd_send(0x21, LCD_CMD);  // Extended instruction set
    lcd_send(0xBF, LCD_CMD);  // Set contrast (VOP)
    lcd_send(0x04, LCD_CMD);  // Temp coefficient
    lcd_send(0x14, LCD_CMD);  // Bias mode
    lcd_send(0x20, LCD_CMD);  // Basic instruction set
    lcd_send(LCD_DISPLAY_ON, LCD_CMD);
    lcd_send(0x0C, LCD_CMD);  // Normal display mode
}

void app_main() {
    lcd_init();
   
    // Display text
    const char* text = "SUP DUDE, BICILOCOS ON ESP-IDF Nokia 5110";
    for (int i=0; text[i]; i++) {
        lcd_send(text[i], LCD_DATA);
    }
}
