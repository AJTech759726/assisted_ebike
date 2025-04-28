#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// 📍 Configuración de pines (puedes cambiarlos si quieres)
typedef struct {
    gpio_num_t pin_dc;      // Data/Command
    gpio_num_t pin_reset;   // Reset
    spi_host_device_t spi_host; // SPI host (ej: SPI2_HOST)
    gpio_num_t pin_cs;      // Chip Select
} lcd_nokia5110_config_t;

// 📍 Handle de la pantalla (estructura interna)
typedef struct {
    spi_device_handle_t spi_handle;
    gpio_num_t pin_dc;
    gpio_num_t pin_reset;
} lcd_nokia5110_t;

// 📍 Funciones que vamos a implementar
esp_err_t lcd_nokia5110_init(const lcd_nokia5110_config_t *config, lcd_nokia5110_t *lcd);
void lcd_nokia5110_write_text(lcd_nokia5110_t *lcd, const char *text);
void lcd_nokia5110_clear(lcd_nokia5110_t *lcd);

#ifdef __cplusplus
}
#endif
