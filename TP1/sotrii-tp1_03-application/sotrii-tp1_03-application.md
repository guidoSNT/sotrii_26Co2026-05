# Actividad 3 - TP1 - SOTR II

## Paso 3

Este conjunto de archivos implementa una arquitectura base para un sistema embebido utilizando **FreeRTOS** sobre un microcontrolador STM32 (basado en la biblioteca HAL de ST). El diseño está alineado con el paradigma de **Sistemas Orientados a Eventos (Event-Triggered Systems - ETS)** y estructurado para la adquisición de datos de un ADC junto con el procesamiento concurrente de tareas.

---

### 1. `app.c` (Inicialización de la Aplicación)

Este archivo constituye el punto de entrada principal para la lógica de la aplicación bajo el sistema operativo de tiempo real.

- **Función `app_init()`**: Configura el entorno inicial antes de que el planificador (_scheduler_) comience a correr.
- **Inicialización de Variables**: Resetea los contadores globales de rendimiento y control, como el contador de ticks (`g_app_tick_cnt`), el contador de la tarea Idle (`g_task_idle_cnt`) y el de desbordamiento de pila (`g_app_stack_overflow_cnt`).
- **Diagnóstico**: Envía mensajes informativos al sistema de log (`LOGGER_INFO`) detallando el nombre de la aplicación y el estado del Tick.
- **Creación de Tareas**: Utiliza la API `xTaskCreate()` para registrar de forma explícita la tarea **`Task Receiver`** con una prioridad de `tskIDLE_PRIORITY + 1ul` (prioridad 1). Valida el éxito de esta operación mediante un macro de aserción `configASSERT(pdPASS == ret)`.
- **Control de Hardware y Ciclos**: Inicializa el "Driver" simulado del ADC llamando a `open_adc(&hadc1)`, configura las interrupciones de la aplicación (`app_it_init()`) e inicia el contador de ciclos por hardware (`cycle_counter_init()`) para mediciones temporales precisas en microsegundos.

---

### 2. `task_receiver.c` (Tarea Receptora)

Define el comportamiento de una de las tareas principales del sistema, encargada de simular la recepción o procesamiento de datos.

- **Función `task_receiver()`**: Implementa un bucle infinito (`for (;;)`) estándar en tareas de FreeRTOS.
- **Funcionamiento**: En cada iteración incrementa su contador global de ejecuciones (`g_task_receiver_cnt`) y escribe en el log que se encuentra en estado de espera.
- **Bloqueo Voluntario**: Invoca a `vTaskDelay(TASK_RECEIVER_DEL_MAX)`, lo que suspende la tarea durante **250 ms**. Esto libera el núcleo de procesamiento para que tareas de menor prioridad (como la tarea Idle) puedan ejecutarse en ese intervalo de tiempo.

---

### 3. `task_adc.c` (Tarea de Muestreo ADC)

Define la lógica para una tarea secundaria orientada al procesamiento o captura de datos del ADC (`task_adc_rx`).

- **Función `task_adc_rx()`**: Al igual que el receptor, se ejecuta dentro de un ciclo infinito (`for (;;)`).
- **Funcionamiento**:
  1. Incrementa el contador de ejecuciones de la tarea (`g_task_xxxx_rx_cnt`).
  2. Reinicia el contador de ciclos del procesador (`cycle_counter_reset()`).
  3. Realiza un cambio de estado en un pin digital (`HAL_GPIO_TogglePin`) sobre el puerto y pin asignados a `LED_A_PIN`, sirviendo como indicador visual de actividad en la placa.
  4. Mide el tiempo exacto que tomó realizar la acción mediante `cycle_counter_get_time_us()` y lo almacena en microsegundos (`g_task_xxxx_rx_runtime_us`).
  5. Se bloquea de forma voluntaria por **250 ms** empleando `vTaskDelay(TASK_XXXX_DEL_MAX)`.
