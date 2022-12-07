#include "lock_controller.h"

QueueHandle_t lkc_input = NULL;

static uint64_t keys[KEYS_SIZE] = {0};
static bool is_open = false;

void lock_controller_task(void *params);
void load_keys();
void open_lock(uint64_t key);
bool contains_key(uint64_t key);
void close_lock();
void register_key(uint64_t key);
void unregister_key(uint64_t key);

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
    
    LkcMsg_t message;

    while (true) {
        xQueueReceive(lkc_input, (void*)(&message), portMAX_DELAY);
        ESP_LOGI(LKC_TAG, "message received");

        switch (message.id) {
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
            default:
                ESP_LOGI(LKC_TAG, "UNKNOWN %llu", message.data);
                break;
        }
    }
}

void load_keys() {
    for (size_t i = 0; i < 6; i++) {
        keys[i] = i;
    }

    ESP_LOGI(LKC_TAG, "6 keys loaded");
}

void open_lock(uint64_t key) {
    if (!contains_key(key)) {
        ESP_LOGE(LKC_TAG, "cannot open lock: incorrect key %llu", key);
    }

    ESP_LOGI(LKC_TAG, "opened the lock with key %llu", key);
    is_open = true;
}

bool contains_key(uint64_t key) {
    for (size_t i = 0; i < KEYS_SIZE; i++) {
        if (key == keys[i]) {
            return true;
        }
    }

    return false;
}

void close_lock() {
    ESP_LOGI("LKC_TAG", "closed the lock");
    is_open = false;
}

void register_key(uint64_t key) {
    if (contains_key(key)) {
        ESP_LOGW(LKC_TAG, "key %llu is already registered", key);
        return;
    }

    for (size_t i = 0; i < KEYS_SIZE; i++) {
        if (keys[i] != 0) {
            continue;
        }

        keys[i] = key;
        ESP_LOGI(LKC_TAG, "key %llu registered", key);
        return;
    }

    ESP_LOGE(LKC_TAG, "cannot register key %llu: no space in key storage", key);
}

void unregister_key(uint64_t key) {
    for (size_t i = 0; i < KEYS_SIZE; i++) {
        if (keys[i] != key) {
            continue;
        }

        keys[i] = 0;
        ESP_LOGI(LKC_TAG, "key %llu unregistered", key);
        return;
    }

    ESP_LOGW(LKC_TAG, "cannot unregister key %llu: no such key in storage", key);
}