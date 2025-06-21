#pragma once

#include "sensors.h"
#include "esp_err.h"

void storage_init(void);
void storage_buffer_data(const sensor_data_t* data);



