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

#define MB_INPUTS_COUNT 160
#define MB_COILS_COUNT 160
#define MB_INPUT_REG_COUNT 8
#define MB_HOLDING_REG_COUNT 10


void Initialization();
void ProcessTask1();
void ProcessTask2();

#endif /* INC_APP_TASKS_H_ */
