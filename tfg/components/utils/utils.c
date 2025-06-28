#include "utils.h"
#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG = "utils";                                   // Etiqueta de logs para utils
static float change_threshold = 0.0f;                               // Umbral para detectar cambios significativos

void timer_manager_init(void)
{
     // Inicializa hardware o recursos de temporización (vacío en esta versión)
}

void timer_manager_delay_ms(uint32_t ms)
{
    // Retrasa la tarea actual ms usando freertos
    vTaskDelay(pdMS_TO_TICKS(ms));
}

int data_formatter_format_json(const sensor_data_t *data, char *buf, size_t bufsize)
{
    // Serializa los valores de sensores en un json simple dentro de buf
    return snprintf(buf, bufsize,
        "{\"distance_cm\":%.2f,\"weight_kg\":%.2f,\"laser_mm\":%.2f}", data->distance_cm, data->weight_kg, data->laser_mm
    );
}
