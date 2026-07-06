/*
 * TMP117.h
 *
 *  Created on: 9 abr 2026
 *      Author: Mauro
 */

#ifndef API_INC_API_TMP117_H_
#define API_INC_API_TMP117_H_

#include "TMP117_port.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include "main.h"

typedef struct {
	float temperature;
}temp_t;

/**
 * @brief Lee el identificador único del sensor TMP117.
 * * Esta función consulta el registro de identificación del dispositivo para verificar
 * que la comunicación I2C sea correcta y que el modelo de sensor sea el esperado.
 * * @param[out] id Puntero a una variable de 16 bits donde se almacenará el ID leído.
 * @return true Si el ID se leyó correctamente.
 * @return false Si hubo un error en la comunicación I2C.
 */
bool TMP117_readID(uint16_t*);

/**
 * @brief Obtiene la temperatura actual medida por el sensor TMP117.
 * * Lee el registro de temperatura y realiza la conversión interna a grados Celsius
 * según la resolución del sensor (7.8125 mC por LSB).
 * * @param[out] temp Puntero a un float donde se guardará el valor de temperatura en °C.
 * @return true Si la lectura y conversión fueron exitosas.
 * @return false Si falló la comunicación con el sensor.
 */
bool TMP117_readTemperature(float *);

/**
 * @brief Escribe un valor de 16 bits en uno de los registros de la EEPROM.
 * @param index Índice de la EEPROM (1, 2 o 3).
 * @param data Valor a guardar.
 * @return true si la escritura fue exitosa.
 */
bool TMP117_writeEEPROM(uint8_t index, uint16_t data);

/**
 * @brief Lee un valor de 16 bits de la EEPROM.
 * @param index Índice de la EEPROM (1, 2 o 3).
 * @param data Puntero donde se guarda el valor leido
 */
bool TMP117_readEEPROM(uint8_t index, uint16_t *data);

/**
 * @brief Lee el registro de configuración del sensor.
 * @param[out] config Puntero donde se almacenará el valor de 16 bits del registro.
 * @return true Si la lectura fue exitosa.
 */
bool TMP117_readConfig(uint16_t *config);

/**
 * @brief Setea los límites de alerta (Temperatura alta/baja).
 * @param[in] highTemp Temperatura de limite alto
 * @param[in] lowTemp Temperatura de limite bajo
 * @return true Si la lectura fue exitosa.
 */
bool TMP117_setLimits(float highTemp, float lowTemp);

/**
 * @brief Lee los límites configurados actualmente en el sensor.
 * @param[out] highTemp Puntero para almacenar el límite superior en °C.
 * @param[out] lowTemp Puntero para almacenar el límite inferior en °C.
 */
bool TMP117_getLimits(float *highTemp, float *lowTemp);

#endif /* API_INC_API_TMP117_H_ */
