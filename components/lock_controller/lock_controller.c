#include "lock_controller.h"

QueueHandle_t lkc_input = NULL;

static uint64_t keys[KEYS_SIZE] = {0};
bool is_lock_open = false;
static LkcReaderMode_t reader_mode = LKC_READER_OPEN;

void lock_controller_task(void *params);
void load_keys();
void handle_key(uint64_t key);
void open_lock(uint64_t key);
bool contains_key(uint64_t key);
void close_lock();
void register_key(uint64_t key);
void unregister_key(uint64_t key);
void change_reader_mode(LkcReaderMode_t new_mode);
void show_lcd_message(LcdMessage_t msg, TickType_t delay);
void show_appropriate_message();

void start_lock_controller() {
    lkc_input = xQueueCreate(16, sizeof(LkcMsg_t));
    xTaskCreate(
        lock_controller_task,
        LKC_TAG,
        32768,
        NULL,
        1,
        NULL
    );
}

void lock_controller_task(void *params)
{
    ESP_LOGI(LKC_TAG, "started");

    load_keys();
    for (int i = 0; i < KEYS_SIZE; i++) {
        uint64_t key = keys[i];
        ESP_LOGI(LKC_TAG, "key #%i: %llu", i, key);
    }
    
    show_appropriate_message();

    LkcMsg_t message;

    while (true) {
        xQueueReceive(lkc_input, (void*)(&message), portMAX_DELAY);

        switch (message.id) {
            case LKC_KEY:
                handle_key(message.data);
                break;
            case LKC_OPEN:
                open_lock(message.data);
                break;
            case LKC_CLOSE:
                close_lock();
                break;
            case LKC_REGISTER:
                register_key(message.data);
                break;
            case LKC_UNREGISTER:
                unregister_key(message.data);
                break;
            case LKC_READER_MODE:
                change_reader_mode((LkcReaderMode_t)message.data);
                break;
            default:
                ESP_LOGI(LKC_TAG, "UNKNOWN %llu", message.data);
                break;
        }

        show_appropriate_message();
    }
}

void load_keys() {
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(LKC_TAG, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return;

    // Read run time blob
    size_t required_size = KEYS_SIZE * sizeof(uint64_t);  // value will default to 0, if not set yet in NVS
    err = nvs_get_blob(my_handle, "keys", (void*)keys, &required_size);
    if (err != ESP_OK) {
        ESP_LOGE(LKC_TAG, "cannot read keys!");
        return;
    }

    // Close
    nvs_close(my_handle);
}

void handle_key(uint64_t key) {
    if (reader_mode == LKC_READER_REGISTER) {
        register_key(key);
        return;
    } else if (reader_mode == LKC_READER_UNREGISTER) {
        unregister_key(key);
        return;
    } else {
        open_lock(key);
    }
}

void open_lock(uint64_t key) {
    if (key != 0 && !contains_key(key)) {
        ESP_LOGE(LKC_TAG, "cannot open lock: incorrect key %llu", key);
        show_lcd_message(LCD_SHOW_ERROR, pdMS_TO_TICKS(CONFIG_MESSAGE_DELAY_MS));
        return;
    }

    ESP_LOGI(LKC_TAG, "opened the lock with key %llu", key);
    is_lock_open = true;
    xQueueSend(sl_queue, (void*)&is_lock_open, portMAX_DELAY);
}

bool contains_key(uint64_t key) {
    if (key == 0) {
        return false;
    }

    for (size_t i = 0; i < KEYS_SIZE; i++) {
        if (key == keys[i]) {
            return true;
        }
    }

    return false;
}

void close_lock() {
    ESP_LOGI("LKC_TAG", "closed the lock");
    is_lock_open = false;
    xQueueSend(sl_queue, (void*)&is_lock_open, portMAX_DELAY);
}

void save_keys() {
    nvs_handle_t my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(LKC_TAG, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LKC_TAG, "cannot open nvs namespace");
        return;
    };

    size_t required_size = KEYS_SIZE * sizeof(uint64_t);
    err = nvs_set_blob(my_handle, "keys", (void*)keys, required_size);
    if (err != ESP_OK) {
        ESP_LOGE(LKC_TAG, "cannot set blob");
        return;
    };

    // Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LKC_TAG, "cannot commit nvs changes");
    };

    // Close
    nvs_close(my_handle);
}

