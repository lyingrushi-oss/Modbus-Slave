#ifndef __MODBUS_H
#define __MODBUS_H

#include "stm32f1xx_hal.h"
#include "register.h"

#define SLAVE_ID 0x01//定义从机ID



#define MODBUS_READ_HOLD_MAX_COUNT         125   // 读保持寄存器 (FC03)
#define MODBUS_READ_INPUT_MAX_COUNT        125   // 读输入寄存器 (FC04)



// ===== 功能码 =====
typedef enum {
    FC_READ_COIL      = 0x01,   // 读取线圈状态 (位，可读写)
    FC_READ_DISCRETE  = 0x02,   // 读取离散输入状态 (位，只读)
    FC_READ_HOLD      = 0x03,   // 读取保持寄存器 (16位，可读写)
    FC_READ_INPUT     = 0x04,   // 读取输入寄存器 (16位，只读)
    FC_WRITE_COIL     = 0x05,   // 写单个线圈
    FC_WRITE_HOLD     = 0x06    // 写单个保持寄存器
} modbus_func_code;

// ===== 异常码 =====
typedef enum {
    EX_ILLEGAL_FUNC   = 0x01,   // 不支持的功能码
    EX_ILLEGAL_ADDR   = 0x02,   // 地址越界
    EX_ILLEGAL_VALUE  = 0x03    // 数据值非法
} modbus_ex_code;

//对外函数接口
void modbus_init(UART_HandleTypeDef* huart,DMA_HandleTypeDef* dma);
uint16_t modbus_crc16(uint8_t* buf,uint16_t len);
void modbus_poll(void);

#endif
