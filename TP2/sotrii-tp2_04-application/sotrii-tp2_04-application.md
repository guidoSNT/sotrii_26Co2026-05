# Actividad 4 - TP2 - RTOS II

## Paso 03

Para el desarrollo de la aplicacion propuesta se crearon:

- Dos AO `btn`: `BTN_A` (B1) y `BTN_B` (D7) que envian el evento y tiempo.
- Un AO `sys`: Este contiene el statechart de la solución particular a esta aplicación. Fue el unico que debio modificarse de los objetos activos.
- Tres AO `led`: `LED_A` (LD2), `LED_B`(D12) y `LED_C`(D11).

El siguiente codigo es el statechart de `sys`:

```c
void task_sys_statechart(h_sys_t *h_sys_) {
	switch (h_sys_->sys_sc->state) {
		case ST_SYS_IDLE:
			if (EV_SYS_OFF == h_sys_->sys_sc->ev_in) {
				if(h_sys_->sys_sc->tick_out <= THRESHOLD_MS){
					h_sys_->sys_sc->state = ST_SYS_SHORT_PUSH;
					h_sys_->sys_sc->tick = ZERO;

					if(h_sys_->sys_sc->btn_id >= 2) break;
					send_led_ao(&h_led[h_sys_->sys_sc->btn_id],EV_SYS_BLINK, h_sys_->sys_sc->tick_out);
					send_led_ao(&h_led[LED_C],EV_SYS_ON, h_sys_->sys_sc->tick_out);
				} else {
					if(h_sys_->sys_sc->btn_id == 0){
						h_sys_->sys_sc->timeout_a = h_sys_->sys_sc->tick_out;
					} else {
						h_sys_->sys_sc->timeout_b = h_sys_->sys_sc->tick_out;
					}
				}
			}
			else
			{
				h_sys_->sys_sc->tick += DEL_SYS_MIN;
			}

			break;

		case ST_SYS_SHORT_PUSH:

			h_sys_->sys_sc->tick += DEL_SYS_MIN;
			if(h_sys_->sys_sc->btn_id == 0){
				if(h_sys_->sys_sc->timeout_a - h_sys_->sys_sc->tick == ZERO){
					send_led_ao(&h_led[LED_A], EV_SYS_ON, ZERO);
					send_led_ao(&h_led[LED_B], EV_SYS_ON, ZERO);
					send_led_ao(&h_led[LED_C], EV_SYS_OFF, ZERO);
					h_sys_->sys_sc->state = ST_SYS_IDLE;
				}
			} else if(h_sys_->sys_sc->btn_id == 1){
				if(h_sys_->sys_sc->timeout_b - h_sys_->sys_sc->tick == ZERO){
					send_led_ao(&h_led[LED_A], EV_SYS_ON, ZERO);
					send_led_ao(&h_led[LED_B], EV_SYS_ON, ZERO);
					send_led_ao(&h_led[LED_C], EV_SYS_OFF, ZERO);
					h_sys_->sys_sc->state = ST_SYS_IDLE;
				}
			}
			break;
	}
}
```

Esta statechart tiene los siguientes estados:

- `ST_SYS_IDLE`: Este estado espera a que el usuario presione y suelte un botón. Si el botón fue presionado un tiempo menor a `THRESHOLD_MS` entonces se considera una pulsación breve. En caso contrario, se considera una pulsación prolongada que modifica el tiempo de blinkeo del led asociado al boton según cuanto tiempo se presionó el mismo.
- `ST_SYS_SHORT_PUSH`: Durante este estado, se verifica constantemente si se cumple el tiempo de blinkeo a traves de `h_sys_->sys_sc->timeout_b(a) - h_sys_->sys_sc->tick` tanto para `BTN_A` como para `BTN_B`. En caso de que se cumpla el tiempo de parpadeo, se vuelve a `ST_SYS_IDLE`, que es el estado base en donde los leds A y B se encuentran encendidos y el led C apagado.

Desde el `app_init` se crearon los siguientes AOs:

```c
/* Active Objects Open */
open_btn_ao(&h_btn[BTN_A]);
open_btn_ao(&h_btn[BTN_B]);
open_sys_ao(&h_sys);
open_led_ao(&h_led[LED_A]);
open_led_ao(&h_led[LED_B]);
open_led_ao(&h_led[LED_C]);
```

Video:

https://drive.google.com/file/d/1LKmrjj-nXqPLtrk5qGtEmjKyYM5chEDS/view?usp=sharing
