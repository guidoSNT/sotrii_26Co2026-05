# Actividad 1 - TP1 - SOTR II

## Paso 3

## Análisis del Sistema Basado en FreeRTOS (ETS)

Este sistema está basado en el sistema operativo de tiempo real **FreeRTOS** y sigue un modelo de diseño **ETS (Event-Triggered Systems)**, enfocado en la comunicación asincrónica a través de un bus I2C mediante colas de mensajes (`Queues`).

A continuación, se detalla el análisis y funcionamiento de cada uno de los archivos del código fuente.

---

### 1. Arquitectura General y Flujo de Inicialización (`app.c`)

El archivo **`app.c`** actúa como el orquestador principal del sistema. Su función es preparar el entorno de FreeRTOS antes de que comience el planificador (_scheduler_).

- **`app_init()`**: Inicializa variables globales de control como los contadores de ticks del sistema, tiempo ocioso y desbordamiento de pila.
- **Creación de tareas**: Instancia las dos tareas principales de la aplicación (`Task Sender` y `Task Receiver`) asignándoles un tamaño de stack equivalente al doble del mínimo (`2 * configMINIMAL_STACK_SIZE`) y una prioridad de `tskIDLE_PRIORITY + 1ul`.
- **Inicialización del subsistema I2C**: Llama a `open_i2c(&hi2c1)`, que se encarga de crear las subtareas y colas necesarias para manejar el hardware I2C de forma aislada y segura.
- **Interrupciones y contadores**: Configura las interrupciones mediante `app_it_init()` e inicia el contador de ciclos del procesador (`DWT`) para permitir mediciones de tiempo de ejecución precisas.

---

### 2. Abstracción y Manejo del Driver (`task_i2c_interface.c`)

Este archivo implementa el patrón de diseño de interfaz de software para interactuar con el bus I2C de forma indirecta, evitando que las tareas de usuario bloqueen directamente el hardware o compitan entre sí.

- **`open_i2c()`**:
  - Crea dos colas de FreeRTOS: una de transmisión (`queue_tx`, para 5 elementos de tipo `task_i2c_tx_dta_t`) y otra de recepción (`queue_rx`, para 10 bytes).
  - Registra las colas en las herramientas de depuración mediante `vQueueAddToRegistry`.
  - Crea internamente dos tareas dedicadas a nivel de driver: `Task I2C Tx` y `Task I2C Rx` con prioridad `tskIDLE_PRIORITY + 1ul`.
- **`write_i2c()`**: Es la función pública que consumen otras tareas. En lugar de transmitir directamente por el bus, empaqueta la dirección del dispositivo (`dev_address`) y el dato (`dev_data`), y los envía de manera segura a `queue_tx` usando `xQueueSend` con espera indefinida (`portMAX_DELAY`).
- **`release_i2c()`**: Realiza la limpieza del driver desregistrando y eliminando tanto las colas como las subtareas asociadas para liberar memoria en tiempo de ejecución.
- **`read_i2c()` / `ioctl_i2c()`**: Están declaradas pero actualmente vacías (con la macro `UNUSED` para evitar advertencias de compilación).

---

### 3. Tareas del Controlador I2C (`task_i2c.c`)

Contiene el código que corre en bucle infinito para las subtareas del driver encargadas de la interacción directa con el periférico.

- **`task_i2c_tx` (Transmisión)**:
  - Inicializa variables de diagnóstico propias y entra en un lazo infinito.
  - Se bloquea en `xQueueReceive` esperando que otra tarea solicite escribir en el bus I2C.
  - Al recibir un elemento, resetea el contador de ciclos e invoca la función de la capa HAL de ST: `HAL_I2C_Master_Transmit()`. El desplazamiento `address << 1` se realiza para adaptar la dirección de 7 bits al formato de lectura/escritura de la biblioteca HAL.
  - Mide el tiempo exacto de la operación en microsegundos (`g_task_xxxx_tx_runtime_us`), imprime un log informativo y se duerme por 250 ms (`vTaskDelay`).
