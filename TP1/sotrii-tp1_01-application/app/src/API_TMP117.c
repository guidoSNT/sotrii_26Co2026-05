/*
 * API_TMP117.c
 *
 *  Created on: 9 abr 2026
 *      Author: Mauro
 */

#include "API_TMP117.h"


#define SENSOR_ADDR (0x48 << 1) // direccion del sensor de la placa izquierda
//#define SENSOR_ADDR (0x49 << 1)  // direccion del sensor de placa derecha
#define ID_ADDR 0x0F // Registro de device ID
#define TEMP_ADDR 0X00 // Registro para leer la temperatura
#define EEPROM1_ADDR 0x05 //registro de la eeprom 1
#define EEPROM2_ADDR 0x06 //registro de la eeprom 2
#define EEPROM3_ADDR 0x08 //registro de la eeprom 3
#define CONFIG_ADDR 0x01 // registro de configuracion
#define HIGH_LIMIT_ADDR 0x02 //registro de high temperature
#define LOW_LIMIT_ADDR  0x03 //registro de low temperature

bool TMP117_readTemperature(float *temp){

	if(temp == NULL)
	{
		return false;
	}
	uint8_t data[2];
	uint16_t device_temp = 0;
	float actual_temp = 0;
	bool status;

	status = readRegister16(SENSOR_ADDR, TEMP_ADDR,data);

	if (status == true)
	{
		  		  device_temp = (data[0] << 8) | data[1]; //debo hacer un desplazamiento de 8 posiciones porque el primer byte que se recibe es el MSB y luego hago una OR con el segundo byte (data[1])
		  		  actual_temp = (float)device_temp * 0.0078125f; //casteo a float para multiplicar por la resolucion (indicado por hoja de datos)
		  		  *temp = actual_temp;
		  		  return true;
	}
	else
	{
		return false;
	}

}

bool TMP117_readID(uint16_t *id){

	if(id == NULL)
	{
		return false;
	}
	uint8_t data[2];
	uint16_t device_id = 0;
	uint16_t actual_id = 0;
	bool status;

	status = readRegister16(SENSOR_ADDR, ID_ADDR,data); //leo el ID del sensor y se almacena en data

	if (status == true)
	{
		device_id = (data[0] << 8) | data[1]; //debo hacer un desplazamiento de 8 posiciones porque el primer byte que se recibe es el MSB y luego hago una OR con el segundo byte (data[1])
		actual_id = device_id & 0x0FFF; //se enmascara con 12 bits porque el address son los 12 bits LSB, los bits 15 a 12 estan reservados
		*id = actual_id;
		return true;
	}
	else
	{
		return false;
	}


}

bool TMP117_writeEEPROM(uint8_t index, uint16_t data) {
    uint8_t regAddr;

    // Mapeo de índice a dirección de registro
    switch(index) {
        case 1: regAddr = EEPROM1_ADDR; break;
        case 2: regAddr = EEPROM2_ADDR; break;
        case 3: regAddr = EEPROM3_ADDR; break;
        default: return false; // Índice inválido
    }

    if (!writeRegister16(SENSOR_ADDR, regAddr, data)) {
        return false;
    }

    HAL_Delay(10); //le doy 10ms de delay para que pueda escribir en la eeprom

    return true;
}

bool TMP117_readEEPROM(uint8_t index, uint16_t *data) {
    if (data == NULL)
    {
    	return false;
    }
    uint8_t regAddr;
    uint8_t rawData[2];

    switch(index) {
        case 1: regAddr = EEPROM1_ADDR; break;
        case 2: regAddr = EEPROM2_ADDR; break;
        case 3: regAddr = EEPROM3_ADDR; break;
        default: return false;
    }

    if (readRegister16(SENSOR_ADDR, regAddr, rawData)) {
        *data = (uint16_t)((rawData[0] << 8) | rawData[1]);
        return true;
    }
    return false;
}

bool TMP117_readConfig(uint16_t *config) {
    if (config == NULL)
    {
        return false;
    }

    uint8_t data[2];
    bool status;

    status = readRegister16(SENSOR_ADDR, CONFIG_ADDR, data);

    if (status == true) {
        *config = (uint16_t)((data[0] << 8) | data[1]);
        return true;
    } else {
        return false;
    }
}

bool TMP117_setLimits(float highTemp, float lowTemp) {
    // El registro es de 16 bits yhago al conversion inversa para guarda el valor

    uint16_t high = (int16_t)(highTemp / 0.0078125f);

    uint16_t low = (int16_t)(lowTemp / 0.0078125f);

    if(writeRegister16(SENSOR_ADDR, 0x02, (uint16_t)high))
    {
    	if(writeRegister16(SENSOR_ADDR, 0x03, (uint16_t)low))
    	{
    		return true;
    	}
    }

    return false;
}

bool TMP117_getLimits(float *highTemp, float *lowTemp) {
    if (highTemp == NULL || lowTemp == NULL)
    {
    	return false;
    }

    uint8_t data[2];
    int16_t rawLimit;

    // Leer límite alto
    if (readRegister16(SENSOR_ADDR, HIGH_LIMIT_ADDR, data)) {
        rawLimit = (int16_t)((data[0] << 8) | data[1]);
        *highTemp = (float)rawLimit * 0.0078125f;
    }
    else
    {
        return false;
    }

    // Leer límite bajo
    if (readRegister16(SENSOR_ADDR, LOW_LIMIT_ADDR, data))
    {
        rawLimit = (int16_t)((data[0] << 8) | data[1]);
        *lowTemp = (float)rawLimit * 0.0078125f;
        return true;
    }

    return false;
}


