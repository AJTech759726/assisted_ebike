#ifndef LCD_NOKIA5110_PRIV_H
#define LCD_NOKIA5110_PRIV_H

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "font_5x7.h"

// Screen definitions
#define LCD_WIDTH            84
#define LCD_HEIGHT           48
#define LCD_ROWS            (LCD_HEIGHT/8) 				// 6 rows of 8 bits
#define LCD_BUFFER_SIZE     (LCD_WIDTH * LCD_ROWS)

// PCD8544 driver commands
#define CMD_FUNCTION_SET     0x20
#define CMD_DISPLAY_CONTROL  0x08
#define CMD_SET_Y_ADDR       0x40
#define CMD_SET_X_ADDR       0x80
#define CMD_TEMP_CONTROL     0x04
#define CMD_BIAS_SYSTEM      0x10
#define CMD_SET_VOP          0x80

// Complete internal structure of the LCD
typedef struct {
    spi_device_handle_t spi_dev; 				// Handle del dispositivo SPI
    spi_host_device_t spi_host;					// Host SPI
    gpio_num_t pin_dc; 						// Pin Data/Command
    gpio_num_t pin_reset; 					// Pin Reset
    gpio_num_t pin_cs; 						// Pin Chip Select (SPI)
    uint8_t contrast; 						// Contrast level (0-0x7F)
    uint8_t x_pos; 						// Current X position
    uint8_t y_pos; 						// Current Y position
    uint8_t framebuffer[LCD_ROWS][LCD_WIDTH]; 			// Screen buffer
    bool inverted; 						// Inverted display mode
} lcd_nokia5110_priv_t;

// Internal function prototypes
static esp_err_t lcd_send_byte(lcd_nokia5110_priv_t *lcd, uint8_t byte, bool is_data);
void lcd_write_command(lcd_nokia5110_priv_t *lcd, uint8_t cmd);
void lcd_write_data(lcd_nokia5110_priv_t *lcd, uint8_t data);
void lcd_reset(lcd_nokia5110_priv_t *lcd);
void lcd_initialize(lcd_nokia5110_priv_t *lcd);
void lcd_update_region(lcd_nokia5110_priv_t *lcd, uint8_t x_start, uint8_t y_start, 
                      uint8_t x_end, uint8_t y_end);

// Functions for characters handling
void lcd_draw_char(lcd_nokia5110_priv_t *lcd, char c);
void lcd_draw_string(lcd_nokia5110_priv_t *lcd, const char *str);

// Functions for graphics handling
void lcd_set_pixel(lcd_nokia5110_priv_t *lcd, uint8_t x, uint8_t y, bool state);
void lcd_draw_line(lcd_nokia5110_priv_t *lcd, uint8_t x0, uint8_t y0, 
                  uint8_t x1, uint8_t y1, bool state);
void lcd_draw_rect(lcd_nokia5110_priv_t *lcd, uint8_t x, uint8_t y, 
                  uint8_t width, uint8_t height, bool fill, bool state);

#endif 				// LCD_NOKIA5110_PRIV_H
