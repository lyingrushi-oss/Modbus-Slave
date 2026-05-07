#ifndef __REGISTER_H
#define __REGISTER_H

#include "stm32f1xx_hal.h"


#define REG_HOLD_NUM 8//保持寄存器数量
#define REG_INPUT_NUM 8//输入寄存器数量
#define COIL_NUM 8 //线圈数量
#define DISCRETE_NUM 8//离散输入状态数量

extern uint16_t hold_reg[REG_HOLD_NUM];
extern uint16_t input_reg[REG_INPUT_NUM];
extern uint8_t  coil_reg[COIL_NUM];
extern uint8_t discrete_input[DISCRETE_NUM];



#endif

