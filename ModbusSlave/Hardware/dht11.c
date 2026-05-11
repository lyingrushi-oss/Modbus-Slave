/**
 * @file    dht11.c
 * @brief   DHT11 传感器驱动实现
 *
 * 协议时序参考 DHT11 datasheet（AOSONG）：
 *
 *  主机起始信号：
 *    拉低 ≥ 18ms → 释放（拉高）→ 等待 20~40μs
 *
 *  传感器响应：
 *    拉低 80μs → 拉高 80μs
 *
 *  数据位（40 bit = 8bit湿度整+8bit湿度小+8bit温度整+8bit温度小+8bit校验）：
 *    每 bit 起始：低电平 ~50μs
 *    bit = 0：高电平 ~26~28μs
 *    bit = 1：高电平 ~70μs
 *    判断阈值：高电平持续时间 > 40μs → bit=1
 */

#include "dht11.h"
#include "dht11_hal.h"

/* ------------------------------------------------------------------ */
/*  内部常量                                                            */
/* ------------------------------------------------------------------ */

#define DHT11_BITS          40U     /* 总数据位数 */
#define DHT11_TIMEOUT_US    100U    /* 单次电平等待超时 μs */

/* ------------------------------------------------------------------ */
/*  内部工具函数                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief 等待数据引脚变为指定电平，超时返回错误
 * @param level    期望的电平值（0或1）
 * @param timeout  最大等待时间，单位 μs
 * @return 实际等到目标电平所用时间（μs），超时返回 0xFFFF
 */
static uint16_t wait_level(uint8_t level, uint16_t timeout)
{
    uint16_t elapsed = 0U;
    while (dht11_hal_pin_read() != level) {
        dht11_hal_delay_us(1U);
        elapsed++;
        if (elapsed >= timeout) {
            return 0xFFFFU;   /* 超时标志 */
        }
    }
    return elapsed;
}

/* ------------------------------------------------------------------ */
/*  公共接口实现                                                        */
/* ------------------------------------------------------------------ */

DHT11_Status_t dht11_read(DHT11_Data_t *out)
{
    uint8_t  raw[5]  = {0};   /* 40 bit → 5 byte 原始数据 */
    uint16_t elapsed = 0U;

    /* ---- 1. 主机发送起始信号 ---- */
    dht11_hal_pin_output();
    dht11_hal_pin_write(0);        /* 拉低 */
    HAL_Delay(20);                 /* ≥ 18ms，用HAL ms延时即可 */
    dht11_hal_pin_write(1);        /* 释放总线 */
    dht11_hal_delay_us(30U);       /* 等待 20~40μs */

    /* ---- 2. 切换为输入，等待传感器响应 ---- */
    dht11_hal_pin_input();

    /* 等待传感器拉低（响应起始低电平，~80μs） */
    elapsed = wait_level(0U, DHT11_TIMEOUT_US);
    if (elapsed == 0xFFFFU) {
        return DHT11_ERR_NORESP;
    }

    /* 等待传感器释放（响应高电平，~80μs） */
    elapsed = wait_level(1U, DHT11_TIMEOUT_US);
    if (elapsed == 0xFFFFU) {
        return DHT11_ERR_NORESP;
    }

    /* 等待高电平结束，准备接收第一个 bit */
    elapsed = wait_level(0U, DHT11_TIMEOUT_US);
    if (elapsed == 0xFFFFU) {
        return DHT11_ERR_NORESP;
    }

    /* ---- 3. 接收 40 bit 数据 ---- */
    for (uint8_t i = 0U; i < DHT11_BITS; i++) {
        /* 每 bit 以 ~50μs 低电平开头，等待其结束 */
        elapsed = wait_level(1U, DHT11_TIMEOUT_US);
        if (elapsed == 0xFFFFU) {
            return DHT11_ERR_TIMEOUT;
        }

        /* 测量高电平持续时间来判断 bit 值 */
        dht11_hal_delay_us(40U);   /* 40μs 采样点：0 已结束，1 仍在高 */

        raw[i / 8U] <<= 1U;
        if (dht11_hal_pin_read() == 1U) {
            raw[i / 8U] |= 0x01U; /* bit = 1 */
        }

        /* 等待当前 bit 高电平结束 */
        elapsed = wait_level(0U, DHT11_TIMEOUT_US);
        if (elapsed == 0xFFFFU) {
            return DHT11_ERR_TIMEOUT;
        }
    }

    /* ---- 4. 校验和验证 ---- */
    uint8_t checksum = (uint8_t)(raw[0] + raw[1] + raw[2] + raw[3]);
    if (checksum != raw[4]) {
        return DHT11_ERR_CRC;
    }

    /* ---- 5. 填充输出结构体 ---- */
    out->humi_int = raw[0];
    out->humi_dec = raw[1];
    out->temp_int = raw[2];
    out->temp_dec = raw[3];

    return DHT11_OK;
};
