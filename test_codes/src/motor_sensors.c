#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"

// PWN configuration
#define PWM_GPIO		GPIO_NUM_18
#define PWM_CHANNEL		LEDC_CHANNEL_0
#define PWM_TIMER		LEDC_TIMER_0
#define PWM_FREQ		25000				// 25kHz
#define PWM_RESOLUTION		LEDC_TIMER_10_BIT

// Hall sensors configuration
#define HALL_U_GPIO		GPIO_NUM_32
#define HALL_V_GPIO		GPIO_NUM_33
#define HALL_W_GPIO		GPIO_NUM_34
#define HALL_VCC_GPIO		GPIO_NUM_25				// For sensors supply
#define HALL_GND_GPIO		GPIO_NUM_26

void init_pwm() {
	ledc_timer_config_t timer_cfg = {
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.duty_resolution = PWM_RESOLUTION,
		.timer_num = PWM_TIMER,
		.freq_hz = PWM_FREQ,
		.clk_cfg = LEDC_AUTO_CLK
	};

	ledc_timer_config(&timer_cfg);

	ledc_channel_config_t channel_cfg = {
		.gpio_num = PWM_GPIO,
		.speed_mode = LEDC_HIGH_SPEED_MODE,
		.channel = PWM_CHANNEL,
		.timer_sel = PWM_TIMER,
		.duty = 0,
		.hpoint = 0
	};

	ledc_channel_config(&channel_cfg);
}

void init_hall_sensors() {
	// Configure Hall sensors GPIOs as inputs
	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << HALL_U_GPIO) | (1ULL << HALL_V_GPIO) | (1ULL << HALL_W_GPIO),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};

	gpio_config(&io_conf);

	// Optional: Hall sensor supply by GPIO
	gpio_set_direction(HALL_VCC_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_level(HALL_VCC_GPIO, 1);				// +3.3V from sensors
}

uint32_t read_hall_sensors() {
	return(gpio_get_level(HALL_W_GPIO) << 2) |
		(gpio_get_level(HALL_V_GPIO) << 1) |
		gpio_get_level(HALL_U_GPIO);
}

void set_motor_speed(uint32_t duty) {
	duty = (duty > 1023) ? 1023 : duty;
	ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, duty);
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
}

void app_main(void)
{
	init_pwm();
	init_hall_sensors();

	// Boot sequence
	printf("Iniciando motor...\n");

	// Soft start
	for(int speed = 0; speed < 500; speed += 10) {
		set_motor_speed(speed);
		printf("Speed: &d, Hall: &d\n", speed, read_hall_sensors());
		vTaskDelay(pdMS_TO_TICKS(50));
	}

	// Main loop
	while (1) {
		// Hall sensors lecture for monitoring
		uint32_t hall_state = read_hall_sensors();
		printf("Estado sensores Hall: &d\n", hall_state);

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
