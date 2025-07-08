# Sistema de Adquisición de Datos con ESP32
# Descripción
Este proyecto implementa un sistema completo de adquisición de datos utilizando ESP32, con una infraestructura de monitoreo basada en Telegraf, InfluxDB y Grafana. El sistema está diseñado para recopilar datos de sensores, procesarlos y visualizarlos en tiempo real a través de dashboards en Grafana.
# Arquitectura del Sistema
El proyecto se compone de dos partes principales:

- Firmware ESP32 (tfg/): Código para el microcontrolador ESP32 que maneja la adquisición de datos
- Infraestructura de Monitoreo (tfg_telegraf_influx_grafana/): Telegraf, InfluxDB y Grafana para procesamiento y visualización

# Estructura del Proyecto
.
├── README.md
├── tfg/                                    # Firmware ESP32
│   ├── CMakeLists.txt
│   ├── components/                         # Componentes modulares
│   │   ├── communication/                  # Gestión WiFi y MQTT
│   │   ├── config/                        # Configuración del sistema
│   │   ├── diagnostics/                   # Diagnósticos y monitoreo
│   │   ├── power_manager/                 # Gestión de energía
│   │   ├── sensors/                       # Interfaz con sensores
│   │   ├── storage/                       # Almacenamiento local
│   │   ├── tasks/                         # Gestión de tareas FreeRTOS
│   │   └── utils/                         # Utilidades generales
│   ├── main/                              # Código principal
│   └── sdkconfig                          # Configuración ESP-IDF
└── tfg_telegraf_influx_grafana/           # Infraestructura de monitoreo
    ├── docker-compose.yaml
    └── telegraf/
        └── telegraf.conf
# Requisitos

# Software

ESP-IDF v5
Docker y Docker Compose
Git

# Configuración Inicial
1. Configuración de Credenciales
Antes de compilar el firmware, debes configurar las credenciales de red:
bash# Copia el archivo de ejemplo
cp tfg/components/communication/include/communication_secrets.h.example \
   tfg/components/communication/include/communication_secrets.h
Edita tfg/components/communication/include/communication_secrets.h con tus credenciales:
c// WiFi Configuration
static const char* WIFI_SSID     = "TU_SSID_AQUI"; 
static const char* WIFI_PASSWORD = "TU_PASSWORD_AQUI"; 

// MQTT Broker Configuration
static const char* MQTT_URI      = "mqtt://TU_BROKER:1883";

2. Compilación del Firmware ESP32
bashcd tfg
# Conectar el ESP32 al ordenador

# Compilar
idf.py build

# Flashear al dispositivo
idf.py flash monitor
3. Configuración de la Infraestructura
bashcd tfg_telegraf_influx_grafana

# Levantar los servicios
docker-compose up -d

# Verificar que los servicios estén corriendo
docker-compose ps
Componentes del Firmware
Communication
Gestiona la conectividad WiFi y la comunicación MQTT con el broker.
Archivos principales:

communication.c/h: Implementación de la comunicación
communication_secrets.h: Credenciales (NO incluir en git)

Config
Maneja la configuración general del sistema y parámetros de funcionamiento.
Diagnostics
Proporciona información de diagnóstico del sistema, incluyendo estado de memoria, CPU y conectividad.
Power Manager
Controla los modos de energía del ESP32 para optimizar el consumo según las necesidades de la aplicación.
Sensors
Interfaz unificada para la lectura de diferentes tipos de sensores.
Storage
Gestiona el almacenamiento local de datos cuando no hay conectividad.
Tasks
Organiza las tareas del sistema utilizando FreeRTOS para un funcionamiento concurrente eficiente.
Utils
Funciones de utilidad general utilizadas por otros componentes.
Infraestructura de Monitoreo
Telegraf
Agente de recopilación de métricas que recibe datos del ESP32 vía MQTT y los envía a InfluxDB.
InfluxDB
Base de datos que almacena los datos de los sensores.
Grafana
Plataforma de visualización que permite ver los resultados en un dashboard interactivo con los datos almacenados.
URLs de Acceso
Una vez levantada la infraestructura:

Grafana: http://localhost:3000

Usuario: admin
Password: admin


InfluxDB: http://localhost:8086

Flujo de Datos

ESP32 lee datos de los sensores
Los datos se envían vía MQTT al broker
Telegraf consume los mensajes MQTT
Los datos se almacenan en InfluxDB
Grafana visualiza los datos en tiempo real

