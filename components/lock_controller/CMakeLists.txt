idf_component_register(SRCS "lock_controller.c"
                    INCLUDE_DIRS "include"
                    REQUIRES lcd servo_lock nvs_flash)
