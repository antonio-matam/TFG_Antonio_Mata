#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "sensors.h"   

void timer_manager_init(void);
/// Retarda el task ms milisegundos.
void timer_manager_delay_ms(uint32_t ms);

void change_detector_init(float threshold);

bool change_detector_has_significant_change(float prev, float current);

int data_formatter_format_json(const sensor_data_t *data, char *buf, size_t bufsize);

