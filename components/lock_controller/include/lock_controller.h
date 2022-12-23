#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "servo_lock.h"
#include "lcd.h"
#include "nvs.h"
#include "nvs_flash.h"

#define KEYS_SIZE CONFIG_KEY_STORAGE_SIZE

typedef enum
{
    LKC_KEY,
    LKC_OPEN,
    LKC_CLOSE,
    LKC_REGISTER,
    LKC_UNREGISTER,
    LKC_READER_MODE
} LkcDataId_t;

typedef enum {
    LKC_READER_MODE_NEXT,
    LKC_READER_OPEN,
    LKC_READER_REGISTER,
    LKC_READER_UNREGISTER
} LkcReaderMode_t;

typedef struct
{
    LkcDataId_t id;
    uint64_t data; 
} LkcMsg_t;

typedef struct
{
    QueueHandle_t input_queue;
} LkcParams_t;

extern QueueHandle_t lkc_input;
extern bool is_lock_open;
static const char* LKC_TAG = "lock_controller";

void start_lock_controller();