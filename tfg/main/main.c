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
    diagnostics_init();          // 1) Arranca el sistema de logs y diagnóstico
    config_init();               // 2) Carga o inicializa la configuración global (nvs, etc)
    sensors_init();              // 3) Pone a punto los sensores
    storage_init();              // 4) Inicializa el almacenamiento local (nvs)
    communication_init();        // 5) Arranca wifi y mqtt y sincroniza hora
    power_manager_init();        // 6) Comprueba y registra si venimos de deep sleep
    timer_manager_init();        // 7) Prepara el gestor de temporización
    tasks_start_all();           // 8) Crea y lanza las tareas de freertos (lectura, publicación y deep sleep)
}


