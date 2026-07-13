/*
 * TMP117_port.h
 *
 *  Created on: 12 abr 2026
 *      Author: Mauro
 */

#ifndef API_INC_TMP117_PORT_H_
#define API_INC_TMP117_PORT_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Lee un registro específico del sensor a través del bus I2C.
 * * Función de abstracción de hardware (capa de puerto) para leer datos crudos
 * desde el sensor TMP117.
 * @param[in] address Dirección I2C del dispositivo (slave address).
 * @param[in] regAddress Dirección del registro interno que se desea leer.
 * @param[out] pData Puntero al buffer donde se almacenarán los bytes recibidos.
 * @return true Operación exitosa.
 * @return false Error en el bus I2C o dispositivo no responde.
 */
bool readRegister16(uint16_t address, uint8_t regAddress, uint8_t *pData);

/**
 * @brief Escribe un valor de 16 bits en un registro del sensor.
 * * Envía un comando de escritura por I2C para configurar parámetros internos
 * del TMP117.
 * @param[in] address Dirección I2C del dispositivo.
 * @param[in] regAddress Dirección del registro interno donde se va a escribir.
 * @param[in] data Valor de 16 bits que se desea escribir en el registro.
 * @return true Escritura confirmada.
 * @return false Error durante la transferencia de datos.
 */
bool writeRegister16(uint16_t address, uint8_t regAddress, uint16_t data);

#endif /* API_INC_TMP117_PORT_H_ */
