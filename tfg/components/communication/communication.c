#include "communication.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "sensors.h"
#include <string.h>
#include "freertos/event_groups.h"

// Para SNTP y timestamp
#include "esp_sntp.h"
#include <sys/time.h>
#include <time.h>

static const char* TAG = "communication";
static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_started = false;

// EventGroup para wifi y mqtt
static EventGroupHandle_t comm_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static const int MQTT_CONNECTED_BIT = BIT1;

// Id, password y uri para conectar a wifi y mqtt
static const char* WIFI_SSID     = "S22 Ultra de Antonio";
static const char* WIFI_PASSWORD = "dlff9165";
static const char* MQTT_URI      = "mqtt://mqtt.eclipseprojects.io:1883";

// 3 topics donde se enviarán los datos
static const char* MQTT_TOPIC_ULTRASONIC = "sensors/ultrasonic";
static const char* MQTT_TOPIC_WEIGHT     = "sensors/weight";
static const char* MQTT_TOPIC_LASER      = "sensors/laser";

// Prototipos internos
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data);
static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
static void init_sntp_and_wait(void);
static void get_iso8601_utc(char *out, size_t out_size);

void communication_init(void) {
    comm_event_group = xEventGroupCreate();
    if (comm_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create EventGroup");
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

    esp_event_handler_instance_t wifi_any_id_handle;
    esp_event_handler_instance_t ip_got_ip_handle;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &wifi_any_id_handle));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &ip_got_ip_handle));

    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = "",
            .password = "",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char*)wifi_cfg.sta.ssid, WIFI_SSID, sizeof(wifi_cfg.sta.ssid));
    strncpy((char*)wifi_cfg.sta.password, WIFI_PASSWORD, sizeof(wifi_cfg.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Wi-Fi init done, waiting for IP...");

    if (comm_event_group) {
        xEventGroupWaitBits(
            comm_event_group,
            WIFI_CONNECTED_BIT,
            pdFALSE,
            pdTRUE,
            portMAX_DELAY
        );
        ESP_LOGI(TAG, "Wi-Fi connected, IP obtained");
    }

    init_sntp_and_wait();

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = MQTT_URI
            }
        }
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to init MQTT client");
        return;
    }
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));

    esp_mqtt_client_start(mqtt_client);
    mqtt_started = true;
    ESP_LOGI(TAG, "MQTT client started");
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi disconnected, retrying...");
        if (comm_event_group) {
            xEventGroupClearBits(comm_event_group, WIFI_CONNECTED_BIT | MQTT_CONNECTED_BIT);
        }
        mqtt_started = false;
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Wi-Fi EVENT: GOT IP");
        if (comm_event_group) {
            xEventGroupSetBits(comm_event_group, WIFI_CONNECTED_BIT);
        }
    }
}

static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        if (comm_event_group) {
            xEventGroupSetBits(comm_event_group, MQTT_CONNECTED_BIT);
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        if (comm_event_group) {
            xEventGroupClearBits(comm_event_group, MQTT_CONNECTED_BIT);
        }
        mqtt_started = false;
        break;
    default:
        break;
    }
}

void communication_wait_for_connection(void) {
    if (comm_event_group == NULL) {
        ESP_LOGW(TAG, "communication_wait_for_connection: EventGroup NULL, skipping wait");
        return;
    }
    xEventGroupWaitBits(
        comm_event_group,
        WIFI_CONNECTED_BIT | MQTT_CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY
    );
    ESP_LOGI(TAG, "communication: Wi-Fi + MQTT connected, proceeding to publish");
}

static void init_sntp_and_wait(void) {
    ESP_LOGI(TAG, "SNTP: Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    const char *ntp_server = "pool.ntp.org";
    esp_sntp_setservername(0, ntp_server);
    ESP_LOGI(TAG, "SNTP: server set to %s", ntp_server);
    esp_sntp_init();
    ESP_LOGI(TAG, "SNTP: init done, waiting for sync...");
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
        ESP_LOGI(TAG, "SNTP: not yet synced, waiting 1s...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    time_t now = tv_now.tv_sec;
    struct tm tm_utc;
    gmtime_r(&now, &tm_utc);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm_utc);
    ESP_LOGI(TAG, "SNTP: time synchronized: %s", buf);
}

static void get_iso8601_utc(char *out, size_t out_size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t now = tv.tv_sec;
    struct tm tm_utc;
    gmtime_r(&now, &tm_utc);
    strftime(out, out_size, "%Y-%m-%dT%H:%M:%SZ", &tm_utc);
}

void communication_publish(const sensor_data_t* d_unused) {
    if (mqtt_client == NULL) {
        ESP_LOGW(TAG, "communication_publish: mqtt_client NULL, skipping");
        return;
    }

    char tsbuf[32];
    get_iso8601_utc(tsbuf, sizeof(tsbuf));

    char buf[128];
    int len;

    float distance_cm = (float)(esp_random() % 100);
    float weight_kg   = (float)(esp_random() % 20);
    float laser_mm    = (float)(esp_random() % 500);

    len = snprintf(buf, sizeof(buf), "{\"value\": %.2f, \"timestamp\": \"%s\"}", distance_cm, tsbuf);
    esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_ULTRASONIC, buf, len, 1, 0);
    ESP_LOGI(TAG, "Published to %s: %s", MQTT_TOPIC_ULTRASONIC, buf);

    len = snprintf(buf, sizeof(buf), "{\"value\": %.2f, \"timestamp\": \"%s\"}", weight_kg, tsbuf);
    esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_WEIGHT, buf, len, 1, 0);
    ESP_LOGI(TAG, "Published to %s: %s", MQTT_TOPIC_WEIGHT, buf);

    len = snprintf(buf, sizeof(buf), "{\"value\": %.2f, \"timestamp\": \"%s\"}", laser_mm, tsbuf);
    esp_mqtt_client_publish(mqtt_client, MQTT_TOPIC_LASER, buf, len, 1, 0);
    ESP_LOGI(TAG, "Published to %s: %s", MQTT_TOPIC_LASER, buf);
}
