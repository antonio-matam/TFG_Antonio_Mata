#include "sensors.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_random.h"


static const char* TAG = "sensors";                              // Etiqueta de logs para este modulo

void sensors_init(void) {
    ESP_LOGI(TAG, "Inicializando sensores simulados...");

    // Inicializa y calibra (simulado) los sensores con un delay
    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_LOGI(TAG, "Sensores simulados calibrados");
}

sensor_data_t sensors_read_all(void) {
    // Lee y genera valores aleatorios para cada sensor, los registra y devuelve
    sensor_data_t d = { 0 };

    d.distance_cm = (float)(esp_random() % 100);                    
    d.weight_kg   = ((float)(esp_random() % 200)) * 0.01f;          
    d.laser_mm    = (float)(esp_random() % 500);                    

    ESP_LOGI(TAG, "Lecturas: %.2f cm, %.2f kg, %.2f mm", d.distance_cm, d.weight_kg, d.laser_mm);

    return d;
}
