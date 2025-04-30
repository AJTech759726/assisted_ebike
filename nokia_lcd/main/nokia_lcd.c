#include "lcd_nokia5110.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "LCD_MAIN"

void app_main(void)
{
    ESP_LOGI(TAG, "Starting application with Nokia 5110 display");

    lcd_nokia5110_t lcd = NULL; 				// Pointer in use

    // 1. Initial configuration of the LCD
    lcd_nokia5110_config_t config = {
        .pin_sclk = GPIO_NUM_18, 				// Pin CLK SPI
        .pin_din = GPIO_NUM_23, 				// Pin MOSI SPI
        .pin_dc = GPIO_NUM_4, 					// Pin Data/Command
        .pin_cs = GPIO_NUM_5, 					// Pin Chip Select
        .pin_rst = GPIO_NUM_15, 				// Pin Reset
        .spi_host = SPI2_HOST, 					// SPI port to use
        .contrast = 0x3F 					// Initial contrast (0-0x7F)
    };

    // 2. LCD initialization
    esp_err_t ret = lcd_nokia5110_init(&config, &lcd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LCD initialization error: %s", esp_err_to_name(ret));
        return;
    }

    // 3. Clear screen
    lcd_nokia5110_clear(lcd);

    // 4. Display basic information
    lcd_nokia5110_set_cursor(lcd, 0, 0);
    lcd_nokia5110_write_string(lcd, "Hola, bicilocos");

    lcd_nokia5110_set_cursor(lcd, 0, 1);
    lcd_nokia5110_write_string(lcd, "E-Bike");
    
    lcd_nokia5110_set_cursor(lcd, 0, 2);
    lcd_nokia5110_write_string(lcd, "Status: OK");
    
    lcd_nokia5110_set_cursor(lcd, 0, 3);
    lcd_nokia5110_write_string(lcd, "Velocity: 0 km/h");
    
    lcd_nokia5110_set_cursor(lcd, 0, 4);
    lcd_nokia5110_write_string(lcd, "Battery: 100%");

    // 5. Update screen
    lcd_nokia5110_update(lcd);

    // 6. Simple animation example
    uint8_t contrast = 0x3F;
    bool increasing = false;
    
    while (1) {
        // Change contrast for demostration
        if (increasing) {
            contrast++;
            if (contrast >= 0x7F) increasing = false;
        } else {
            contrast--;
            if (contrast <= 0x20) increasing = true;
        }
        
        lcd_nokia5110_set_contrast(lcd, contrast);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
