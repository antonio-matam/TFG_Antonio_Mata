// components/diagnostics/diagnostics.c

#include "diagnostics.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <inttypes.h>    // para PRIu32

static const char *TAG = "diagnostics";
// renombrado para no colisionar con el tipo `nvs_handle` de nvs.h
static nvs_handle_t diag_nvs_handle;

void diagnostics_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES
     || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_open("diag", NVS_READWRITE, &diag_nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open diag failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Diagnostics initialized");
    }
}

void diagnostics_log_error(const char *subsystem, esp_err_t err, const char *msg)
{
    ESP_LOGE(subsystem, "Error %s: %s", msg, esp_err_to_name(err));
    if (diag_nvs_handle) {
        uint32_t cnt = 0;
        nvs_get_u32(diag_nvs_handle, "err_cnt", &cnt);
        cnt++;
        nvs_set_u32(diag_nvs_handle, "err_cnt", cnt);
        nvs_set_str(diag_nvs_handle, "last_err", msg);
        nvs_commit(diag_nvs_handle);
    }
}

void diagnostics_record_event(const char *event_name, const char *details)
{
    ESP_LOGI(TAG, "Event %s: %s", event_name, details ? details : "");

    if (diag_nvs_handle) {
        uint32_t idx = 0;
        nvs_get_u32(diag_nvs_handle, "evt_cnt", &idx);

        char key[16];
        // usamos PRIu32 para formatear uint32_t
        snprintf(key, sizeof(key), "evt_%" PRIu32, idx);

        nvs_set_str(diag_nvs_handle, key, event_name);
        idx++;
        nvs_set_u32(diag_nvs_handle, "evt_cnt", idx);
        nvs_commit(diag_nvs_handle);
    }
}
