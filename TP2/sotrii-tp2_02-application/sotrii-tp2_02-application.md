# Actividad 2 - TP2 - RTOS II

## Paso 03

Al igual que la actividad anterior ahora se transformara el `SYS` en un objeto activo, por lo que creamos las siguientes funciones:

- `void open_sys_ao(h_sys_t *h_sys_)`.
- `void release_sys_ao(h_sys_t *h_sys_)`.
- `BaseType_t send_sys_ao(h_sys_t *h_sys_, sys_ev_t event_, TickType_t tick_out, uint8_t btn_id)`.

Que tienen el siguiente codigo:

```c
void open_sys_ao(h_sys_t *h_sys_)
{
	h_sys_->sys_ao->h_queue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
	configASSERT(NULL != h_sys_->sys_ao->h_queue);
	vQueueAddToRegistry(h_sys_->sys_ao->h_queue, h_sys_->sys_ao->queue_txt);

    BaseType_t ret;

	ret = xTaskCreate(task_sys,							/* Pointer to the function thats implement the task. */
					  h_sys_->sys_ao->task_txt,			/* Text name for the task. This is to facilitate debugging only. */
					  (configMINIMAL_STACK_SIZE),		/* Stack depth in words. */
					  (void *)h_sys_,					/* We are using the task parameter. */
					  (tskIDLE_PRIORITY + 1ul),			/* This task will run at priority 1. */
					  &h_sys_->sys_ao->h_task);			/* We are using a variable as task handle. */

    configASSERT(pdPASS == ret);

    ret = xPortGetFreeHeapSize();
}

void release_sys_ao(h_sys_t *h_sys_)
{
    vQueueUnregisterQueue(h_sys_->sys_ao->h_queue);
	vQueueDelete(h_sys_->sys_ao->h_queue);

	vTaskDelete(h_sys_->sys_ao->h_task);
}

BaseType_t send_sys_ao(h_sys_t *h_sys_, sys_ev_t event_, TickType_t tick_out, uint8_t btn_id){
	sys_dta_t sys_dta = {event_, tick_out, btn_id};

	return xQueueSend((QueueHandle_t)h_sys_->sys_ao->h_queue, &sys_dta, (TickType_t)ZERO);
}
```

Ademas, se tuvo que crear la estructura `sys_dta_t` que contiene el tiempo y evento del boton que se envia por la cola:

```c
typedef struct {
	sys_ev_t sys_ev;
	TickType_t tick;
} sys_dta_t;
```

La implementacion de este objecto activo se hará en la aplicación a desarrollar actividad 4.
Ademas, se quito la creación de la tarea `task_sys` del `app_init` y se agrego el llmado a `open` que inicializa la tarea y el queue.
