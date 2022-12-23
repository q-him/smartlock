idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES lock_controller lcd rfid server inputs servo_lock nvs_flash)
