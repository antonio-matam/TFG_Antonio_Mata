#include "sensors.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"

static const char* TAG = "sensors";

// Pines y canales
#define ULTRASONIC_TRIG   GPIO_NUM_5
#define ULTRASONIC_ECHO   GPIO_NUM_18
#define ADC_UNIT          ADC_UNIT_1
#define ADC_CHANNEL       ADC_CHANNEL_0
#define LASER_I2C_ADDR    0x62

// Número de muestras para calcular la tara (offset)
#define TARE_SAMPLES      10

static int    raw_tare_offset = 0;
static float  kg_per_count     = 0.001f;

static adc_oneshot_unit_handle_t adc_handle;

static float measure_ultrasonic_cm(void) {
    // TODO: trigger + captura de echo
    return 0.0f;
}

static float read_laser_mm(void) {
    // TODO: lectura I2C del láser
    return 0.0f;
}

void sensors_init(void) {
    // Ultrasonido
    gpio_reset_pin(ULTRASONIC_TRIG);
    gpio_set_direction(ULTRASONIC_TRIG, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ULTRASONIC_ECHO);
    gpio_set_direction(ULTRASONIC_ECHO, GPIO_MODE_INPUT);

    // ADC nuevo (Oneshot)
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT,
    };
    adc_oneshot_new_unit(&init_cfg, &adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc_handle, ADC_CHANNEL, &chan_cfg);

    // Calcular la tara
    long sum = 0;
    int val = 0;
    for (int i = 0; i < TARE_SAMPLES; i++) {
        adc_oneshot_read(adc_handle, ADC_CHANNEL, &val);
        sum += val;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    raw_tare_offset = sum / TARE_SAMPLES;
    ESP_LOGI(TAG, "Tare raw offset: %d", raw_tare_offset);
}

sensor_data_t sensors_read_all(void) {
    sensor_data_t d = { 0 };

    // 1) Ultrasonido
    d.distance_cm = measure_ultrasonic_cm();

    // 2) Celda de carga: restamos offset y convertimos
    int raw = 0;
    adc_oneshot_read(adc_handle, ADC_CHANNEL, &raw);
    raw -= raw_tare_offset;
    if (raw < 0) raw = 0;
    d.weight_kg = raw * kg_per_count;

    // 3) Láser
    d.laser_mm = read_laser_mm();

    ESP_LOGI(TAG,
             "Distance: %.2f cm, Weight: %.2f kg, Laser: %.2f mm",
             d.distance_cm,
             d.weight_kg,
             d.laser_mm);

    return d;
}
