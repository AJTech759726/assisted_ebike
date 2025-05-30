#include <esp_log.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "rc522.h"
#include "driver/rc522_spi.h"
#include "rc522_picc.h"

// Hardware configuration
#define RCWL_GPIO		GPIO_NUM_33				// Movement sensor RCWL-0516

// RFID RC522 TAG Reader configuration
#define RC522_SPI_HOST		SPI3_HOST				// RFID RC522 TAG Reader
#define RC522_MISO_GPIO		GPIO_NUM_19
#define RC522_MOSI_GPIO		GPIO_NUM_23
#define RC522_SCLK_GPIO		GPIO_NUM_18
#define RC522_SDA_GPIO		GPIO_NUM_5
#define RC522_RST_GPIO		GPIO_NUM_4				// soft-reset

#define BLIND_SPOT_LIGHT_GPIO	GPIO_NUM_25				// Blind spot lights
#define SYSTEM_ACTIVE_GPIO	GPIO_NUM_2				// System active indicator LED
#define POWER_RELAY_GPIO	GPIO_NUM_21				// Relay activates electrical system
#define EMERGENCY_BUTTON_GPIO	GPIO_NUM_27				// Emergency stop button

static bool system_activated = false;

// RFID Authorized tag
static rc522_spi_config_t driver_config = {
	.host_id = RC522_SPI_HOST,
	.bus_config = &(spi_bus_config_t) {
		.miso_io_num = RC522_MISO_GPIO,
		.mosi_io_num = RC522_MOSI_GPIO,
		.sclk_io_num = RC522_SCLK_GPIO,
	},

	.dev_config = {
		.spics_io_num = RC522_SDA_GPIO,
	},

	.rst_io_num = RC522_RST_GPIO,
};

static rc522_handle_t scanner;
static rc522_driver_handle_t driver;
static bool waiting_tag = true;				// Initial state: waiting for tag to activate

// Callback RFID
static void on_rfid_detection(void *arg, esp_event_base_t base,
				int32_t event_id, void *data) {
	rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
	rc522_picc_t *picc = event->picc;

	if (waiting_tag && picc->state == RC522_PICC_STATE_ACTIVE) {
		ESP_LOGI("RFID", "Authorized TAG detected - Activating system");
		system_activated = true;
		waiting_tag = false;
		gpio_set_level(POWER_RELAY_GPIO, 1);				// Activate electrical system
		gpio_set_level(SYSTEM_ACTIVE_GPIO, 1);
	}
}
	
void setup_gpio() {
	// INPUT Configuration
	gpio_config_t input_conf = {
		.pin_bit_mask = (1ULL << RCWL_GPIO) | (1ULL << EMERGENCY_BUTTON_GPIO),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,				// Use pull-up for the button
		.intr_type = GPIO_INTR_DISABLE
	};

	gpio_config(&input_conf);

	// OUTPUT Configuration
	gpio_config_t output_conf = {
		.pin_bit_mask = (1ULL << BLIND_SPOT_LIGHT_GPIO) |
				(1ULL << SYSTEM_ACTIVE_GPIO) |
				(1ULL << POWER_RELAY_GPIO),
		.mode = GPIO_MODE_OUTPUT
	};

	gpio_config(&output_conf);
}

void shutdown_system() {
	ESP_LOGI("SYSTEM", "Shutting down due to emergency");
	system_activated = false;
	waiting_tag = true;
	gpio_set_level(POWER_RELAY_GPIO, 0);
	gpio_set_level(SYSTEM_ACTIVE_GPIO, 0);
	gpio_set_level(BLIND_SPOT_LIGHT_GPIO, 0);
}

void app_main(void)
{
	ESP_LOGI("SYSTEM", "EBicycle initializing");

	rc522_spi_create(&driver_config, &driver);
	rc522_driver_install(driver);

	rc522_config_t scanner_config = {
		.driver = driver,
	};

	rc522_create(&scanner_config, &scanner);
	rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED,
				on_rfid_detection, NULL);
	rc522_start(scanner);

	// Setup GPIOs
	setup_gpio();

	// Initial state
	shutdown_system();

	while(1) {
		// Verify emergency button
		if (gpio_get_level(EMERGENCY_BUTTON_GPIO) == 0) {
			vTaskDelay(pdMS_TO_TICKS(50));				// Debounce
			
			if (gpio_get_level(EMERGENCY_BUTTON_GPIO) == 0) {
				shutdown_system();
				
				while(gpio_get_level(EMERGENCY_BUTTON_GPIO) == 0){
					vTaskDelay(pdMS_TO_TICKS(100));
				}

				ESP_LOGI("SYSTEM", "Waiting authorized TAG...");
			}
		}
		
		// Blind spot control
		if (system_activated) {
			bool movement = gpio_get_level(RCWL_GPIO);
			gpio_set_level(BLIND_SPOT_LIGHT_GPIO, movement);

			if (movement) {
				ESP_LOGI("SENSOR", "Blind spot detected!");
			}
		}

		vTaskDelay(pdMS_TO_TICKS(1000));
	}

}
