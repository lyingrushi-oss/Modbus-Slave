/**
 * @file    dht11.h
 * @brief   DHT11 传感器驱动抽象层
 *
 * 该层只依赖 dht11_hal.h 定义的平台无关接口，
 * 不直接包含任何 HAL 头文件，实现跨平台可移植。
 *
 * 使用示例：
 * @code
 *   dht11_hal_delay_init();          // main() 中初始化一次
 *
 *   DHT11_Data_t data;
 *   DHT11_Status_t ret = dht11_read(&data);
 *   if (ret == DHT11_OK) {
 *       printf("Temp: %d.%d C, Humi: %d.%d %%\n",
 *              data.temp_int, data.temp_dec,
 *              data.humi_int, data.humi_dec);
 *   }
 * @endcode
 */

#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  公共类型定义                                                        */
/* ------------------------------------------------------------------ */

/** 驱动返回状态码 */
typedef enum {
    DHT11_OK         = 0,   /**< 读取成功 */
    DHT11_ERR_NORESP = 1,   /**< 传感器无响应（未检测到应答信号） */
    DHT11_ERR_CRC    = 2,   /**< 校验和错误（数据损坏） */
    DHT11_ERR_TIMEOUT= 3,   /**< 等待超时 */
} DHT11_Status_t;

/** 传感器读数 */
typedef struct {
    uint8_t humi_int;   /**< 湿度整数部分，单位 % */
    uint8_t humi_dec;   /**< 湿度小数部分（DHT11始终为0，预留） */
    uint8_t temp_int;   /**< 温度整数部分，单位 °C */
    uint8_t temp_dec;   /**< 温度小数部分（DHT11始终为0，预留） */
} DHT11_Data_t;

/* ------------------------------------------------------------------ */
/*  公共接口                                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief  从DHT11读取一次温湿度数据（阻塞，约 5ms）
 * @param  out  指向用于存放读数的结构体
 * @retval DHT11_Status_t  见状态码定义
 *
 * @note   两次调用之间至少间隔 1 秒（DHT11采样周期限制）。
 * @note   调用前需确保 dht11_hal_delay_init() 已执行。
 */
DHT11_Status_t dht11_read(DHT11_Data_t *out);

#endif /* DHT11_H */
