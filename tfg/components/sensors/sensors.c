#include "sensors.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_random.h"


static const char* TAG = "sensors";

void sensors_init(void) {
    ESP_LOGI(TAG, "Inicializando sensores simulados...");

    // Simulación de calibración (espera artificial)
    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_LOGI(TAG, "Sensores simulados calibrados");
}

sensor_data_t sensors_read_all(void) {
    sensor_data_t d = { 0 };

    d.distance_cm = (float)(esp_random() % 100);                    // Ultrasonido
    d.weight_kg   = ((float)(esp_random() % 200)) * 0.01f;          // Peso: 0.00 - 2.00 kg
    d.laser_mm    = (float)(esp_random() % 500);                    // Láser

    ESP_LOGI(TAG, "Lecturas: %.2f cm, %.2f kg, %.2f mm", d.distance_cm, d.weight_kg, d.laser_mm);

    return d;
}