- **`task_i2c_rx` (Recepción)**:
  - Actualmente no lee del bus físico. En su lugar, incrementa su propio contador, conmuta un pin de hardware (`LED_A_PIN`) para señalización visual, registra el tiempo de ejecución en microsegundos y se duerme por 250 ms.

---

### 4. Tareas de Aplicación (`task_sender.c` y `task_receiver.c`)

Estas representan la lógica de usuario de la aplicación en la capa superior.

- **`task_sender.c` (Productor)**:
  - Configura por defecto la dirección de un dispositivo esclavo (`0x27`) y un dato inicial (`0x55`), comunes para un módulo expansor I2C PCF8574 (típicamente usado en pantallas LCD).
  - En su bucle infinito, realiza un parpadeo lógico de los bits del dato invirtiendo su valor (`~dev_data`).
  - Invoca a `write_i2c(&hi2c1, dev_address, dev_data)` para encolar el mensaje y luego se bloquea por 250 ms.
- **`task_receiver.c` (Consumidor simplificado)**:
  - En esta versión del código posee una lógica mínima. Su función actual es incrementar su contador de ejecuciones (`g_task_receiver_cnt`), imprimir que está activa en los logs y ceder el procesador retrasándose por 250 ms.

---

### 5. Manejo de Interrupciones (`app_it.c`)

Maneja los eventos de hardware asincrónicos externos al flujo continuo de las tareas.

- **`app_it_init()`**: Muestra cómo deshabilitar y habilitar las interrupciones globales a nivel de ensamblador (`CPSID i` / `CPSIE i`) para proteger secciones críticas durante la inicialización.
- **`HAL_GPIO_EXTI_Callback()`**: Es la función de respuesta (callback) disparada por hardware ante una interrupción por flanco en los pines de entrada. El código evalúa si el origen fue el botón `BTN_A_PIN`. Aunque el cuerpo está vacío (`/* Work to be done. */`), este es el lugar diseñado para notificar a tareas del RTOS utilizando las funciones seguras para interrupciones del kernel (ej. `xQueueSendFromISR` o `xSemaphoreGiveFromISR`).

---

### 6. Funciones de Gancho del Sistema Operativo (`freertos.c`)

Este archivo contiene los _Hooks_ (callbacks) globales que FreeRTOS invoca de forma automática ante eventos internos del kernel:

- **`vApplicationIdleHook()`**: Se ejecuta repetidamente cuando ninguna tarea de la aplicación está lista para correr (la CPU está ociosa). Incrementa `g_task_idle_cnt` y es el punto ideal para colocar al procesador en estados de bajo consumo energético. No debe bloquearse nunca.
- **`vApplicationTickHook()`**: Se ejecuta dentro de la ISR del temporizador del sistema (Tick del RTOS). Incrementa `g_app_tick_cnt`. Al correr dentro de una interrupción, es muy corta y ligera.
- **`vApplicationStackOverflowHook()`**: Se dispara si el mecanismo de protección de FreeRTOS detecta que una tarea excedió el límite de su memoria de pila (Stack) asignada. Detiene la ejecución de forma segura entrando en una sección crítica y congelando el microcontrolador con un `configASSERT(0)` para facilitar la depuración por hardware.

---

#### Flujo del Ciclo de Datos (ETS)

1. **`task_sender`** genera un nuevo dato (`~dev_data`) y solicita su envío asincrónico mediante `write_i2c()`.
2. **`write_i2c`** deposita de manera segura los parámetros estructurados dentro de `queue_tx`.
3. El _scheduler_ despierta a **`task_i2c_tx`** (que estaba bloqueada en la cola).
4. **`task_i2c_tx`** toma los datos y realiza la transferencia física real llamando a la función bloqueante del hardware `HAL_I2C_Master_Transmit()` sin afectar los tiempos directos de la tarea de aplicación.

## Paso 6

