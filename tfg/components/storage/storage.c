#include "storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "sensors.h"  // Dependemos de sensors, así que el REQUIRES en CMake refleja eso.

static const char* TAG = "storage";

// Renombramos la variable para no chocar con el typedef nvs_handle_t
static nvs_handle_t nvs_handle_local;
static uint32_t record_index = 0;

void storage_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    nvs_open("storage", NVS_READWRITE, &nvs_handle_local);
}

void storage_buffer_data(const sensor_data_t* d) {
    char key[16];
    // El especificador %lu ya funciona correctamente para un uint32_t
    snprintf(key, sizeof(key), "rec%lu", (unsigned long)record_index++);
    esp_err_t err = nvs_set_blob(nvs_handle_local, key, d, sizeof(sensor_data_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error writtng to NVS: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Register saved %s", key);
        nvs_commit(nvs_handle_local);
    }
}
