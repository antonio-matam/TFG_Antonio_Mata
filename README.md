# TFG: Sistema de Adquisición de Datos con ESP32

## Estructura del Proyecto

- **main/**: Código principal con `app_main`.
- **components/**: Módulos separados:
  - **power_manager**: Gestión de modos de bajo consumo.
  - **sensors**: Lectura de ultrasonidos, celda de carga y láser.
  - **storage**: Almacenamiento en NVS.
  - **communication**: Comunicación Wi-Fi y MQTT (sin cJSON).
  - **config**: Carga de parámetros de configuración.
  - **diagnostics**: Registros y diagnósticos.
  - **utils**: Funciones auxiliares.
- **docs/**: Documentación del proyecto.

## Diagrama de Bloques

![Diagrama de Bloques](./diagram.png)

## Flujo de Estados

1. **Arranque**: Inicialización de diagnósticos y configuración.
2. **Inicializar Componentes**: `power_manager`, `sensors`, `storage`, `communication`.
3. **Revisar Modo Bajo Consumo**: Si debe
entrar en deep sleep, se entra.
4. **Tarea Principal**: Lectura de sensores → Almacenamiento → Envío por MQTT → Espera.
