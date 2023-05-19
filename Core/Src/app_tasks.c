/*
 * app_tasks.c
 *
 *  Created on: May 18, 2023
 *      Author: Master
 */


#include "app_tasks.h"
#include <stdbool.h>
#include "modbus.h"
#define RX_BUF_SIZE 256
#define TX_BUF_SIZE 256
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

typedef struct{
	uint8_t rxData[RX_BUF_SIZE];
	uint8_t txData[TX_BUF_SIZE];
	uint16_t txLen;
	uint16_t receivedCount;
	uint16_t prevCount;
	UART_HandleTypeDef *huart;
} port_data_t;

typedef struct
{
	port_data_t portData[2];
} appData_t;

appData_t appData={
	.portData =	{
					{
							.prevCount = 0,
							.receivedCount = 0,
							.huart = &huart1
					},
					{
							.prevCount = 0,
							.receivedCount = 0,
							.huart = &huart2
					}
			}
};

typedef enum {

	APP_BUSY,
	APP_FINISHED

}APPReceiveStatus_t;

bool IsReceivingIdle(port_data_t *port)
{
	return port->huart->RxXferCount == port->prevCount;
}
APPReceiveStatus_t processReceive(port_data_t *port)
{
	APPReceiveStatus_t res = APP_BUSY;
	port->receivedCount = RX_BUF_SIZE - port->huart->RxXferCount;
	if (IsReceivingIdle(port) && (port->receivedCount>0))
	{

		//рестарт приема
		HAL_UART_AbortReceive_IT(port->huart);
		HAL_UART_Receive_IT(port->huart, port->rxData, RX_BUF_SIZE);
		res = APP_FINISHED;

	}
	port->prevCount =port->huart->RxXferCount;
	return res;
}
void processTask1()
{
	HAL_UART_Receive_IT(&huart1, appData.portData[0].rxData, RX_BUF_SIZE);
	for(;;)
	{
		if (processReceive(&appData.portData[0])== APP_FINISHED)
		{
			//HAL_UART_Transmit_IT(appData.portData[0].huart, appData.portData[0].rxData, appData.portData[0].receivedCount);
			if (MODBUS_ProcessRequest(0, appData.portData[0].rxData, appData.portData[0].receivedCount, appData.portData[0].txData, &appData.portData[0].txLen)==MODBUS_OK)
				HAL_UART_Transmit_IT(appData.portData[0].huart, appData.portData[0].txData, appData.portData[0].txLen);
		}
		osDelay(1);
	}

}

void processTask2()
{
	HAL_UART_Receive_IT(&huart2, appData.portData[1].rxData, RX_BUF_SIZE);
		for(;;)
		{
			if (processReceive(&appData.portData[1])== APP_FINISHED)
			{
			//	HAL_UART_Transmit_IT(appData.portData[1].huart, appData.portData[0].rxData, appData.portData[0].receivedCount);
			}
			osDelay(1);
		}
}

