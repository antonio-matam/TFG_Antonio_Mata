#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "diagnostics.h"
#include "config.h"
#include "sensors.h"
#include "storage.h"
#include "communication.h"
#include "power_manager.h"
#include "utils.h"
#include "tasks.h"

void app_main(void) {
    diagnostics_init();
    config_init();
    sensors_init();
    storage_init();
    communication_init();  
    power_manager_init();
    timer_manager_init();
    tasks_start_all();
}


