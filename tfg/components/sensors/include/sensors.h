#pragma once

typedef struct {
    float distance_cm;
    float weight_kg;
    float laser_mm;
} sensor_data_t;

void sensors_init(void);
sensor_data_t sensors_read_all(void);
