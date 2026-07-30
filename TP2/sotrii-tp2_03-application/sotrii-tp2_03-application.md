# Actividad 3 - TP2 - RTOS II

## Paso 03

Como ultima actividad previa al desarrollo de la aplicación se reformulo el boton para transformarlo en un AO al igual que el `SYS` y el `LED`.

A diferencia de los dos anteriores, este AO genera sus propios eventos por lo que la cola no sera necesario utilizarla ni crearla.

Se crearon las siguientes funciones:

- `void open_btn_ao(h_btn_t *h_btn_)`.
- `void release_btn_ao(h_btn_t *h_btn_)`.
- `BaseType_t send_btn_ao(h_btn_t *h_btn_, void *event_)`.

Que tiene el siguiente codigo:

```c
void open_btn_ao(h_btn_t *h_btn_) {
	BaseType_t ret = xTaskCreate(task_btn,
    				 h_btn_->btn_ao->task_txt,
					 (configMINIMAL_STACK_SIZE),
					 (void *)h_btn_,
					 (tskIDLE_PRIORITY + 1ul),
					 &h_btn_->btn_ao->h_task);

    configASSERT(pdPASS == ret);
}

void release_btn_ao(h_btn_t *h_btn_) {
    vQueueUnregisterQueue(h_btn_->btn_ao->h_queue);
	vQueueDelete(h_btn_->btn_ao->h_queue);

	vTaskDelete(h_btn_->btn_ao->h_task);
}

BaseType_t send_btn_ao(h_btn_t *h_btn_, void *event_){
	UNUSED(h_btn_);
	UNUSED(event_);
	return pdPASS;
}
```

Ademas, se quito la creación de la tarea `task_btn` del `app_init` y se agrego el llmado a `open` que inicializa la tarea.
