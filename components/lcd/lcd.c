#include "./include/lcd.h"

void lcd_task(void* param);
void init_i2c_master();

void start_lcd_task()
{
    xTaskCreate(lcd_task, LCD_TAG, 16384, NULL, 1, NULL);
}

void lcd_task(void* param)
{
    // Set up I2C
    init_i2c_master();
    i2c_port_t i2c_num = I2C_MASTER_NUM;
    uint8_t address = CONFIG_LCD_I2C_ADDR;

    // Set up the SMBus
    smbus_info_t * smbus_info = smbus_malloc();
    ESP_ERROR_CHECK(smbus_init(smbus_info, i2c_num, address));
    ESP_ERROR_CHECK(smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS));

    // Set up the LCD1602 device with backlight off
    i2c_lcd1602_info_t * lcd_info = i2c_lcd1602_malloc();
    ESP_ERROR_CHECK(i2c_lcd1602_init(lcd_info, smbus_info, true,
                                     LCD_NUM_ROWS, LCD_NUM_COLUMNS, LCD_NUM_VISIBLE_COLUMNS));

    ESP_ERROR_CHECK(i2c_lcd1602_reset(lcd_info));

    ESP_LOGI(LCD_TAG, "backlight on");
    i2c_lcd1602_set_backlight(lcd_info, true);
    
    ESP_LOGI(LCD_TAG, "cursor on");
    i2c_lcd1602_set_cursor(lcd_info, true);

    ESP_LOGI(LCD_TAG, "display A at 0,0");
    i2c_lcd1602_move_cursor(lcd_info, 0, 0);
    i2c_lcd1602_write_char(lcd_info, 'A');

    ESP_LOGI(LCD_TAG, "display B at 8,0");
    i2c_lcd1602_move_cursor(lcd_info, 8, 0);
    i2c_lcd1602_write_char(lcd_info, 'B');

    ESP_LOGI(LCD_TAG, "display C at 15,1");
    i2c_lcd1602_move_cursor(lcd_info, 15, 1);
    i2c_lcd1602_write_char(lcd_info, 'C');

    ESP_LOGI(LCD_TAG, "move to 0,1 and blink");  // cursor should still be on
    i2c_lcd1602_move_cursor(lcd_info, 0, 1);
    i2c_lcd1602_set_blink(lcd_info, true);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void init_i2c_master(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;  // GY-2561 provides 10kΩ pullups
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;  // GY-2561 provides 10kΩ pullups
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_MASTER_RX_BUF_LEN,
                       I2C_MASTER_TX_BUF_LEN, 0);
}