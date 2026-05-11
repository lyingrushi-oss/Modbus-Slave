/**
 * @file    dht11_hal.h
 * @brief   DHT11 HAL适配层 —— 平台相关接口
 *
 * 该层封装所有与STM32 HAL库的耦合。
 * 若移植到其他平台，只需重新实现本文件对应的 .c 即可。
 */

#ifndef DHT11_HAL_H
#define DHT11_HAL_H

#include <stdint.h>
#include "stm32f1xx_hal.h"   /* 根据你的芯片系列修改，如 stm32f4xx_hal.h */

/* ------------------------------------------------------------------ */
/*  硬件配置：按实际接线修改这两个宏                                    */
/* ------------------------------------------------------------------ */
#define DHT11_GPIO_PORT     GPIOA
#define DHT11_GPIO_PIN      GPIO_PIN_1

/* ------------------------------------------------------------------ */
/*  平台接口声明                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief 将DHT11数据引脚配置为推挽输出模式
 */
void dht11_hal_pin_output(void);

/**
 * @brief 将DHT11数据引脚配置为浮空输入模式
 */
void dht11_hal_pin_input(void);

/**
 * @brief 设置数据引脚电平
 * @param level  1 = 高电平，0 = 低电平
 */
void dht11_hal_pin_write(uint8_t level);

/**
 * @brief 读取数据引脚电平
 * @return 1 = 高电平，0 = 低电平
 */
uint8_t dht11_hal_pin_read(void);

/**
 * @brief 微秒级延时（阻塞）
 * @param us  延时时间，单位 μs
 *
 * 实现方式依赖 DWT 计数器（Cortex-M3/M4/M7）。
 * 需要在 SystemClock_Config 之后调用 dht11_hal_delay_init() 初始化。
 */
void dht11_hal_delay_us(uint32_t us);

/**
 * @brief 初始化微秒延时所需的 DWT 计数器
 *        在 main() 中 SystemClock_Config() 之后调用一次即可。
 */
void dht11_hal_delay_init(void);

#endif /* DHT11_HAL_H */
