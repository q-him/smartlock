#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "rc522.h"
#include "lock_controller.h"

#define RFID_TAG "rfid"

void start_rfid_task();