#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "rc522.h"

#define LED_GPIO		GPIO_NUM_2

void app_main(void)
{
	// Configurar el pin del LED como salida
	gpio_reset_pin(LED_GPIO);
	gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(LED_GPIO, 0);

	// Configuración el RC522
	rc522_config_t config = {
		.spi_host_id 		= SPI2_HOST,
		.spi_miso_gpio 	= GPIO_NUM_19,
		.spi_mosi_gpio 	= GPIO_NUM_23,
		.spi_sck_gpio 		= GPIO_NUM_18,
		.spi_cs_gpio 		= GPIO_NUM_5,
		.rst_gpio 		= GPIO_NUM_22,
	};

	rc522_handle_t scanner;
	rc522_create(&config, &scanner);
	rc522_start(scanner);

	while(1) {
		rc522_tag_t tag;
		
		if (rc522_read_tag_(scanner, &tag) == ESP_OK) {
			gpio_set_level(LED_GPIO, 1);				// Enciender LED

		} else {
			gpio_set_level(LED_GPIO, 0);				// Apaga LED
		}

		vTaskDelay(pdMS_TO_TICKS(500));

}
