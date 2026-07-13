# Actividad 2 - TP1 - SOTR II
## Paso 3

Este sistema de código constituye una plantilla o esqueleto base para un desarrollo embebido utilizando **FreeRTOS** y la capa de abstracción de hardware (HAL) de STMicroelectronics. Actualmente, el sistema implementa un entorno multitarea básico con dos hilos principales ejecutándose de forma periódica, dejando preparada la estructura para el manejo de interrupciones, tareas secundarias y abstracción de periféricos.

---

### 1. `app.c` (Punto de Entrada de la Aplicación)
Es el archivo encargado de inicializar las variables globales del sistema, configurar los periféricos iniciales y crear las tareas antes de que el planificador (*scheduler*) de FreeRTOS tome el control del procesador.

* **`app_init()`**: Inicializa los contadores globales de ticks, ociosidad y desbordamiento de pila (`g_app_tick_cnt`, `g_task_idle_cnt`, `g_app_stack_overflow_cnt`).
* **Creación de Tareas**: Utiliza la función `xTaskCreate` para dar de alta dos tareas con la misma prioridad (`tskIDLE_PRIORITY + 1ul`, es decir, prioridad 1):
  * `task_sender` (asociada al manejador `h_task_sender`).
  * `task_receiver` (asociada al manejador `h_task_receiver`).
* **Inicialización de Periféricos e Interrupciones**: Llama a `open_uart` para preparar el dispositivo serie, a `app_it_init` para las interrupciones de la aplicación y a `cycle_counter_init` para el contador de ciclos de hardware.
* **Notas de diseño**: En los comentarios se observa la intención de incorporar próximamente colas (`QueueHandle_t`) y semáforos/mutex (`SemaphoreHandle_t`), esenciales para la sincronización basada en eventos, aunque actualmente no están instanciados.

---

### 2. `task_sender.c` y `task_receiver.c` (Tareas Principales)
Ambas tareas representan hilos de ejecución independientes. En este momento, poseen una lógica idéntica y simétrica orientada al tiempo:

* **Bucle Infinito**: Se ejecutan dentro de un lazo `for (;;)` del cual nunca salen, comportamiento estándar en tareas de RTOS.
* **Contadores**: Cada una incrementa su propio contador global (`g_task_sender_cnt` y `g_task_receiver_cnt`) para registrar sus ciclos de ejecución.
* **Bloqueo Temporal**: Imprimen un mensaje informativo mediante `LOGGER_INFO` y ceden el procesador invocando `vTaskDelay(pdMS_TO_TICKS(250ul))`. Al compartir la misma prioridad, el planificador las alternará de forma equitativa (Round-Robin) cada vez que venza su tiempo de espera.

---

### 3. `app_it.c` (Rutinas de Interrupción - ISR)
Este archivo gestiona las respuestas asíncronas a los eventos de hardware a través de funciones de *callback* de la HAL.

* **`HAL_GPIO_EXTI_Callback`**: Se activa cuando se detecta un cambio de estado en el pin del botón físico (`BTN_A_PIN`). Actualmente está vacío, dispuesto para albergar la lógica de disparo por eventos (ej. desbloquear una tarea mediante un semáforo).
* **`HAL_UART_TxCpltCallback`**: Se ejecuta inmediatamente cuando el periférico de hardware `USART2` finaliza la transmisión de datos. Al dispararse, levanta una bandera de estado (`hal_xxxx_callback_flag`), incrementa un contador de transmisiones y registra el tiempo exacto de ejecución en microsegundos empleando el contador de ciclos del procesador.

---

### 4. `task_uart.c` (Tareas Secundarias de UART)
Define las funciones para dos tareas orientadas a la comunicación: `task_uart_tx` (transmisión) y `task_uart_rx` (recepción).

