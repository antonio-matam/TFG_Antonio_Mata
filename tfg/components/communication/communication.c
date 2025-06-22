#include "communication.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "sensors.h"
#include "esp_sntp.h"
#include <sys/time.h>
#include <time.h>
#include "freertos/event_groups.h"

static const char* TAG = "communication";
static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_started = false;

static EventGroupHandle_t comm_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static const int MQTT_CONNECTED_BIT = BIT1;

static const char* WIFI_SSID     = "S22 Ultra de Antonio";
static const char* WIFI_PASSWORD = "dlff9165";
static const char* MQTT_URI      = "mqtt://mqtt.eclipseprojects.io:1883";

static const char* MQTT_TOPIC_ULTRASONIC = "sensors/ultrasonic";
static const char* MQTT_TOPIC_WEIGHT     = "sensors/weight";
static const char* MQTT_TOPIC_LASER      = "sensors/laser";

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
static void init_sntp_and_wait(void);
static void get_iso8601_utc(char *out, size_t out_size);

void communication_init(void) {
    comm_event_group = xEventGroupCreate();

    ESP_LOGI(TAG, "Initializing communication module..."); 

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

    esp_event_handler_instance_t wifi_any_id_handle;
    esp_event_handler_instance_t ip_got_ip_handle;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &wifi_any_id_handle));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &ip_got_ip_handle));

    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = "",
            .password = "",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        }
    };
    strncpy((char*)wifi_cfg.sta.ssid, WIFI_SSID, sizeof(wifi_cfg.sta.ssid));
    strncpy((char*)wifi_cfg.sta.password, WIFI_PASSWORD, sizeof(wifi_cfg.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    xEventGroupWaitBits(comm_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    init_sntp_and_wait();

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_URI
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    esp_mqtt_client_start(mqtt_client);
    mqtt_started = true;
}


void communication_wait_for_connection(void) {
    xEventGroupWaitBits(comm_event_group, WIFI_CONNECTED_BIT | MQTT_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(comm_event_group, WIFI_CONNECTED_BIT | MQTT_CONNECTED_BIT);
        mqtt_started = false;
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(comm_event_group, WIFI_CONNECTED_BIT);
    }
}

static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    if (event_id == MQTT_EVENT_CONNECTED) {
        xEventGroupSetBits(comm_event_group, MQTT_CONNECTED_BIT);
    } else if (event_id == MQTT_EVENT_DISCONNECTED) {
        xEventGroupClearBits(comm_event_group, MQTT_CONNECTED_BIT);
        mqtt_started = false;
    }
}

static void init_sntp_and_wait(void) {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void get_iso8601_utc(char *out, size_t out_size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm_utc;
    gmtime_r(&tv.tv_sec, &tm_utc);
    strftime(out, out_size, "%Y-%m-%dT%H:%M:%SZ", &tm_utc);
}

void communication_publish(const sensor_data_t* data) {
    if (!mqtt_client || !data) return;

    char ts[32], msg[128];
    get_iso8601_utc(ts, sizeof(ts));

    snprintf(msg, sizeof(msg), "{\"value\": %.2f, \"timestamp\": \"%s\"}", data->distance_cm, ts);
    esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_ULTRASONIC, msg, 0, 1, 0);
    ESP_LOGI(TAG, "Publicado en %s: %s", MQTT_TOPIC_ULTRASONIC, msg);

    snprintf(msg, sizeof(msg), "{\"value\": %.2f, \"timestamp\": \"%s\"}", data->weight_kg, ts);
    esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_WEIGHT, msg, 0, 1, 0);
    ESP_LOGI(TAG, "Publicado en %s: %s", MQTT_TOPIC_WEIGHT, msg);

    snprintf(msg, sizeof(msg), "{\"value\": %.2f, \"timestamp\": \"%s\"}", data->laser_mm, ts);
    esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_LASER, msg, 0, 1, 0);
    ESP_LOGI(TAG, "Publicado en %s: %s", MQTT_TOPIC_LASER, msg);
}
