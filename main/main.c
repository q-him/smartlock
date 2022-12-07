#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "lock_controller.h"
#include "lcd.h"

static const char* TAG = "main";

void app_main(void)
{
    int i = 0;

    start_lock_controller();
    start_lcd_task();

    while (true) {
        LkcDataId_t id = i % 4;

        LkcMsg_t message = {
            .id = id,
            .data = i
        };

        LcdMessage_t lcd_msg = i % 4;

        xQueueSend(lkc_input, &message, 0);
        xQueueSend(lcd_queue, &lcd_msg, 0);

        vTaskDelay(pdMS_TO_TICKS(5000));
        i++;
    }
}
