#pragma once                                // Le indica al compilador que procese este fichero solo una vez por compilacion

typedef struct {
    float distance_cm;                      // Distancia medida por ultrasonidos en centímetros
    float weight_kg;                        // Peso medido en kilogramos
    float laser_mm;                         // Distancia medida por sensor láser en milímetros
} sensor_data_t;

void sensors_init(void);                    // Inicializa los drivers y periféricos de los sensores
sensor_data_t sensors_read_all(void);       // Lee todos los sensores y devuelve sus valores en un struct
