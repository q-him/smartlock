#include "./include/rfid.h"

static rc522_handle_t scanner;

static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data)
{
    rc522_event_data_t* data = (rc522_event_data_t*) event_data;

    switch(event_id) {
        case RC522_EVENT_TAG_SCANNED: {
                rc522_tag_t* tag = (rc522_tag_t*) data->ptr;
                ESP_LOGI(RFID_TAG, "Tag scanned (sn: %" PRIu64 ")", tag->serial_number);
                LkcMsg_t message = {
                    .id = LKC_KEY,
                    .data = tag->serial_number
                };
                xQueueSend(lkc_input, &message, portMAX_DELAY);
            }
            break;
    }
}

void start_rfid_task() {
    rc522_config_t config = {
        .spi.host = VSPI_HOST,
        .spi.miso_gpio = CONFIG_SPI_MISO_PIN,
        .spi.mosi_gpio = CONFIG_SPI_MOSI_PIN,
        .spi.sck_gpio = CONFIG_SPI_SCK_PIN,
        .spi.sda_gpio = CONFIG_SPI_SDA_PIN,
    };

    rc522_create(&config, &scanner);
    rc522_register_events(scanner, RC522_EVENT_ANY, rc522_handler, NULL);
    rc522_start(scanner);
}