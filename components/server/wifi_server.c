#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_http_server.h"

#define ESP_WIFI_SSID      "Smartlock Access Point"
#define ESP_WIFI_PASS      "smartlock"
#define ESP_WIFI_CHANNEL   0
#define MAX_STA_CONN       10

static const char *TAG = "wifi softAP";
static const char *WIFITASKTAG = "wifi task";
const char master_password[100] = "1111";    // мастер-пароль (только цифры)
bool is_locker_open = false;                 // статус замка


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .ssid_len = strlen(ESP_WIFI_SSID),
            .channel = ESP_WIFI_CHANNEL,
            .password = ESP_WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             ESP_WIFI_SSID, ESP_WIFI_PASS, ESP_WIFI_CHANNEL);
}

// Отправка веб-страницы
esp_err_t get_handler(httpd_req_t *req)
{
    extern const char index_start[] asm("_binary_index_html_start");
    extern const char index_end[] asm("_binary_index_html_end");
    size_t index_len = index_end - index_start;

    httpd_resp_send(req, index_start, index_len);
    return ESP_OK;
}

// Возврат мастер-пароля 
esp_err_t post_check_password_handler(httpd_req_t *req)
{
    char content[100];
    httpd_req_recv(req, content, req->content_len);
    ESP_LOGI(TAG, "%s", content);
    for (int i=0; i<100; i++) {
      if (master_password[i] == 0) {
        httpd_resp_send(req, "true", HTTPD_RESP_USE_STRLEN);
        break;
      }
      if (master_password[i] != content[i]) {
        httpd_resp_send(req, "false", HTTPD_RESP_USE_STRLEN);
        break;
      }
    }
    return ESP_OK;
}

// Возврат текущего статуса замка (закрыт или открыт) для корректного отображения хедера
// и скрытия панели ввода пароля
esp_err_t post_get_status(httpd_req_t *req)
{
    if (is_locker_open)
      httpd_resp_send(req, "true", HTTPD_RESP_USE_STRLEN);
    else
      httpd_resp_send(req, "false", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Обработка события при нажатии на кнопку "Зарегистрировать"
esp_err_t post_start_register(httpd_req_t *req)
{




    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Started register");
    return ESP_OK;
}

// Обработка события при нажатии на кнопку возврата из режима регистрации
esp_err_t post_stop_register(httpd_req_t *req)
{
    



    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Stopped register");
    return ESP_OK;
}

// Обработка события при нажатии на кнопку "Удалить"
esp_err_t post_start_delete(httpd_req_t *req)
{
    



    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Started delete");
    return ESP_OK;
}

// Обработка события при нажатии на кнопку возврата из режима удаления
esp_err_t post_stop_delete(httpd_req_t *req)
{
    



    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Stopped delete");
    return ESP_OK;
}

// Обработка открытия замка
esp_err_t post_open_locker(httpd_req_t *req)
{
    is_locker_open = true;
    



    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Locker is open now");
    return ESP_OK;
}

// Обработка открытия замка
esp_err_t post_close_locker(httpd_req_t *req)
{
    is_locker_open = false;




    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    ESP_LOGI(TAG, "Locker is close now");
    return ESP_OK;
}

httpd_uri_t uri_get_lockpanel = {
    .uri      = "/lockpanel",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_post_check_password = {
    .uri      = "/check_password",
    .method   = HTTP_POST,
    .handler  = post_check_password_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_post_get_status = {
    .uri      = "/get_status",
    .method   = HTTP_POST,
    .handler  = post_get_status,
    .user_ctx = NULL
};

httpd_uri_t uri_post_start_register = {
    .uri      = "/start_register",
    .method   = HTTP_POST,
    .handler  = post_start_register,
    .user_ctx = NULL
};

httpd_uri_t uri_post_stop_register = {
    .uri      = "/stop_register",
    .method   = HTTP_POST,
    .handler  = post_stop_register,
    .user_ctx = NULL
};

httpd_uri_t uri_post_start_delete = {
    .uri      = "/start_delete",
    .method   = HTTP_POST,
    .handler  = post_start_delete,
    .user_ctx = NULL
};

httpd_uri_t uri_post_stop_delete = {
    .uri      = "/stop_delete",
    .method   = HTTP_POST,
    .handler  = post_stop_delete,
    .user_ctx = NULL
};

httpd_uri_t uri_post_open_locker = {
    .uri      = "/open_locker",
    .method   = HTTP_POST,
    .handler  = post_open_locker,
    .user_ctx = NULL
};

httpd_uri_t uri_post_close_locker = {
    .uri      = "/close_locker",
    .method   = HTTP_POST,
    .handler  = post_close_locker,
    .user_ctx = NULL
};

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get_lockpanel);
        httpd_register_uri_handler(server, &uri_post_check_password);
        httpd_register_uri_handler(server, &uri_post_get_status);
        httpd_register_uri_handler(server, &uri_post_start_delete);
        httpd_register_uri_handler(server, &uri_post_stop_delete);
        httpd_register_uri_handler(server, &uri_post_start_register);
        httpd_register_uri_handler(server, &uri_post_stop_register);
        httpd_register_uri_handler(server, &uri_post_open_locker);
        httpd_register_uri_handler(server, &uri_post_close_locker);
    }
    return server;
}

void wifi_server_task(void *params) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
    start_webserver();
    
    while(1) {

    }
}

void start_server_task(void)
{
    xTaskCreate(wifi_server_task, WIFITASKTAG, 32768, NULL, 1, NULL);
}