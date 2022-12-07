idf_component_register(SRCS "rfid.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp-idf-rc522 lock_controller)
