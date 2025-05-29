#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"

// Configuración de pines según tu imagen
#define PWM_PIN          GPIO_NUM_17      // VSP (Velocidad/PWM)
#define DIR_PIN          GPIO_NUM_16      // CW/CCW (Dirección)
#define ENABLE_PIN       GPIO_NUM_4       // Opcional, si necesitas habilitar

// Configuración PWM
#define PWM_CHANNEL      LEDC_CHANNEL_0
#define PWM_TIMER        LEDC_TIMER_0
#define PWM_FREQ         25000           // 25kHz (frecuencia típica para BLDC)
#define PWM_RESOLUTION   LEDC_TIMER_10_BIT // 10-bit (0-1023)

void init_motor_control() {
    // Configurar PWM para control de velocidad
    ledc_timer_config_t pwm_timer = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = PWM_RESOLUTION,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&pwm_timer);

    ledc_channel_config_t pwm_channel = {
        .gpio_num = PWM_PIN,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = PWM_CHANNEL,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&pwm_channel);

    // Configurar pin de dirección (opcional)
    gpio_set_direction(DIR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DIR_PIN, 1); // Establece dirección (1=CW, 0=CCW)

    // Configurar pin de enable si es necesario
    gpio_set_direction(ENABLE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(ENABLE_PIN, 1); // Habilita el driver
}

void set_motor_speed(uint32_t speed) {
    speed = (speed > 1023) ? 1023 : speed; // Limitar a resolución máxima
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL, speed);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, PWM_CHANNEL);
    printf("Speed set: %"PRIu32"/1023\n", speed);
}

void app_main() {
    init_motor_control();
    
    printf("Starting motor test...\n");
    
    // Secuencia de arranque seguro
    printf("Soft start...\n");
    for(uint32_t speed = 600; speed <= 900; speed += 50) {
        set_motor_speed(speed);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Espera 1 segundo entre incrementos
    }
    
    printf("Maintaining medium speed for 5 seconds...\n");
    set_motor_speed(900);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    printf("Stopping motor...\n");
    set_motor_speed(0);
    
    printf("Test completed.\n");
}
