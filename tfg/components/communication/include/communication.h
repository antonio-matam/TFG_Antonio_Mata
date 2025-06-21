#pragma once

#include <esp_err.h>
#include "sensors.h"
//Inicializa wifi y mqtt y gestiona eventos para saber cuando está conectado
void communication_init(void);

//Esta función se encarga de bloquear hasta que wifi y mqtt estén conectados
void communication_wait_for_connection(void);

//Publica los datos de los sensores en 3 topics diferentes: sensors/ultrasonic, sensors/weight y sensors/laser
void communication_publish(const sensor_data_t* data);