- _Nota de arquitectura_: Aunque la función está completamente definida, en el archivo `app.c` actual **no se evidencia una llamada a `xTaskCreate` para esta tarea**, por lo que se encuentra inactiva a menos que se agregue explícitamente a la inicialización.

---

### 4. `task_adc_interface.c` y `task_adc_interface.h` (Capa de Abstracción de Hardware)

Estos archivos proveen una interfaz limpia y desacoplada (estilo driver de sistema operativo) para interactuar con el periférico ADC.

- **Funciones Declaradas**: `open_adc()`, `release_adc()`, `write_adc()`, `read_adc()`, y `ioctl_adc()`.
- **Estado Actual**: Actúan como funciones de marcación de posición (_stubs_ o funciones vacías). Utilizan el macro `UNUSED(h_adc_device)` para mitigar las advertencias del compilador por variables sin usar. Su propósito es aislar el código de la aplicación de las llamadas directas a las funciones de bajo nivel de la HAL de ST (como `HAL_ADC_Start` o `HAL_ADC_GetValue`).

---

### 5. `app_it.c` (Gestión de Interrupciones)

Centraliza los métodos de respuesta (_callbacks_) de las interrupciones de hardware (ISR) del microcontrolador.

- **Función `app_it_init()`**: Prepara las variables compartidas de las ISR. Deshabilita temporalmente las interrupciones globales mediante ensamblador (`__asm("CPSID i")`) para asegurar la exclusión mutua durante la inicialización de banderas, y las vuelve a habilitar inmediatamente después (`__asm("CPSIE i")`).
- **Callback `HAL_GPIO_EXTI_Callback()`**: Se activa cuando ocurre una interrupción externa en los pines GPIO. Filtra si el evento fue gatillado por el botón `BTN_A_PIN` para ejecutar código asociado.
- **Callback `HAL_ADC_ConvCpltCallback()`**: Se dispara de forma asíncrona por hardware cuando el ADC1 finaliza una conversión en modo no bloqueante. Modifica una bandera global (`hal_xxxx_callback_flag = true`), incrementa el contador de interrupciones del ADC y almacena el tiempo de ejecución de la conversión en microsegundos.

---

### 6. `freertos.c` (Hooks del Sistema Operativo)

Contiene las funciones _Hook_ (o callbacks del Kernel) que FreeRTOS invoca automáticamente ante eventos internos específicos.

- **`vApplicationIdleHook()`**: Se ejecuta de manera continua dentro de la tarea `Idle` (la de menor prioridad) únicamente cuando ninguna otra tarea del sistema está lista para ejecutarse. Incrementa el contador `g_task_idle_cnt`. Es el espacio ideal para colocar al microcontrolador en estados de bajo consumo energético.
- **`vApplicationTickHook()`**: Se invoca en cada interrupción del reloj del sistema (el Tick del RTOS). Al ejecutarse dentro del contexto de una ISR de alta frecuencia, se limita a incrementar de forma rápida el contador de tiempo de la aplicación `g_app_tick_cnt`.
- **`vApplicationStackOverflowHook()`**: Actúa como un mecanismo crítico de seguridad. Si el sistema operativo detecta que el consumo de pila de alguna tarea ha excedido su límite asignado, detiene inmediatamente la ejecución normal entrando en una sección crítica y congelando el flujo mediante `configASSERT(0)` para facilitar la depuración.

---

### Resumen del Flujo de Ejecución

1. El sistema arranca y ejecuta `app_init()`, inicializando los contadores, el driver periférico y la tarea `Task Receiver`.
2. Una vez lanzado el planificador de FreeRTOS, `Task Receiver` toma el control, ejecuta sus acciones, escribe en el log y se bloquea voluntariamente por 250 ms.
3. Durante ese tiempo de bloqueo, el procesador ejecuta la tarea `Idle` (registrada por el Hook) reduciendo el consumo.
4. En paralelo, cualquier evento de hardware como la finalización de una muestra en el ADC interrumpe el flujo normal para ser atendido de forma inmediata por los callbacks de `app_it.c`.

