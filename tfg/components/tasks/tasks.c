#include "tasks.h"
#include "sensors.h"
#include "storage.h"
#include "communication.h"
#include "power_manager.h"
#include "utils.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <stdbool.h>

static const char* TAG = "tasks";
static QueueHandle_t data_queue;

#define SENSOR_TASK_STACK    2048
#define SENSOR_TASK_PRI      (tskIDLE_PRIORITY + 2)
#define SENSOR_PERIOD_MS     60000

#define PUBLISH_TASK_STACK   4096
#define PUBLISH_TASK_PRI     (tskIDLE_PRIORITY + 1)

// --------------------------------------------------
//  Sensor task: lee cada SENSOR_PERIOD_MS y mete los datos en la cola
// --------------------------------------------------
static void sensor_task(void* _) {
    sensor_data_t d;
    for (;;) {
        d = sensors_read_all();
        if (xQueueSend(data_queue, &d, 0) != pdTRUE) {
            ESP_LOGW(TAG, "sensor_task: queue full, dropping sample");
        }
        ESP_LOGI(TAG, "Read: %.2f cm, %.2f kg, %.2f mm",
                 d.distance_cm, d.weight_kg, d.laser_mm);
        vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
    }
}

// --------------------------------------------------
//  Publish task: espera datos y publica cada sensor en su topic
// --------------------------------------------------
static void publish_task(void* _) {
    sensor_data_t d;

    for (;;) {
        if (xQueueReceive(data_queue, &d, portMAX_DELAY) == pdTRUE) {
            storage_buffer_data(&d);

            // Esperar conexión antes de publicar
            communication_wait_for_connection();

            communication_publish(&d);

            vTaskDelay(pdMS_TO_TICKS(200));
            ESP_LOGI(TAG, "Entering deep sleep for 30 seconds");
            power_manager_enter_deep_sleep();
        }
    }
}


void tasks_start_all(void) {
    // Asegúrate de que en app_main llamas a communication_init() antes de tasks_start_all().
    data_queue = xQueueCreate(10, sizeof(sensor_data_t));
    if (!data_queue) {
        ESP_LOGE(TAG, "tasks_start_all: failed to create queue");
        return;
    }

    xTaskCreate(sensor_task, "sensor_task", SENSOR_TASK_STACK, NULL, SENSOR_TASK_PRI, NULL);
    xTaskCreate(publish_task, "publish_task", PUBLISH_TASK_STACK, NULL, PUBLISH_TASK_PRI, NULL);

    ESP_LOGI(TAG, "tasks_start_all: all tasks started");
}
