#include "lcd_nokia5110.h"

lcd_nokia5110_t lcd;

void app_main(void)
{
    lcd_nokia5110_config_t config = {
        .pin_dc = GPIO_NUM_4,
        .pin_reset = GPIO_NUM_15,
        .spi_host = SPI2_HOST,
        .pin_cs = GPIO_NUM_5
    };

    // 🚀 Inicializar pantalla
    lcd_nokia5110_init(&config, &lcd);

    // 📢 Mostrar texto
    lcd_nokia5110_write_text(&lcd, "Hola Mundo!");
}
