#pragma once

#include "esp_err.h"

void diagnostics_init(void);

void diagnostics_log_error(const char *tag, esp_err_t err, const char *msg);

void diagnostics_record_event(const char *event_name, const char *details);

