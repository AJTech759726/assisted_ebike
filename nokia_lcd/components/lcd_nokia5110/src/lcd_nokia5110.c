#include "lcd_nokia5110.h"
#include "lcd_nokia5110_priv.h"
#include "font_5x7.h"
#include "esp_log.h"
#include "freertos/task.h"

static const char *TAG = "NOKIA5110";

// Internal function to send bytes via SPI
static esp_err_t lcd_send_byte(lcd_nokia5110_priv_t *lcd, uint8_t byte, bool is_data) {
    gpio_set_level(lcd->pin_dc, is_data ? 1 : 0);
    
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &byte,
        .flags = SPI_TRANS_USE_TXDATA
    };
    
    return spi_device_polling_transmit(lcd->spi_dev, &t);
}

void lcd_write_command(lcd_nokia5110_priv_t *lcd, uint8_t cmd) {
	lcd_send_byte(lcd, cmd, false);
}

void lcd_write_data(lcd_nokia5110_priv_t *lcd, uint8_t data) {
	lcd_send_byte(lcd, data, true);
}

// LCD initialization
esp_err_t lcd_nokia5110_init(const lcd_nokia5110_config_t *config, lcd_nokia5110_t *lcd) {
    esp_err_t ret;
    lcd_nokia5110_priv_t *priv = calloc(1, sizeof(lcd_nokia5110_priv_t));
    if (!priv) return ESP_ERR_NO_MEM;

    // GPIO pins configuration
    priv->pin_dc = config->pin_dc;
    priv->pin_reset = config->pin_rst;
    priv->pin_cs = config->pin_cs;
    priv->contrast = config->contrast;
    priv->inverted = false;
    priv->spi_host = config->spi_host;

    // Pins configuration
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << priv->pin_dc) | (1ULL << priv->pin_reset),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    // Reset hardware
    gpio_set_level(priv->pin_reset, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(priv->pin_reset, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // SPI configuration
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = config->pin_din,
        .sclk_io_num = config->pin_sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4 * 1000 * 1000, 				// 4 MHz
        .mode = 0, 							// SPI mode 0
        .spics_io_num = priv->pin_cs,
        .queue_size = 7,
    };

    // Initialize SPI bus and device
    ret = spi_bus_initialize(config->spi_host, &buscfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI init failed: %s", esp_err_to_name(ret));
        free(priv);
        return ret;
    }

    ret = spi_bus_add_device(config->spi_host, &devcfg, &priv->spi_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI add device failed: %s", esp_err_to_name(ret));
        free(priv);
        return ret;
    }

    // Initialize screen
    lcd_send_byte(priv, CMD_FUNCTION_SET | 0x01, false); 				// Extended instructions
    lcd_send_byte(priv, CMD_SET_VOP | priv->contrast, false); 				// Contrast
    lcd_send_byte(priv, CMD_TEMP_CONTROL | 0x02, false); 				// Temp coefficient
    lcd_send_byte(priv, CMD_BIAS_SYSTEM | 0x03, false); 				// Bias mode
    lcd_send_byte(priv, CMD_FUNCTION_SET, false); 					// Basic instructions
    lcd_send_byte(priv, CMD_DISPLAY_CONTROL | 0x04, false); 				// Normal mode

    // Clear screen
    lcd_nokia5110_clear((lcd_nokia5110_t)priv);
    
    *lcd = (lcd_nokia5110_t)priv;
    ESP_LOGI(TAG, "LCD initialized successfully");
    return ESP_OK;
}

// Implementation of public functions

void lcd_nokia5110_clear(lcd_nokia5110_t lcd) {
    lcd_nokia5110_priv_t *priv = (lcd_nokia5110_priv_t *)lcd;
    memset(priv->framebuffer, 0, sizeof(priv->framebuffer));
    priv->x_pos = 0;
    priv->y_pos = 0;
}

void lcd_nokia5110_write_char(lcd_nokia5110_t lcd, char c) {
    lcd_nokia5110_priv_t *priv = (lcd_nokia5110_priv_t *)lcd;
    
    if (c < 32 || c > 127) c = ' ';
    
    for (uint8_t i = 0; i < 5; i++) {
        if (priv->x_pos < LCD_WIDTH) {
            priv->framebuffer[priv->y_pos][priv->x_pos] = 
                priv->inverted ? ~font_5x7[c - 32][i] : font_5x7[c - 32][i];
            priv->x_pos++;
        }
    }
    
    // Space between characters
    if (priv->x_pos < LCD_WIDTH) {
        priv->framebuffer[priv->y_pos][priv->x_pos] = priv->inverted ? 0xFF : 0x00;
        priv->x_pos++;
    }
}

void lcd_nokia5110_write_string(lcd_nokia5110_t lcd, const char *str) {
	while (*str) {
		lcd_nokia5110_write_char(lcd, *str++);
	}
}

void lcd_nokia5110_set_cursor(lcd_nokia5110_t lcd, uint8_t x, uint8_t y) {
	lcd_nokia5110_priv_t *priv = (lcd_nokia5110_priv_t *)lcd;
	priv->x_pos = x;
	priv->y_pos = y;
}

void lcd_nokia5110_update(lcd_nokia5110_t lcd) {
	lcd_nokia5110_priv_t *priv = (lcd_nokia5110_priv_t *)lcd;
	for (uint8_t row = 0; row < LCD_ROWS; row++) {
		lcd_write_command(priv, CMD_SET_Y_ADDR | row);
		lcd_write_command(priv, CMD_SET_X_ADDR | 0);
		for (uint8_t col = 0; col < LCD_WIDTH; col++) {
			lcd_write_data(priv, priv->framebuffer[row][col]);
		}
	}
}

void lcd_nokia5110_set_contrast(lcd_nokia5110_t lcd, uint8_t contrast) {
	lcd_nokia5110_priv_t *priv = (lcd_nokia5110_priv_t *)lcd;
	priv->contrast = contrast;
	lcd_write_command(priv, CMD_FUNCTION_SET | 0x01); 				// Ext. instr.
	lcd_write_command(priv, CMD_SET_VOP | contrast);
	lcd_write_command(priv, CMD_FUNCTION_SET); 					// Basic instr.
}

void lcd_nokia5110_deinit(lcd_nokia5110_t lcd) {
	if (lcd) {
		lcd_nokia5110_priv_t *priv = (lcd_nokia5110_priv_t *)lcd;

		// 1. Remove the SPI device
		if (priv->spi_dev) {
			spi_bus_remove_device(priv->spi_dev);
		}

		// 2. Release the SPI bus (usign the host)
		if (priv->spi_host) {
			spi_bus_free(priv->spi_host);
		}

		// 3. Free memory
		free(priv);
	}
}
