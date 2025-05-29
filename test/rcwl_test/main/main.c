#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define RCWL_GPIO GPIO_NUM_34				// Pin salida del RCWL
#define LED_PIN GPIO_NUM_2				// Pin LED

void app_main(void)
{
	// Configurar el pin del RCWL como entrada
	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << RCWL_GPIO),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	
	gpio_config(&io_conf);

	// Configurar el pin del LED como salida
	esp_rom_gpio_pad_select_gpio(LED_PIN);
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

	while(1) {
		int motion_detected = gpio_get_level(RCWL_GPIO);
		gpio_set_level(LED_PIN, motion_detected);				// Enciende LED si hay presencia
		
		vTaskDelay(pdMS_TO_TICKS(100));					// Espera 100ms
	}

}
