#include "driver/i2c.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

static const char *TAG = "oled_test";

void app_main(void)
{
    // 1. Configurar bus I2C
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    i2c_param_config(I2C_MASTER_NUM, &i2c_conf);
    i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, 0, 0, 0);

    // 2. Crear IO handle para el panel SSD1306 por I2C
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = 0x3D,  // Dirección típica del SSD1306
        .control_phase_bytes = 1,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_bit_offset = 6,
        .flags = {
            .dc_low_on_data = false,  // El bit DC en alto indica data
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(I2C_MASTER_NUM, &io_config, &io_handle));

    // 3. Configuración específica para el SSD1306
    esp_lcd_panel_ssd1306_config_t vendor_config = {
        .height = OLED_HEIGHT
    };

    // 4. Configuración general del panel
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,
        .color_space = ESP_LCD_COLOR_SPACE_MONOCHROME,
        .bits_per_pixel = 1,
        .vendor_config = &vendor_config
    };

    // 5. Crear panel
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_err_t err = esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel: %s", esp_err_to_name(err));
        return;
    }

    // 6. Inicializar y mostrar algo
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_disp_on_off(panel_handle, true);
    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, OLED_WIDTH, OLED_HEIGHT, NULL);  // Limpia pantalla
    vTaskDelay(pdMS_TO_TICKS(100));
}
