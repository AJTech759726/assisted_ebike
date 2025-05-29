#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define RCWL_GPIO_1 GPIO_NUM_33				// Pin salida del RCWL
#define RCWL_GPIO_2 GPIO_NUM_25				// Pin salida del RCWL
#define LED_PIN GPIO_NUM_2				// Pin LED

void app_main(void)
{
	// Configurar el pin del RCWL como entrada
	gpio_config_t io_conf1 = {
		.pin_bit_mask = (1ULL << RCWL_GPIO_1),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	
	gpio_config(&io_conf1);


	gpio_config_t io_conf2 = {
		.pin_bit_mask = (1ULL << RCWL_GPIO_2),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	
	gpio_config(&io_conf2);

	// Configurar el pin del LED como salida
	esp_rom_gpio_pad_select_gpio(LED_PIN);
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

	while(1) {
		int motion_detected1 = gpio_get_level(RCWL_GPIO_1);
		int motion_detected2 = gpio_get_level(RCWL_GPIO_2);

		if (motion_detected1 || motion_detected2) {
			gpio_set_level(LED_PIN, 1);				// Enciende LED si hay presencia
		} else {
			gpio_set_level(LED_PIN, 0);				// Apaga LED si no hay presencia
		}

		vTaskDelay(pdMS_TO_TICKS(100));					// Espera 100ms
	}

}
