#pragma once
#include <stdbool.h>

void power_manager_init(void);
bool power_manager_should_sleep(void);
void power_manager_enter_deep_sleep(void);
