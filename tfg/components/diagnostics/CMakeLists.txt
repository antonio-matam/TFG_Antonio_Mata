idf_component_register(
    SRCS        "diagnostics.c"
    INCLUDE_DIRS "include"
    REQUIRES    nvs_flash 
                log
)
