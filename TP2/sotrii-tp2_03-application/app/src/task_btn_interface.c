/*
 * Copyright (c) 2026 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
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
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
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
#include "app_it.h"
#include "task_btn.h"
#include "task_btn_attribute.h"

/********************** macros and definitions *******************************/
#define QUEUE_LENGTH_       (5)
#define QUEUE_ITEM_SIZE_    (sizeof(btn_ev_t))

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/

/********************** external functions definition ************************/
/* Interface functions */
void open_btn_ao(h_btn_t *h_btn_)
{
	BaseType_t ret;
  	h_btn_->btn_ao->h_queue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
	configASSERT(NULL != h_btn_->btn_ao->h_queue);
	vQueueAddToRegistry(h_btn_->btn_ao->h_queue,h_btn_->btn_ao->queue_txt);

    ret = xTaskCreate(task_btn,
    				  h_btn_->btn_ao->task_txt,
					  (configMINIMAL_STACK_SIZE),
					  (void *)h_btn_,
					  (tskIDLE_PRIORITY + 1ul),
					  &h_btn_->btn_ao->h_task);

    configASSERT(pdPASS == ret);
}

void release_btn_ao(h_btn_t *h_btn_)
{
    vQueueUnregisterQueue(h_btn_->btn_ao->h_queue);
	vQueueDelete(h_btn_->btn_ao->h_queue);

	vTaskDelete(h_btn_->btn_ao->h_task);
}

BaseType_t send_btn_ao(h_btn_t *h_btn_, void *event_){
	return xQueueSend((QueueHandle_t)h_btn_->btn_ao->h_queue, event_, (TickType_t)ZERO);
}

void ioctl_btn_ao(h_btn_t *h_btn_)
{
	/* Prevent unused argument(s) compilation warning */
	UNUSED(h_btn_);
}

/********************** end of file ******************************************/
