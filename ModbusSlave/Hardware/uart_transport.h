#ifndef __UART_TRANSPORT_H
#define __UART_TRANSPORT_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

void uart_transport_init(UART_HandleTypeDef* modbus_huart);
void uart_transport_poll(uint8_t* buf,uint8_t* buf_len,uint8_t* flag_ready);
void rs485_send(uint8_t* buf,uint8_t len);

#endif
