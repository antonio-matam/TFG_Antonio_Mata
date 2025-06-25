idf_component_register(
    SRCS        "communication.c"
    INCLUDE_DIRS "include"
    REQUIRES    
    esp_netif esp_wifi 
    esp_event 
    mqtt 
    freertos 
    sensors
)

