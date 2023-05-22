/*
 * app_tasks.h
 *
 *  Created on: May 18, 2023
 *      Author: Master
 */

#ifndef INC_APP_TASKS_H_
#define INC_APP_TASKS_H_

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"

//------DEVICE 1 ----------------
#define DEVICE_1_PORT 0
#define DEVICE_1_ID	1
#define MB_HOLDING_REG_COUNT 10

//----- DEVICE 2 -----------------
#define DEVICE_2_PORT 0
#define DEVICE_2_ID 2
#define MB_INPUT_REG_COUNT 8

//----- DEVICE 3 -----------------
#define DEVICE_3_PORT 1
#define DEVICE_3_ID 1
#define MB_COILS_COUNT 160

//----- DEVICE 4 -----------------
#define DEVICE_4_PORT 1
#define DEVICE_4_ID 2
#define MB_INPUTS_COUNT 160





void Initialization();
void ProcessTask1();
void ProcessTask2();

#endif /* INC_APP_TASKS_H_ */
