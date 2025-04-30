#ifndef LCD_NOKIA5110_H
#define LCD_NOKIA5110_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C" {
#endif

// Tipo opaco para encapsulamiento
typedef struct lcd_nokia5110_priv_t *lcd_nokia5110_t;				// Puntero opaco

// Configuración de inicialización
typedef struct {
    gpio_num_t pin_sclk; 				// Pin CLK SPI
    gpio_num_t pin_din; 				// Pin MOSI SPI
    gpio_num_t pin_dc; 					// Pin Data/Command
    gpio_num_t pin_cs; 					// Pin Chip Select
    gpio_num_t pin_rst; 				// Pin Reset
    spi_host_device_t spi_host; 			// Puerto SPI (SPI2_HOST, etc.)
    uint8_t contrast; 					// Nivel de contraste (0-0x7F)
} lcd_nokia5110_config_t;

// Funciones públicas
esp_err_t lcd_nokia5110_init(const lcd_nokia5110_config_t *config, lcd_nokia5110_t *lcd);
void lcd_nokia5110_deinit(lcd_nokia5110_t lcd);

// Control de pantalla
void lcd_nokia5110_clear(lcd_nokia5110_t lcd);
void lcd_nokia5110_update(lcd_nokia5110_t lcd);
void lcd_nokia5110_set_contrast(lcd_nokia5110_t lcd, uint8_t contrast);
void lcd_nokia5110_invert(lcd_nokia5110_t lcd, bool invert);

// Funciones de texto
void lcd_nokia5110_set_cursor(lcd_nokia5110_t lcd, uint8_t x, uint8_t y);
void lcd_nokia5110_write_char(lcd_nokia5110_t lcd, char c);
void lcd_nokia5110_write_string(lcd_nokia5110_t lcd, const char *str);

// Funciones gráficas
void lcd_nokia5110_draw_pixel(lcd_nokia5110_t lcd, uint8_t x, uint8_t y, bool on);
void lcd_nokia5110_draw_line(lcd_nokia5110_t lcd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on);
void lcd_nokia5110_draw_rect(lcd_nokia5110_t lcd, uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool fill, bool on);
void lcd_nokia5110_draw_bitmap(lcd_nokia5110_t lcd, uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t w, uint8_t h);

#ifdef __cplusplus
}
#endif

#endif 				// LCD_NOKIA5110_H
