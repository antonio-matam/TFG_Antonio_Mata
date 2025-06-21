#include "utils.h"
#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "utils";
static float change_threshold = 0.0f;

void timer_manager_init(void)
{
    // Si quieres arrancar algún hardware de temporizador, hazlo aquí.
    // Por ahora no hace nada.
}

void timer_manager_delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void change_detector_init(float threshold)
{
    change_threshold = threshold;
    ESP_LOGI(TAG, "Change detector threshold set to %.2f", change_threshold);
}

bool change_detector_has_significant_change(float prev, float current)
{
    return fabsf(current - prev) >= change_threshold;
}

int data_formatter_format_json(const sensor_data_t *data, char *buf, size_t bufsize)
{
    // Construye un JSON muy simple
    return snprintf(buf, bufsize,
        "{\"distance_cm\":%.2f,\"weight_kg\":%.2f,\"laser_mm\":%.2f}",
        data->distance_cm,
        data->weight_kg,
        data->laser_mm
    );
}
