#include "inputs.h"
#include "lock_controller.h"

static bool door_last_level;
static bool register_last_level;

void inputs_task(void *params) {
    while (true) {
        bool current_level = gpio_get_level(CONFIG_CLOSE_DETECTOR_PIN);
        if (!door_last_level && current_level) {
            LkcMsg_t message = {
                .id = LKC_CLOSE
            };
            xQueueSend(lkc_input, &message, portMAX_DELAY);
        }

        door_last_level = current_level;

        current_level = gpio_get_level(CONFIG_REGISTER_BUTTON_PIN);
        if (!register_last_level && current_level) {
            LkcMsg_t message = {
                .id = LKC_READER_MODE,
                .data = LKC_READER_MODE_NEXT
            };
            xQueueSend(lkc_input, &message, portMAX_DELAY);
        }

        register_last_level = current_level;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void start_inputs_task() {
    gpio_set_direction(CONFIG_CLOSE_DETECTOR_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CONFIG_CLOSE_DETECTOR_PIN, GPIO_PULLUP_ONLY);
    door_last_level = gpio_get_level(CONFIG_CLOSE_DETECTOR_PIN);

    gpio_set_direction(CONFIG_REGISTER_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(CONFIG_REGISTER_BUTTON_PIN, GPIO_PULLUP_ONLY);
    register_last_level = gpio_get_level(CONFIG_REGISTER_BUTTON_PIN);

    xTaskCreate(inputs_task, IN_TAG, 8192, NULL, 1, NULL);
}