idf_component_register(
    SRCS    "tasks.c"
    INCLUDE_DIRS "include"
    REQUIRES    freertos  
                utils      
                storage
                communication
                power_manager
                sensors
)