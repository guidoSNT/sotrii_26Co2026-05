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
#include "task_adc.h"
#include "task_adc_attribute.h"
#include "task_adc_interface.h"

/********************** macros and definitions *******************************/

/********************** internal data declaration ****************************/

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/
task_adc_dta_t task_adc_dta;
extern uint16_t adc_rx_buff[];
/********************** external functions definition ************************/
/* Interface functions */
void open_adc(ADC_HandleTypeDef *h_adc_device) {
	if (h_adc_device == NULL)
		return;
	task_adc_dta_t *p_task_adc_dta = &task_adc_dta;

	p_task_adc_dta->device_id = h_adc_device;

	BaseType_t ret = xTaskCreate(task_adc_rx, "Task ADC Rx",
			(configMINIMAL_STACK_SIZE), (void*) p_task_adc_dta,
			(tskIDLE_PRIORITY + 1ul), &p_task_adc_dta->task_rx);
	configASSERT(pdPASS == ret);

	p_task_adc_dta->dma_adc_semph = xSemaphoreCreateBinary();
	xSemaphoreGive(p_task_adc_dta->dma_adc_semph);
	vQueueAddToRegistry(p_task_adc_dta->dma_adc_semph, "Task ADC Rx");
}

void release_adc(ADC_HandleTypeDef *h_adc_device) {
	task_adc_dta_t *p_task_adc_dta = &task_adc_dta;
	p_task_adc_dta->device_id = h_adc_device;

	if (p_task_adc_dta->device_id == h_adc_device) {
		vTaskDelete(p_task_adc_dta->task_rx);
		vSemaphoreDelete(p_task_adc_dta->dma_adc_semph);
	}
}

void write_adc(ADC_HandleTypeDef *h_adc_device) {
	// Does nothing. There is nothing to write.
}

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

void ioctl_adc(ADC_HandleTypeDef *h_adc_device) {
	/* Prevent unused argument(s) compilation warning */
	UNUSED(h_adc_device);
}

/********************** end of file ******************************************/
