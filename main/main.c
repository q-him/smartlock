#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "lock_controller.h"
#include "lcd.h"
#include "rfid.h"
#include "inputs.h"
#include "wifi_server.h"
#include "servo_lock.h"
#include "nvs_flash.h"

static const char* TAG = "main";

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    start_servo_lock_task();
    start_lock_controller();
    start_lcd_task();
    start_rfid_task();
    start_inputs_task();
    start_server_task();

    // while (true) {
    //     LkcDataId_t id = i % 4;

    //     LkcMsg_t message = {
    //         .id = id,
    //         .data = i
    //     };

    //     LcdMessage_t lcd_msg = i % 4;

    //     // xQueueSend(lkc_input, &message, 0);
    //     // xQueueSend(lcd_queue, &lcd_msg, 0);

    //     vTaskDelay(pdMS_TO_TICKS(5000));
    //     i++;
    // }
}
