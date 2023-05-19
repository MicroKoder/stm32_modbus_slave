/*
 * modbus.c
 *
 *  Created on: May 19, 2023
 *      Author: Master
 */

#include "modbus.h"
#include "crc16.h"
#include <stdlib.h>
typedef struct
{
	uint16_t holdingRegCnt;
	uint16_t inputRegCnt;
	uint16_t coilCnt;
	uint16_t inputCnt;

	uint16_t *AORegisters;
	uint16_t *AIRegisters;
	uint8_t *Inputs;
	uint8_t *Coils;

	uint8_t ID;

}MODBUS_device_t;

const MODBUS_device_t MODBUS_device_init={
		.holdingRegCnt = 0,
		.inputRegCnt = 0,
		.inputCnt = 0,
		.coilCnt = 0,
		.ID = 0
};

typedef struct
{
	uint8_t deviceCount;
	MODBUS_device_t device[MB_MAX_DEVICES_COUNT];
}MODBUS_port_t;

static MODBUS_port_t mb_port[MB_PORTS_COUNT]={
		{
				.deviceCount =0,
				.device = {
						MODBUS_device_init,
						MODBUS_device_init,
						MODBUS_device_init,
						MODBUS_device_init
				}
		},
		{
				.deviceCount =0,
				.device = {
						MODBUS_device_init,
						MODBUS_device_init,
						MODBUS_device_init,
						MODBUS_device_init
				}
		}
};

static uint16_t GetAI(MODBUS_device_t* device, uint8_t nreg);
static uint16_t GetAO(MODBUS_device_t* device, uint8_t nreg);
//static void SetAI(MODBUS_device_t* device, uint8_t nreg, uint16_t value);
static void SetAO(MODBUS_device_t* device, uint8_t nreg, uint16_t value);

static bool IsAlreadyExist(uint8_t nPort, uint8_t ID)
{
	for (uint8_t i=0; i<mb_port[nPort].deviceCount; i++)
	{
		if (mb_port[nPort].device[i].ID == ID)
			return true;
	}
	return false;
}
MODBUSResult_t GetDeviceIndexByID(uint8_t nPort, uint8_t ID, uint8_t *index)
{
	for (uint8_t i=0; i<mb_port[nPort].deviceCount; i++)
	{
		if (mb_port[nPort].device[i].ID == ID)
		{
			(*index)=i;
			return MODBUS_OK;
		}
	}
	return MODBUS_ERROR;
}
MODBUSResult_t MODBUS_AddDevice(uint8_t nPort, uint8_t ID, uint16_t inputCnt, uint16_t coilCnt, uint16_t inputRegCnt, uint16_t holdingRegCnt)
{
	if (nPort>= MB_PORTS_COUNT)
		return MODBUS_ERROR;

	if (mb_port[nPort].deviceCount >= MB_MAX_DEVICES_COUNT)
		return MODBUS_ERROR;

	if (IsAlreadyExist(nPort, ID))
		return MODBUS_ERROR;

	uint8_t nDevice = mb_port[nPort].deviceCount++;
	mb_port[nPort].device[nDevice].ID = ID;

	if (holdingRegCnt>0)
		mb_port[nPort].device[nDevice].AORegisters = (uint16_t*)malloc(holdingRegCnt*sizeof(uint16_t));

	if (inputRegCnt>0)
		mb_port[nPort].device[nDevice].AIRegisters = (uint16_t*)malloc(inputRegCnt*sizeof(uint16_t));

	if (inputCnt>0)
		mb_port[nPort].device[nDevice].Inputs = (uint8_t*)malloc((inputCnt-1)/8+1);

	if (coilCnt>0)
		mb_port[nPort].device[nDevice].Coils = (uint8_t*)malloc((coilCnt-1)/8+1);

	mb_port[nPort].device[nDevice].holdingRegCnt = holdingRegCnt;
	mb_port[nPort].device[nDevice].inputRegCnt = inputRegCnt;
	mb_port[nPort].device[nDevice].inputCnt = inputCnt;
	mb_port[nPort].device[nDevice].coilCnt = coilCnt;

	return MODBUS_OK;
}

bool IsCRCValid(uint8_t *data, uint8_t len)
{
	uint16_t* pPackageCRC = (uint16_t*)&data[len-2];
	uint16_t crc = CRC16(data, len-2);
	return (*pPackageCRC) == crc;
}

MODBUSResult_t MODBUS_ProcessRequest(uint8_t nPort, uint8_t *data, uint8_t len, uint8_t *pAnswer, uint16_t *answerLen)
{
	uint8_t fc;
	uint16_t startreg;
	uint16_t nreg;
	uint16_t value;
	uint16_t crc;
	uint8_t id;
	uint8_t index=0;	//device index
	MODBUSResult_t res = MODBUS_OK;

	if (IsCRCValid(data, len))
	{
			id = data[0];
			if (GetDeviceIndexByID(nPort, id, &index) != MODBUS_OK)
				return MODBUS_ERROR;

			fc = data[1];
			startreg = (data[2]<<8) + data[3];
			nreg =  (data[4]<<8) + data[5];
			value = nreg;

			switch(fc)
			{
				case FC_READ_AI:
					if (nreg == 1)
					{
						pAnswer[0] = data[0];
						pAnswer[1] = fc;
						pAnswer[2] = 2;
						pAnswer[3] = GetAI(&mb_port[nPort].device[index], startreg)>>8;
						pAnswer[4] = GetAI(&mb_port[nPort].device[index], startreg) & 0xFF;
						crc = CRC16(pAnswer, 5);
						pAnswer[5] = crc & 0xff;
						pAnswer[6] = crc >> 8;

						(*answerLen) = 7;

					} break;
				case FC_READ_AO:

					pAnswer[0] = data[0];
					pAnswer[1] = fc;
					pAnswer[2] = nreg*2;
					for(int i=0; i< nreg; i++)
					{

						value = GetAO(&mb_port[nPort].device[index], startreg + i);

						pAnswer[i*2 + 3] = value >>8;
						pAnswer[i*2 + 4] = value & 0xFF;
					}
					crc = CRC16(pAnswer, nreg*2 + 3);
					pAnswer[nreg*2 + 3] = crc & 0xff;
					pAnswer[nreg*2 + 4] = crc >> 8;

					(*answerLen) = nreg*2 + 5;

				 break;
				case FC_WRITE_AO:

					SetAO(&mb_port[nPort].device[index], startreg, value);


					pAnswer[0] = data[0];
					pAnswer[1] = data[1];
					pAnswer[2] = data[2];
					pAnswer[3] = data[3];
					pAnswer[4] = data[4];
					pAnswer[5] = data[5];
					pAnswer[6] = data[6];
					pAnswer[7] = data[7];

					(*answerLen) = 8;

				break;
			}//switch

		}//crc valid

	return res;
}


uint16_t GetAI(MODBUS_device_t* device, uint8_t nreg)
{
	if (nreg < device->inputRegCnt)
		return device->AIRegisters[nreg];
	else
		return 0;
}

uint16_t GetAO(MODBUS_device_t* device, uint8_t nreg)
{
	if (nreg < device->holdingRegCnt)
		return device->AORegisters[nreg];
	else
		return 0;
}

/*void SetAI(MODBUS_device_t* device, uint8_t nreg, uint16_t value)
{
	if (nreg < device->inputRegCnt)
		device->AIRegisters[nreg]=value;
}*/

void SetAO(MODBUS_device_t* device, uint8_t nreg, uint16_t value)
{
	if (nreg < device->holdingRegCnt)
		device->AORegisters[nreg]=value;
}