* **Estado Actual**: Aunque sus funciones están completamente programadas en este archivo, **no están siendo creadas** en el archivo `app.c`, por lo que permanecen inactivas en el sistema.
* **Lógica**: Si se activaran, ambas tareas incrementarían contadores de rendimiento, medirían el tiempo exacto en microsegundos de sus operaciones y conmutarían el estado de un LED físico (`LED_A_PIN`) para ofrecer una respuesta visual en la placa antes de retrasarse por 250 ms.

---

### 5. `task_uart_interface.c` (Interfaz del Driver)
Proporciona las funciones de abstracción de entrada/salida (E/S) para interactuar con la UART: `open_uart`, `release_uart`, `write_uart`, `read_uart` e `ioctl_uart`.

* **Estado Actual**: Las funciones se encuentran vacías y solo emplean la macro `UNUSED()` para evitar que el compilador genere advertencias (*warnings*) por variables no utilizadas. Su propósito final es aislar la lógica de la aplicación de los comandos directos de la HAL de STM32.

---

### 6. `freertos.c` (Funciones de Gancho / Hooks del Kernel)
Contiene las funciones *Hook* (o callbacks del sistema operativo), que se ejecutan automáticamente ante eventos específicos del kernel de FreeRTOS:

* **`vApplicationIdleHook()`**: Se invoca de manera continua cuando no existe ninguna tarea de la aplicación lista para ejecutarse. Incrementa `g_task_idle_cnt` y es el lugar idóneo para implementar modos de bajo consumo (Sleep) en el microcontrolador. No debe contener funciones bloqueantes.
* **`vApplicationTickHook()`**: Se ejecuta en cada interrupción del temporizador del sistema (Tick). Incrementa la variable de tiempo global `g_app_tick_cnt`. Al ejecutarse dentro de una ISR, su código es sumamente corto y rápido.
* **`vApplicationStackOverflowHook()`**: Es un mecanismo crítico de seguridad. Si una tarea se excede del espacio de pila (*stack*) que le fue asignado en `xTaskCreate`, el kernel detecta la anomalía e invoca esta función. El código entra en una sección crítica y detiene la simulación/ejecución mediante un `configASSERT( 0 )`, previniendo comportamientos erráticos del hardware y facilitando la depuración.

---

#### Diagnóstico del Flujo de Ejecución Actual
1. El microcontrolador inicia e invoca a `app_init()`.
2. Se crean `Task Sender` y `Task Receiver` con prioridad 1, y se habilitan las rutinas de interrupción.
3. Al iniciar el planificador, ambas tareas se ejecutan secuencialmente en el mismo milisegundo (Round-Robin), imprimen su respectivo log y se bloquean por 250 ms.
4. Durante esos 250 ms en los que las tareas principales están inactivas, el procesador ejecuta indefinidamente la tarea `vApplicationIdleHook`. Cada 1 ms, el reloj del sistema interrumpe momentáneamente el estado para procesar el `vApplicationTickHook`.

## Paso 06
Lo que se hizo fue implementar las siguientes funciones de `task_uart_interface`:
- `open_uart()`.
- `close_uart()`.
- `read_uart()`.
- `write_uart()`.

Además se implemento el gatekeeper para la lectura/escritura de la `UART` y las tareas de prueba para los llamados a uart_interface.

La secuencia de lectura comienza con el llamado a `read_uart`. Esta funcion recibe un buffer donde se guarda la información recibida y el handler de donde se espera esta comunicación.
La función espera que la `queue` donde se almacenan los punteros a memoria con los datos tenga algun elemento. Una vez se obtuvo, copia los datos del spooler y libera la memoria.
```c
void read_uart(UART_HandleTypeDef *h_uart_device, uint8_t *msg, size_t msg_len) {
	if(msg ==NULL || msg_len<SPOOL_LEN) return;
	task_uart_dta.device_id = h_uart_device;

	// Check which version of the uart triggered this function
	if (task_uart_dta.device_id == h_uart_device) {
		uint8_t * p_mem;
		if(pdPASS == xQueueReceive(task_uart_dta.queue_rx, &p_mem, portMAX_DELAY)){
			memcpy(msg, p_mem, spooler_LEN);
			vPortFree(p_mem);
            // Procesamiento de datos
		}
	}
}
```

