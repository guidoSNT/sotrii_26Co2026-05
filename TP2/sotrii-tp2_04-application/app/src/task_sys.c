/*
 * Copyright (c) 2026 Sebastian Bedin <sebabedin@gmail.com> &
 * 					  Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author : Sebastian Bedin <sebabedin@gmail.com> &
 * 			 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 */

/********************** inclusions *******************************************/
/* Project includes */
#include "main.h"
#include "cmsis_os.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "app.h"
#include "task_sys_attribute.h"
#include "task_sys_interface.h"
#include "task_led_attribute.h"
#include "task_led_interface.h"
#include "task_led.h"

/********************** macros and definitions *******************************/
#define G_TASK_SYS_CNT_INI	0ul

#define DEL_SYS_MIN			(pdMS_TO_TICKS(50ul))
#define DEL_SYS_BLINK		(pdMS_TO_TICKS(500ul))

#define TASK_SYS_DEL_ZERO	(pdMS_TO_TICKS(0ul))
#define TASK_SYS_DEL_MAX	DEL_SYS_MIN

#define TIMEOUT_BTN_A (pdMS_TO_TICKS(5000ul))
#define TIMEOUT_BTN_B (pdMS_TO_TICKS(10000ul))

#define THRESHOLD_MS (pdMS_TO_TICKS(600ul))

/********************** internal data declaration ****************************/
sys_sc_t sys_sc = {ST_SYS_IDLE, EV_SYS_OFF, ZERO, EV_SYS_NONE, ZERO, 0xFF, 	TIMEOUT_BTN_A, TIMEOUT_BTN_B};

sys_ao_t sys_ao = {NULL,"Queue SYS_A AO", NULL, "Task SYS_A AO"};

sys_dta_t sys_dta = {EV_SYS_NONE, ZERO};
/********************** internal functions declaration ***********************/
void task_sys_statechart(h_sys_t *h_sys_);

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/
uint32_t g_task_sys_cnt;

h_sys_t h_sys = {&sys_sc, &sys_ao, &sys_dta};

/********************** external functions definition ************************/
/* Task thread */
void task_sys(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
	g_task_sys_cnt = G_TASK_SYS_CNT_INI;
	h_sys_t *p_h_sys = (h_sys_t *)parameters;
	BaseType_t ret;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

	send_led_ao(&h_led[LED_A], EV_SYS_ON, ZERO);
	send_led_ao(&h_led[LED_B], EV_SYS_ON, ZERO);
	send_led_ao(&h_led[LED_C], EV_SYS_OFF, ZERO);

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;)
    {
		/* Update Task Counter */
		g_task_sys_cnt++;

		/* Get Events to excite Statechart */
		ret = xQueueReceive(p_h_sys->sys_ao->h_queue, (void *)p_h_sys->sys_dta, (TickType_t)ZERO);

		if(pdFAIL == ret){
			p_h_sys->sys_sc->ev_in = EV_SYS_NONE;
			p_h_sys->sys_sc->tick_out = 0;
		} else {
			p_h_sys->sys_sc->ev_in = p_h_sys->sys_dta->sys_ev;
			p_h_sys->sys_sc->tick_out = p_h_sys->sys_dta->tick;
			p_h_sys->sys_sc->btn_id = p_h_sys->sys_dta->btn_id;
		}

		/* Run Statechart */
    	task_sys_statechart(p_h_sys);

    	/* We want this task to execute every 50 milliseconds. */
		vTaskDelay(TASK_SYS_DEL_MAX);
	}
}

void task_sys_statechart(h_sys_t *h_sys_)
{
	switch (h_sys_->sys_sc->state)
	{
		case ST_SYS_IDLE:

			if (EV_SYS_OFF == h_sys_->sys_sc->ev_in)
			{
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

/********************** end of file ******************************************/
