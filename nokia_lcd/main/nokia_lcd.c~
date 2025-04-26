#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "pcd8544.h"

// Declarar el handle del dispositivo SPI
static spi_device_handle_t spi;

// Función de inicialización del SPI
spi_device_handle_t init_spi() {
    spi_bus_config_t buscfg = {
        .mosi_io_num = MOSI,
        .sclk_io_num = SCLK,
        .miso_io_num = -1, // No se usa MISO en este caso
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = FRAME_BUFFER_SIZE
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4000000, // 4 MHz
        .mode = 0,                 // Modo SPI 0
        .spics_io_num = CS,        // Pin CS
        .queue_size = QUEUE_SIZE   // Tamaño de la cola SPI
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    spi_device_handle_t spi;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));
    return spi;
}

// Función principal
void app_main() {
    // Inicializar SPI
    spi = init_spi();

    // Inicializar el Nokia 5110
    init_pcd8544(spi);

    // Dar tiempo al LCD para inicializarse
    vTaskDelay(pdMS_TO_TICKS(100));

    // Limpiar el buffer de pantalla
    clear_frame_buffer();

    // Escribir "Hola bicilocos" en la pantalla
    write_string("Hola bicilocos", true, 0);

    // Dibujar el contenido del frame buffer en el Nokia 5110
    draw_frame_buffer(spi);

    // Entrar en un bucle infinito para mantener el programa activo
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
