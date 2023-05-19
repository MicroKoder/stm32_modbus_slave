/*
 * modbus.h
 *
 *  Created on: May 19, 2023
 *      Author: Master
 */

#ifndef INC_MODBUS_H_
#define INC_MODBUS_H_

#include <stdbool.h>
#include <stdint-gcc.h>

#define FC_READ_AO 3
#define FC_READ_AI 4
#define FC_WRITE_AO 6

#define MB_PORTS_COUNT 2
#define MB_MAX_DEVICES_COUNT 4

typedef enum
{
	MODBUS_OK,
	MODBUS_ERROR
}
MODBUSResult_t;

MODBUSResult_t MODBUS_AddDevice(uint8_t nPort, uint8_t ID, uint16_t inputCnt, uint16_t coilCnt, uint16_t inputRegCnt, uint16_t holdingRegCnt);
MODBUSResult_t MODBUS_ProcessRequest(uint8_t nPort, uint8_t *data, uint8_t len, uint8_t *pAnswer, uint16_t *answerLen);

#endif /* INC_MODBUS_H_ */
