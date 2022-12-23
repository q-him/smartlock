#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define SL_TAG "servo_lock"

extern QueueHandle_t sl_queue;

void start_servo_lock_task();