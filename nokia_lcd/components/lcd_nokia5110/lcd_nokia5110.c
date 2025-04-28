#include "lcd_nokia5110.h"
#include "freertos/task.h"
#include "esp_log.h"

// Comandos del Nokia 5110
#define CMD_FUNCTION_SET     0x20
#define CMD_DISPLAY_CONTROL  0x08
#define CMD_SET_Y_ADDR       0x40
#define CMD_SET_X_ADDR       0x80
#define CMD_TEMP_CONTROL     0x04
#define CMD_BIAS_SYSTEM      0x10
#define CMD_SET_VOP          0x80

static const char *TAG = "LCD_NOKIA5110";

// Función interna para mandar un byte (comando o dato)
static esp_err_t lcd_send_byte(lcd_nokia5110_t *lcd, uint8_t byte, bool is_data)
{
    gpio_set_level(lcd->pin_dc, is_data); 				// 0 = comando, 1 = dato

    spi_transaction_t t = {
        .length = 8,    			// bits
        .tx_buffer = &byte
    };

    return spi_device_transmit(lcd->spi_handle, &t);
}

// Inicializar la pantalla
esp_err_t lcd_nokia5110_init(const lcd_nokia5110_config_t *config, lcd_nokia5110_t *lcd)
{
    ESP_LOGI(TAG, "Inicializando pantalla Nokia 5110");

    // 1. Configurar pines
    lcd->pin_dc = config->pin_dc;
    lcd->pin_reset = config->pin_reset;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << lcd->pin_dc) | (1ULL << lcd->pin_reset),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);

    // 2. Reset físico
    gpio_set_level(lcd->pin_reset, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(lcd->pin_reset, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    // 3. Configurar SPI
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4000000,           			// 4 MHz
        .mode = 0,                           			// SPI mode 0
        .spics_io_num = config->pin_cs,      			// CS pin
        .queue_size = 1,
    };

    esp_err_t ret = spi_bus_add_device(config->spi_host, &devcfg, &lcd->spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error agregando dispositivo SPI: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. Inicializar LCD
    lcd_send_byte(lcd, CMD_FUNCTION_SET | 0x01, false); 				// Instrucciones extendidas
    lcd_send_byte(lcd, CMD_SET_VOP | 0x3F, false);      				// Vop = contraste
    lcd_send_byte(lcd, CMD_TEMP_CONTROL | 0x02, false); 				// Control de temperatura
    lcd_send_byte(lcd, CMD_BIAS_SYSTEM | 0x03, false);  				// Bias System
    lcd_send_byte(lcd, CMD_FUNCTION_SET, false);        				// Instrucciones básicas
    lcd_send_byte(lcd, CMD_DISPLAY_CONTROL | 0x04, false); 				// Display normal

    // 5. Limpiar pantalla
    lcd_nokia5110_clear(lcd);

    return ESP_OK;
}

// Escribir un texto básico
void lcd_nokia5110_write_text(lcd_nokia5110_t *lcd, const char *text)
{
    while (*text) {
        lcd_send_byte(lcd, *text++, true); 				// Manda cada carácter como dato
    }
}

// Limpiar la pantalla
void lcd_nokia5110_clear(lcd_nokia5110_t *lcd)
{
    lcd_send_byte(lcd, CMD_SET_X_ADDR, false);
    lcd_send_byte(lcd, CMD_SET_Y_ADDR, false);

    for (int i = 0; i < (84 * 6); i++) { 				// 504 bytes = 84x48 pixels
        lcd_send_byte(lcd, 0x00, true);
    }
}