Este snippet es el codigo de la tarea gatekeeper para la recepcion. En esta se reserva la memoria para el spooler de datos y se habilita la interrupcion para la recepcion por UART.
Con un semaforo, se sincroniza cuando los datos estan listos para enviar por la queue. Este proceso se repite indefinidamente ya que la memoria se libera desde `read_uart`.
```c
uint8_t *msg = pvPortMalloc(spooler_LEN);
HAL_UART_Receive_IT(p_task_uart_rx_dta->device_id, msg, 128);

for (;;) {
	/* Update Task Counter */
	g_task_xxxx_rx_cnt++;

	cycle_counter_reset();
	g_task_xxxx_rx_runtime_us = cycle_counter_get_time_us();
	xSemaphoreTake(p_task_uart_rx_dta->ready_rx, portMAX_DELAY);
	xQueueSend(p_task_uart_rx_dta->queue_rx, &msg, portMAX_DELAY);

	msg = pvPortMalloc(spooler_LEN);
	HAL_UART_Receive_IT(p_task_uart_rx_dta->device_id, msg, sizeof(msg));

	/* Print out: Wait 250mS */
	LOGGER_INFO(p_task_uart_rx_wait_250mS);
	vTaskDelay(TASK_XXXX_DEL_MAX);
}
```


Finalmente, desde la interrupcion se da la señal por el semaforo de que el spooler esta lleno.
```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		hal_xxxx_callback_flag = true;
		hal_xxxx_callback_cnt++;

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(task_uart_dta.ready_rx, xHigherPriorityTaskWoken);
		hal_xxxx_callback_runtime_us = cycle_counter_get_time_us();
	}
}
```


El siguiente log contiene el comienzo solo con la lectura habilitada. Una vez llega a `read_uart` la tarea se queda esperando a que el callback de el semaforo.
```
[info]  
[info] app_init is running - Tick [mS] = 0
[info]  app is a RTOS - Event-Triggered Systems (ETS)
[info]  app is a sotrii-tp1_02-application: Demo Code
[info]  app is a (Source => CESE - Sistemas Operativos de Tiempo Real
[info]  
[info] Task UART Rx is running - Tick [mS] =   0
[info]  
[info]   Task Receiver is running - Tick [mS] = 0
[info]  
[info] Task UART Tx is running - Tick [mS] =   0
```

Una vez se envian 128 bytes (el tamaño del spooler) el callback de la interrupcion da el semaforo lo que libera a `task_uart_rx` para que envie por la queue el puntero a los datos. Esto se recibe por `read_uart` que copia el mensaje y libera la memoria.

El siguiente log representa la continuación del anterior segmento despues de una recepcion exitosa. En esta se observa el contenido del mensaje y como ambas tareas se liberan.
```
[info]    ==> Task UART RX - Wait:   250mS
[info] alkdfjsaljflsasaldkfjsaldfjaslkefjsaeiofsaenasodvldsanvpasifdi
[info]    ==> Task RECEIVER - Wait:   250mS
```


Se pudo medir el WCET de las funciones de transmision/recepcion dando los siguientes valores:
- `Transmit`: 4us
- `Receive`: 5us
Este es el tiempo que tarda la linea de codigo en dar los datos al buffer. Si la data es mayor a 16 bytes se tardaria mas tiempo ya que el buffer interno del perififerico es de 16 bytes.

Las pruebas se hicieron transmitiendo el string "Test\r\n" y recibiendo por un spooler de 128 bytes.

Cabe aclarar que en el codigo se desactivo la tarea `sender` ya que `receiver` tiene un mayor grado de complejidad.
