#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "smbus.h"
#include "i2c-lcd1602.h"

#define LCD_TAG "lcd"

#define LCD_NUM_ROWS               2
#define LCD_NUM_COLUMNS            32
#define LCD_NUM_VISIBLE_COLUMNS    16

#define I2C_MASTER_NUM           I2C_NUM_0
#define I2C_MASTER_TX_BUF_LEN    0                     // disabled
#define I2C_MASTER_RX_BUF_LEN    0                     // disabled
#define I2C_MASTER_FREQ_HZ       100000
#define I2C_MASTER_SDA_IO        CONFIG_I2C_SDA_PIN
#define I2C_MASTER_SCL_IO        CONFIG_I2C_SCL_PIN

typedef enum {
    LCD_SHOW_UNKNOWN,
    LCD_SHOW_OPEN,
    LCD_SHOW_CLOSED,
    LCD_SHOW_ERROR,
    LCD_SHOW_REGISTERING,
    LCD_SHOW_OK,
    LCD_SHOW_UNREGISTERING
} LcdMessage_t;

extern QueueHandle_t lcd_queue;

void start_lcd_task();