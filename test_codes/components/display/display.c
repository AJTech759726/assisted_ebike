#include "display.h"
#include "driver/spi_master.h"
#include "esp_log.h"

#define TAG "DISPLAY"

// PCD8544 (Nokia 5110) commands
#define PCD8544_FUNCTIONSET 0x20
#define PCD8544_DISPLAYCONTROL 0x08
#define PCD8544_SETYADDR 0x40
#define PCD8544_SETXADDR 0x80

static spi_device_handle_t spi;
static uint8_t buffer[504]; // 84x48 / 8

void display_init() {
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = GPIO_NUM_23,
        .sclk_io_num = GPIO_NUM_18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0
    };
    
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 4 * 1000 * 1000, // 4MHz
        .mode = 0,
        .spics_io_num = GPIO_NUM_5,
        .queue_size = 7,
        .pre_cb = NULL
    };
    
    // Initialize SPI bus
    spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    
    // Initialize display
    gpio_set_direction(GPIO_NUM_17, GPIO_MODE_OUTPUT); // RST
    gpio_set_direction(GPIO_NUM_16, GPIO_MODE_OUTPUT); // DC
    
    // Reset display
    gpio_set_level(GPIO_NUM_17, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(GPIO_NUM_17, 1);
    
    // Send initialization commands
    gpio_set_level(GPIO_NUM_16, 0); // Command mode
    
    uint8_t init_commands[] = {
        PCD8544_FUNCTIONSET | 0x01, // Extended instruction set
        0xBF, // Set Vop (contrast)
        0x14, // Temperature coefficient
        0x20, // Basic instruction set, horizontal addressing
        0x0C  // Display control: normal mode
    };
    
    spi_transaction_t t = {
        .length = sizeof(init_commands) * 8,
        .tx_buffer = init_commands
    };
    spi_device_polling_transmit(spi, &t);
    
    // Clear display
    memset(buffer, 0, sizeof(buffer));
    display_update();
}

void display_update() {
    gpio_set_level(GPIO_NUM_16, 0); // Command mode
    uint8_t cmd = PCD8544_SETYADDR;
    spi_transaction_t t1 = {
        .length = 8,
        .tx_buffer = &cmd
    };
    spi_device_polling_transmit(spi, &t1);
    
    cmd = PCD8544_SETXADDR;
    spi_transaction_t t2 = {
        .length = 8,
        .tx_buffer = &cmd
    };
    spi_device_polling_transmit(spi, &t2);
    
    gpio_set_level(GPIO_NUM_16, 1); // Data mode
    spi_transaction_t t3 = {
        .length = sizeof(buffer) * 8,
        .tx_buffer = buffer
    };
    spi_device_polling_transmit(spi, &t3);
}

// Additional display functions would be implemented here
// (text rendering, graphics, etc.)
