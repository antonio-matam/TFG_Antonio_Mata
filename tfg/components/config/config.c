#include "config.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char* TAG = "config";

void config_init(void) {
    // Inicializa NVS para que otros módulos no fallen si lo necesitan
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Ya no imprimimos CONFIG_WIFI_SSID ni nada más,
    // porque esas macros no existen aquí.
    ESP_LOGI(TAG, "config_init() called, no CONFIG_* values available");
}


