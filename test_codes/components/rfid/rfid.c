#include "rfid.h"
#include "mfrc522.h"
#include "esp_log.h"

#define TAG "RFID"

static mfrc522_handle_t scanner;
static rfid_callback_t user_callback = NULL;

void rfid_init(rfid_callback_t callback) {
    user_callback = callback;
    
    // Initialize SPI bus (if not already done by display)
    spi_bus_config_t buscfg = {
        .miso_io_num = GPIO_NUM_19,
        .mosi_io_num = GPIO_NUM_23,
        .sclk_io_num = GPIO_NUM_18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0
    };
    
    spi_bus_initialize(VSPI_HOST, &buscfg, 1);
    
    // Configure MFRC522
    mfrc522_config_t config = {
        .spi_host = VSPI_HOST,
        .miso_io = GPIO_NUM_19,
        .mosi_io = GPIO_NUM_23,
        .sck_io = GPIO_NUM_18,
        .cs_io = GPIO_NUM_21,
        .rst_io = GPIO_NUM_15,
        .max_transfer_sz = 0
    };
    
    scanner = mfrc522_init(&config);
    mfrc522_pcd_reset(scanner);
    mfrc522_pcd_init(scanner);
    
    xTaskCreate(rfid_task, "rfid_task", 4096, NULL, 5, NULL);
}

void rfid_task(void *pvParameters) {
    uint8_t uid[10];
    uint8_t uid_len;
    
    while (1) {
        if (mfrc522_picc_is_new_card_present(scanner, &uid_len)) {
            if (mfrc522_picc_read_card_serial(scanner, uid, &uid_len)) {
                ESP_LOGI(TAG, "RFID detected");
                
                // In a real system, you would verify the UID against known values
                if (user_callback) {
                    user_callback(true);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
