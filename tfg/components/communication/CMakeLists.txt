idf_component_register(
    SRCS        "communication.c"              # Fichero fuente principal del módulo de comunicación
    INCLUDE_DIRS "include"                     # Carpeta con sus archivos .h
    REQUIRES    esp_netif esp_wifi esp_event mqtt freertos sensors      # Componentes externos necesarios para compilar y enlazar
)

