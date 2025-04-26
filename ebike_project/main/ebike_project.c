#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "rc522.h"
#include "pcd8544.h"

// Pines
#define RCWL_PIN		GPIO_NUM_34
#define LED_PIN			GPIO_NUM_2

#define RFID_CS_PIN		GPIO_NUM_5
#define RFID_SC_PIN		GPIO_NUM_22
#define RFID_RST_PIN		GPIO_NUM_20

#define LCD_RST_PIN		GPIO_NUM_21
#define LCD_CE_PIN		GPIO_NUM_15
#define LCD_DC_PIN		GPIO_NUM_17
#define LCD_DIN_PIN		GPIO_NUM_23
#define LCD_CLK_PIN		GPIO_NUM_18
#define LCD_BL_PIN		GPIO_NUM_4

void rcwl_task(void *pvParameter) {
	esp_rom_gpio_pad_select_gpio(RCWL_PIN);
	gpio_set_direction(RCWL_PIN, GPIO_MODE_INPUT);
	
	gpio_pad_select_gpio(LED_PIN);
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

	while(1) {
		int presence = gpio_get_level(RCWL_PIN);
		gpio_set_level(LED_PIN, presence);
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void app_main(void)
{
	// Inicializar RCWL_0516
	xTaskCreate(&rcwl_task, "rcwl_task", 2048, NULL, 5, NULL);

	// Inicializar RFID RC522
	rc522_config_t config = {
		.spi.host = HSPI_HOST,
		.spi.miso_gpio = GPIO_NUM_19,
		.spi.mosi_gpio = GPIO_NUM_23,
		.spi.sck_gpio = GPIO_NUM_18,
		.spi.sda_gpio = RFID_CS_PIN,
		.rst_gpio = RFID_RST_PIN,
	};
	rc522_init(&config);

	// Inicializar pantalla Nokia 5110
	pcd8544_config_t lcd_config = {
		.rst_pin = LCD_RST_PIN,
		.ce_pin = LCD_CE_PIN,
		.dc_pin = LCD_DC_PIN,
		.din_pin = LCD_DIN_PIN,
		.clk_pin = LCD_CLK_PIN,
		.bl_pin = LCD_BL_PIN,
	};
	pcd8544_init(&lcd_config);
	pcd8544_clear();
	pcd8544_draw_string(0, 0, "Sistema iniciado");
	pcd8544_refresh();

	while(1) {
		// Lectura de tarjeta RFID
		uint8_t uid[10];
		uint8_t uid_len;
		if (rc522_read_card_serial(uid, &uid_len) == ESP_OK) {
			printf("Tarjeta detectada: ");
			for (int i = 0; i < uid_len; i++) {
				printf("%02X ", uid[i]);
			}
			printf("\n");

			// Mostrar en pantalla
			pcd8544_clear();
			pcd8544_draw_string(0, 0, "Tarjeta:");
			char uid_str[30];
			snprintf(uid_str, sizeof(uid_str), "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
			pcd8544_draw_string(0, 1, uid_str);
			pcd8544_refresh();
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}

}
