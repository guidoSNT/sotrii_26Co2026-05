# Análisis y Explicación del Código Fuente (Sistemas Operativos de Tiempo Real)

Este documento presenta un análisis detallado del funcionamiento del firmware basado en FreeRTOS, diseñado siguiendo un modelo de **Event-Triggered Systems (ETS)** (Sistemas Disparados por Eventos). El código se estructura en tareas desacopladas que se comunican mediante colas (*queues*) y máquinas de estados finitos (*statecharts*).

---

## 1. Visión General de la Arquitectura (`app.c`)

El archivo `app.c` actúa como el núcleo de inicialización de la aplicación. Su función principal es preparar el entorno de ejecución antes de que el planificador (*scheduler*) de FreeRTOS tome el control total.

### Inicializaciones Clave:
* **Contadores Globales:** Inicializa los contadores de ticks, inactividad (*idle*) y desbordamientos de pila (*stack overflow*).
* **Creación de Colas de Mensajes:**
  * `h_sys_task_q`: Cola utilizada para la comunicación entre la tarea de botones y la tarea del sistema (`BTN -> SYS`). Capacidad de 5 elementos.
  * `h_led_task_q`: Cola utilizada para la comunicación entre la tarea del sistema y la tarea de LEDs (`SYS -> LED`). Capacidad de 1 elemento.
* **Creación de Tareas (Threads):** Se crean cinco hilos con diferentes prioridades:
  * **Task A y Task B:** Creadas con alta prioridad (`tskIDLE_PRIORITY + 2ul`), operan como tareas de procesamiento general.
  * **Task Btn, Task Sys y Task Led:** Creadas con prioridad base (`tskIDLE_PRIORITY + 1ul`), encargadas de la gestión de entradas, lógica del sistema y actuadores respectivamente.
* **Subsistemas:** Inicializa las interrupciones de la aplicación (`app_it_init`) y el contador de ciclos de hardware (`cycle_counter_init`).

---

## 2. Gestión de Entradas y Antirrebote (`task_btn.c` y `task_btn_attribute.h`)

La tarea de botones (`task_btn`) digitaliza y procesa el estado físico de los pulsadores mediante una máquina de estados por muestreo periódico.

### Atributos (`task_btn_attribute.h`):
* **IDs de Botones:** Define los botones disponibles (`BTN_A`, `BTN_B`) y el total (`BTN_QTY`).
* **Eventos (`btn_ev_t`):** `EV_BTN_UP` (liberado) y `EV_BTN_DOWN` (presionado).
* **Estados (`btn_st_t`):** `ST_BTN_UP` y `ST_BTN_DOWN`.
* **Estructuras:** Contienen la configuración física del pin GPIO (`btn_t`), los parámetros de la máquina de estados (`btn_sc_t`) y el manipulador global de la tarea (`h_btn_t`).

### Funcionamiento (`task_btn.c`):
1. **Bucle Infinito y Retardo:** Se ejecuta cada **50 ms** (`DEL_BTN_MIN`), asegurando un muestreo constante.
2. **Lectura de Pines:** Lee el estado físico actual del pin mediante la HAL de STM32 (`HAL_GPIO_ReadPin`).
3. **Máquina de Estados (`task_btn_statechart`):**
   * Evalúa transiciones entre `ST_BTN_UP` y `ST_BTN_DOWN`.
   * Al detectar un cambio de estado válido (flanco), genera un evento de salida (`ev_out`).
   * Envía inmediatamente el evento a la cola del sistema (`h_sys_task_q`) utilizando `xQueueSend` sin bloqueo (`TickType_t ZERO`).

---

## 3. Lógica y Orquestación del Sistema (`task_sys.c` y `task_sys_attribute.h`)

La tarea del sistema (`task_sys`) opera como la capa intermedia que interpreta los eventos de los botones y decide el comportamiento global del dispositivo.

### Atributos (`task_sys_attribute.h`):
* **Eventos del Sistema (`sys_ev_t`):** Vinculados directamente a los eventos de los botones (`EV_SYS_OFF = EV_BTN_UP`, `EV_SYS_ON = EV_BTN_DOWN`), además de `EV_SYS_BLINK` y `EV_SYS_NONE`.
* **Estados del Sistema (`sys_st_t`):**
  * `ST_SYS_IDLE`: Estado de reposo.
  * `ST_SYS_ACTIVE_0`: Primer nivel de actividad.
  * `ST_SYS_ACTIVE_1`: Segundo nivel de actividad.

### Funcionamiento (`task_sys.c`):
1. **Recepción de Eventos:** Cada 50 ms, intenta extraer un evento de la cola `h_sys_task_q` usando `xQueueReceive`. Si no hay mensajes, asigna `EV_SYS_NONE`.
2. **Máquina de Estados (`task_sys_statechart`):**
   * **En `ST_SYS_IDLE`:** Ante un evento `EV_SYS_ON`, transiciona a `ST_SYS_ACTIVE_0` y envía el comando de encendido (`EV_SYS_ON`) a la cola de LEDs.
   * **En `ST_SYS_ACTIVE_0`:** Ante un nuevo `EV_SYS_ON`, transiciona a `ST_SYS_ACTIVE_1` y envía el comando de parpadeo (`EV_SYS_BLINK`) a la cola de LEDs.
   * **En `ST_SYS_ACTIVE_1`:** Ante un nuevo `EV_SYS_ON`, retorna a `ST_SYS_IDLE` y envía el comando de apagado (`EV_SYS_OFF`) a la cola de LEDs.

---

## 4. Gestión de Actuadores - LEDs (`task_led.c` y `task_led_attribute.h`)

La tarea de LEDs (`task_led`) controla de forma independiente el comportamiento visual de los indicadores lumínicos de la placa.

### Atributos (`task_led_attribute.h`):
* **IDs de LEDs:** Define múltiples LEDs (`LED_A`, `LED_B`, `LED_C`, `LED_QTY`).
* **Eventos y Estados:** Heredan y extienden los comandos del sistema (`EV_LED_OFF`, `EV_LED_ON`, `EV_LED_BLINK`) con estados para manejo fijo (`ST_LED_OFF`, `ST_LED_ON`) y dinámico (`ST_LED_BLINK`).

### Funcionamiento (`task_led.c`):
1. **Recepción:** Monitorea la cola `h_led_task_q` cada 50 ms para capturar instrucciones emitidas por `task_sys`.
2. **Máquina de Estados (`task_led_statechart`):**
   * **Estados Fijos (`ST_LED_OFF` / `ST_LED_ON`):** Modifican directamente el pin GPIO mediante `HAL_GPIO_WritePin` según el evento recibido. Si reciben `EV_LED_BLINK`, configuran el temporizador interno y cambian al estado de parpadeo.
   * **Estado de Parpadeo (`ST_LED_BLINK`):** Decrementa un contador de ticks (`tick`). Al llegar a cero, alterna el estado del pin con `HAL_GPIO_TogglePin` y reinicia el intervalo de parpadeo (`DEL_LED_BLINK` de 500 ms).

---

## 5. Resumen de Interacción Arquitectónica

El flujo completo del firmware sigue una tubería basada en eventos asíncronos:
1. **Hardware (Pulsador)** -> **`task_btn`** (Muestreo y Antirrebote).
2. **`task_btn`** -> **`task_sys`** (A través de la cola `BTN->SYS`, gestionando la Máquina de Estados Central).
3. **`task_sys`** -> **`task_led`** (A través de la cola `SYS->LED`, controlando los Actuadores).