void register_key(uint64_t key) {
    if (contains_key(key)) {
        ESP_LOGW(LKC_TAG, "key %llu is already registered", key);
        show_lcd_message(LCD_SHOW_OK, pdMS_TO_TICKS(CONFIG_MESSAGE_DELAY_MS));
        return;
    }

    for (size_t i = 0; i < KEYS_SIZE; i++) {
        if (keys[i] != 0) {
            continue;
        }

        keys[i] = key;
        ESP_LOGI(LKC_TAG, "key %llu registered", key);
        save_keys();
        show_lcd_message(LCD_SHOW_OK, pdMS_TO_TICKS(CONFIG_MESSAGE_DELAY_MS));
        return;
    }

    ESP_LOGE(LKC_TAG, "cannot register key %llu: no space in key storage", key);
    reader_mode = LKC_READER_OPEN;
    show_lcd_message(LCD_SHOW_ERROR, pdMS_TO_TICKS(CONFIG_MESSAGE_DELAY_MS));
}

void unregister_key(uint64_t key) {
    for (size_t i = 0; i < KEYS_SIZE; i++) {
        if (keys[i] != key) {
            continue;
        }

        keys[i] = 0;
        ESP_LOGI(LKC_TAG, "key %llu unregistered", key);
        save_keys();
        show_lcd_message(LCD_SHOW_OK, pdMS_TO_TICKS(CONFIG_MESSAGE_DELAY_MS));
        return;
    }

    ESP_LOGW(LKC_TAG, "cannot unregister key %llu: no such key in storage", key);
    reader_mode = LKC_READER_OPEN;
    show_lcd_message(LCD_SHOW_ERROR, pdMS_TO_TICKS(CONFIG_MESSAGE_DELAY_MS));
}

void change_reader_mode(LkcReaderMode_t new_mode) {
    if (new_mode == LKC_READER_MODE_NEXT) {
        if (reader_mode == LKC_READER_OPEN) {
            reader_mode = LKC_READER_REGISTER;
        } else if (reader_mode == LKC_READER_REGISTER) {
            reader_mode = LKC_READER_UNREGISTER;
        } else {
            reader_mode = LKC_READER_OPEN;
        }
    } else {
        reader_mode = new_mode;
    }

    if (reader_mode == LKC_READER_OPEN) {
        ESP_LOGI(LKC_TAG, "open mode");
    } else if (reader_mode == LKC_READER_REGISTER) {
        ESP_LOGI(LKC_TAG, "register mode");
    } else if (reader_mode == LKC_READER_UNREGISTER){
        ESP_LOGI(LKC_TAG, "unregister mode");
    } else {
        ESP_LOGE(LKC_TAG, "reader_mode == LKC_READER_NEXT");
    }
}

void show_lcd_message(LcdMessage_t msg, TickType_t delay) {
        xQueueSend(lcd_queue, &msg, pdMS_TO_TICKS(250));
        vTaskDelay(delay);
}

void show_appropriate_message() {
    LcdMessage_t msg;
    if (reader_mode == LKC_READER_OPEN && is_lock_open) {
        msg = LCD_SHOW_OPEN;
    } else if (reader_mode == LKC_READER_OPEN && !is_lock_open) {
        msg = LCD_SHOW_CLOSED;
    } else if (reader_mode == LKC_READER_REGISTER) {
        msg = LCD_SHOW_REGISTERING;
    } else if (reader_mode == LKC_READER_UNREGISTER) {
        msg = LCD_SHOW_UNREGISTERING;
    } else {
        msg = LCD_SHOW_UNKNOWN;
    }

    show_lcd_message(msg, 0);
}