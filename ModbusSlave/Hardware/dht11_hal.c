	/**
 * @file    dht11_hal.c
 * @brief   DHT11 HAL适配层实现（STM32 HAL + DWT微秒延时）
 */

#include "dht11_hal.h"

/* ------------------------------------------------------------------ */
/*  引脚操作                                                            */
/* ------------------------------------------------------------------ */

void dht11_hal_pin_output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;   /* 推挽输出 */
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

void dht11_hal_pin_input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;            /* 外部已有上拉电阻 */
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

void dht11_hal_pin_write(uint8_t level)
{
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN,
                      level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

uint8_t dht11_hal_pin_read(void)
{
    return (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)
           ? 1u : 0u;
}

/* ------------------------------------------------------------------ */
/*  微秒延时（DWT实现，不占用定时器资源）                               */
/* ------------------------------------------------------------------ */

void dht11_hal_delay_init(void)
{
    /* 使能 DWT */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

#if defined(__CORTEX_M) && (__CORTEX_M == 7U)
    DWT->LAR = 0xC5ACCE55U;   /* Cortex-M7 需要解锁 */
#endif

    DWT->CYCCNT = 0U;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;   /* 启动周期计数器 */
}

void dht11_hal_delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    /* SystemCoreClock 单位是 Hz，即每秒的时钟周期数 */
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks) {
        /* 忙等待 */
    }
}

