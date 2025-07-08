<<<<<<< HEAD
idf_component_register(
    SRCS        "communication.c"              # Fichero fuente principal del módulo de comunicación
    INCLUDE_DIRS "include"                     # Carpeta con sus archivos .h
    REQUIRES                                   # Componentes externos necesarios para compilar y enlazar
    esp_netif esp_wifi 
    esp_event 
    mqtt 
    freertos 
    sensors
)

=======
<<<<<<< HEAD
idf_component_register(
    SRCS        "communication.c"              # Fichero fuente principal del módulo de comunicación
    INCLUDE_DIRS "include"                     # Carpeta con sus archivos .h
    REQUIRES    esp_netif esp_wifi esp_event mqtt freertos sensors      # Componentes externos necesarios para compilar y enlazar
)

=======
idf_component_register(
    SRCS        "communication.c"              # Fichero fuente principal del módulo de comunicación
    INCLUDE_DIRS "include"                     # Carpeta con sus archivos .h
    REQUIRES                                   # Componentes externos necesarios para compilar y enlazar
    esp_netif esp_wifi 
    esp_event 
    mqtt 
    freertos 
    sensors
)

>>>>>>> 574321047df6fc4b22468f4dd7fc3a49a772ef77
>>>>>>> c9e17441fbe05838da6a2e3ba14cdb532dadcb14