## Paso 06

Lo que se hizo fue implementar las siguientes funciones de `task_adc_interface`:

- `open_adc()`.
- `close_adc()`.
- `read_adc()`.
- `write_adc()`.

Esta estructura contiene los handlers para el ADC, la tarea del gatekeeper y para el semaforo que indica la finalizacion de la toma de muestras por parte del DMA.
```c
typedef struct {
	ADC_HandleTypeDef *device_id;
	TaskHandle_t task_rx;
	SemaphoreHandle_t dma_adc_semph;
} task_adc_dta_t;
```

`read_adc` es la funcion del interface que comienza la lectura. Como el buffer que contiene los datos es accedido por esta funcion, el gatekeeper y el DMA, se utiliza el semaforo `dma_adc_semph` para sincronizar su utilizacion. En este caso, espera a poder leer los ultimos datos disponibles.
```c
void read_adc(ADC_HandleTypeDef *h_adc_device, uint16_t *adc_buff, size_t adc_buff_len) {
	if (adc_buff == NULL || adc_buff_len < ADC_RX_BUFF_LENGTH)
		return;
	task_adc_dta_t *p_task_adc_dta = &task_adc_dta;

	if (h_adc_device != p_task_adc_dta->device_id)
		return;

	xSemaphoreTake(p_task_adc_dta->dma_adc_semph, portMAX_DELAY);

	for(uint8_t i = 0; i< ADC_RX_BUFF_LENGTH; i++){
		adc_buff[i] = adc_rx_buff[i];
	}

	xSemaphoreGive(p_task_adc_dta->dma_adc_semph);
}
```

El siguiente codigo contiene el cuerpo del gatekeeper donde una vez se toma el semaforo se comienzan a tomar las muestras con el DMA. Posteriormente espera a la finalizacion del la lectura (el callback da un give al semaforo) para finalizar la lectura por DMA.
```c
for (;;)
{
	/* Update Task Counter */
	g_task_xxxx_rx_cnt++;

	xSemaphoreTake(task_adc_dta.dma_adc_semph, portMAX_DELAY);

	cycle_counter_reset();
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_rx_buff, ADC_RX_BUFF_LENGTH);

	xSemaphoreTake(task_adc_dta.dma_adc_semph, portMAX_DELAY);
	HAL_ADC_Stop_DMA(&hadc1);
	xSemaphoreGive(task_adc_dta.dma_adc_semph);

	g_task_xxxx_rx_runtime_us = cycle_counter_get_time_us();

	/* Print out: Wait 250mS */
	LOGGER_INFO(p_task_adc_rx_wait_250mS);
	vTaskDelay(TASK_XXXX_DEL_MAX);
}
```

El siguiente log es la secuencia de arranque y algunas de las muestras tomadas con un potenciometro:
```
[info] app_init is running - Tick [mS] = 0
[info]  app is a RTOS - Event-Triggered Systems (ETS)
[info]  app is a sotrii-tp1_03-application: Demo Code
[info]  app is a (Source => CESE - Sistemas Operativos de Tiempo Real
[info]  
[info] Task ADC Rx is running - Tick [mS] =   0
[info]    ==> Task ADC RX - Wait:   250mS
[info]  
[info]   Task Receiver is running - Tick [mS] = 0
[info] 2830 2669 2605 2569 2524 2518 2528 2508 2508 2492
[info]    ==> Task RECEIVER - Wait:   250mS
[info]    ==> Task ADC RX - Wait:   250mS
[info] 2607 2477 2398 2366 2338 2300 2324 2296 2298 2322
```

Finalmente el llamado a `HAL_ADC_Start_DMA` tarda aproximadamente 36 us. Cabe remarcar que el DMA es mas efectivo utilizarlo cuando se quiere adquirir un gran volumen de datos.
