#ifndef LCD_NOKIA5110_PRIV_H
#define LCD_NOKIA5110_PRIV_H

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "font_5x7.h"

// Definiciones de la pantalla
#define LCD_WIDTH            84
#define LCD_HEIGHT           48
#define LCD_ROWS            (LCD_HEIGHT/8) 				// 6 filas de 8 bits
#define LCD_BUFFER_SIZE     (LCD_WIDTH * LCD_ROWS)

// Comandos del controlador PCD8544
#define CMD_FUNCTION_SET     0x20
#define CMD_DISPLAY_CONTROL  0x08
#define CMD_SET_Y_ADDR       0x40
#define CMD_SET_X_ADDR       0x80
#define CMD_TEMP_CONTROL     0x04
#define CMD_BIAS_SYSTEM      0x10
#define CMD_SET_VOP          0x80

// Estructura interna completa del LCD
typedef struct {
    spi_device_handle_t spi_dev; 				// Handle del dispositivo SPI
    spi_host_device_t spi_host;					// Host SPI
    gpio_num_t pin_dc; 						// Pin Data/Command
    gpio_num_t pin_reset; 					// Pin Reset
    gpio_num_t pin_cs; 						// Pin Chip Select (SPI)
    uint8_t contrast; 						// Nivel de contraste (0-0x7F)
    uint8_t x_pos; 						// Posición X actual
    uint8_t y_pos; 						// Posición Y actual
    uint8_t framebuffer[LCD_ROWS][LCD_WIDTH]; 			// Buffer de pantalla
    bool inverted; 						// Modo de visualización invertido
} lcd_nokia5110_priv_t;

// Prototipos de funciones internas
static esp_err_t lcd_send_byte(lcd_nokia5110_priv_t *lcd, uint8_t byte, bool is_data);
void lcd_write_command(lcd_nokia5110_priv_t *lcd, uint8_t cmd);
void lcd_write_data(lcd_nokia5110_priv_t *lcd, uint8_t data);
void lcd_reset(lcd_nokia5110_priv_t *lcd);
void lcd_initialize(lcd_nokia5110_priv_t *lcd);
void lcd_update_region(lcd_nokia5110_priv_t *lcd, uint8_t x_start, uint8_t y_start, 
                      uint8_t x_end, uint8_t y_end);

// Funciones para manejo de caracteres
void lcd_draw_char(lcd_nokia5110_priv_t *lcd, char c);
void lcd_draw_string(lcd_nokia5110_priv_t *lcd, const char *str);

// Funciones para manejo de gráficos
void lcd_set_pixel(lcd_nokia5110_priv_t *lcd, uint8_t x, uint8_t y, bool state);
void lcd_draw_line(lcd_nokia5110_priv_t *lcd, uint8_t x0, uint8_t y0, 
                  uint8_t x1, uint8_t y1, bool state);
void lcd_draw_rect(lcd_nokia5110_priv_t *lcd, uint8_t x, uint8_t y, 
                  uint8_t width, uint8_t height, bool fill, bool state);

#endif 				// LCD_NOKIA5110_PRIV_H
