#include "lcd_nokia5110.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "LCD_MAIN"

void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando aplicación con pantalla Nokia 5110");

    lcd_nokia5110_t lcd = NULL; 				// Usamos puntero

    // 1. Configuración inicial del LCD
    lcd_nokia5110_config_t config = {
        .pin_sclk = GPIO_NUM_18, 				// Pin CLK SPI
        .pin_din = GPIO_NUM_23, 				// Pin MOSI SPI
        .pin_dc = GPIO_NUM_4, 					// Pin Data/Command
        .pin_cs = GPIO_NUM_5, 					// Pin Chip Select
        .pin_rst = GPIO_NUM_15, 				// Pin Reset
        .spi_host = SPI2_HOST, 					// Puerto SPI a usar
        .contrast = 0x3F 					// Contraste inicial (0-0x7F)
    };

    // 2. Inicialización del LCD
    esp_err_t ret = lcd_nokia5110_init(&config, &lcd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al inicializar LCD: %s", esp_err_to_name(ret));
        return;
    }

    // 3. Limpiar pantalla
    lcd_nokia5110_clear(lcd);

    // 4. Mostrar información básica
    lcd_nokia5110_set_cursor(lcd, 0, 0);
    lcd_nokia5110_write_string(lcd, "Hola bicilocos");

    lcd_nokia5110_set_cursor(lcd, 0, 1);
    lcd_nokia5110_write_string(lcd, "Bici Electrica");
    
    lcd_nokia5110_set_cursor(lcd, 0, 2);
    lcd_nokia5110_write_string(lcd, "Estado: OK");
    
    lcd_nokia5110_set_cursor(lcd, 0, 3);
    lcd_nokia5110_write_string(lcd, "Velocidad: 0 km/h");
    
    lcd_nokia5110_set_cursor(lcd, 0, 4);
    lcd_nokia5110_write_string(lcd, "Bateria: 100%");

    // 5. Actualizar pantalla
    lcd_nokia5110_update(lcd);

    // 6. Ejemplo de animación simple
    uint8_t contrast = 0x3F;
    bool increasing = false;
    
    while (1) {
        // Cambiar contraste para demostración
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
