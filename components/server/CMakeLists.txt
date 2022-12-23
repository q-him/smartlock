idf_component_register(SRCS "wifi_server.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES "index.html"
                    REQUIRES nvs_flash esp_http_server lock_controller)