Para la comunicacion I2C se utilizo el periferico [TMP117](https://www.ti.com/lit/ds/symlink/tmp117.pdf), un sensor de temperatura digital que posee
registros de lectura y escritura. La direccion de estos es de 8 bits, mientras que el contenido de estos es de 16 bits.

Para la lectura de los registros primero se hace un Transmit con la dirección del registro que se quiere leer. Luego, se hace un Receive para recibir el valor de este registro (en 16 bits).
Dado esto, utilizar una tarea de transmision y otra de recepcion no es adecuado para la lectura de registros. Para adaptarnos a esto, lo que se hizo fue utilizar la tarea `sender` para transmitir la direccion del registro `device_id` que vale 0x117. Luego la tarea `receiver` hace un I2C_Receive para leer este valor.

Aun asi para estos perifericos es conveniente hacer un Receive y un Transmit en la misma tarea de manera consecutiva.

Se pudo medir el WCET de las funciones de transmision como recepcion, dando los siguientes valores:

- `Transmit`: 190us
- `Receive`: 281us

Se deja un log de lo recibido durante la lectura del registro:
LOG:
```
[info]    ==> Task RECEIVER - Wait:   250mS
[info]    ==> Task I2C TX - Wait:   250mS
[info]    ==> Task I2C RX - Wait:   250mS
[info]    ==> Task SENDER - Wait:   250mS
[info] received: 117
[info]    ==> Task RECEIVER - Wait:   250mS
[info]    ==> Task I2C TX - Wait:   250mS
[info]    ==> Task I2C RX - Wait:   250mS
[info]    ==> Task SENDER - Wait:   250mS
[info] received: 117
[info]    ==> Task RECEIVER - Wait:   250mS
[info]    ==> Task I2C TX - Wait:   250mS
[info]    ==> Task I2C RX - Wait:   250mS
[info]    ==> Task SENDER - Wait:   250mS
[info] received: 117
[info]    ==> Task RECEIVER - Wait:   250mS
[info]    ==> Task I2C TX - Wait:   250mS
[info]    ==> Task I2C RX - Wait:   250mS
[info]    ==> Task SENDER - Wait:   250mS
[info] received: 117
[info]    ==> Task RECEIVER - Wait:   250mS
[info]    ==> Task I2C TX - Wait:   250mS
[info]    ==> Task I2C RX - Wait:   250mS
[info]    ==> Task SENDER - Wait:   250mS
[info] received: 117
[info]    ==> Task RECEIVER - Wait:   250mS
[info]    ==> Task I2C TX - Wait:   250mS
[info]    ==> Task I2C RX - Wait:   250mS
[info]    ==> Task SENDER - Wait:   250mS

```

Finalmente se deja la implementacion del bucle para el gatekeeper de lectura (implementada para 16 bits):
```c
for (;;)
{
	task_i2c_rx_dta_t task_i2c_rx_dta = {.address = 0x48, .data = 0x00};

	uint8_t dt[2];

	/* Update Task Counter */
	g_task_xxxx_rx_cnt++;

	cycle_counter_reset();

	if(HAL_OK != HAL_I2C_Master_Receive(p_task_i2c_rx_dta->device_id, (task_i2c_rx_dta.address << 1), dt, 2, HAL_MAX_DELAY)){
		continue;
	}

	g_task_xxxx_rx_runtime_us = cycle_counter_get_time_us();

	task_i2c_rx_dta.data = (dt[0] << 8) | dt[1]; //debo hacer un desplazamiento de 8 posiciones porque el primer byte que se recibe es el MSB y luego hago una OR con el segundo byte (data[1])

	/* Print out: Wait 250mS */
	xQueueSend(p_task_i2c_rx_dta->queue_rx, &task_i2c_rx_dta.data , portMAX_DELAY);

	LOGGER_INFO(p_task_i2c_rx_wait_250mS);
	vTaskDelay(TASK_XXXX_DEL_MAX);
}
```
