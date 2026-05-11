/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "modbus.h"
#include "dht11_hal.h"
#include "dht11.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern uint16_t input_reg[REG_INPUT_NUM];
extern uint8_t  coil_reg[COIL_NUM];
extern uint8_t discrete_input[DISCRETE_NUM];




/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	//初始化modbus
	modbus_init(&huart1,&hdma_usart1_rx);
	// 先给寄存器手动赋固定值，验证读鿚�?
	hold_reg[0] = 1234;
	hold_reg[1] = 5678;
	input_reg[0]   = 9999;
	
		
	uint8_t test_frame[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01};
	//uint8_t test_frame[] = {0x01,0x03,0x00, 0x00,0x00,0x02};
	uint16_t crc = modbus_crc16(test_frame, 6);
	// 调试：打印实�? CRC �?
char debug_buf[32];
sprintf(debug_buf, "crc=0x%04X\r\n", crc);
HAL_UART_Transmit(&huart1, (uint8_t*)debug_buf, strlen(debug_buf), HAL_MAX_DELAY);
	// 正确结果：crc == 0x840A
		// 低字�? 0x0A，高字节 0x84
		if (crc == 0x0A84) {
				// CRC正确，亮�?
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
			//HAL_UART_Transmit(&huart1,"crc=0x840A",sizeof("crc=0x840A"),HAL_MAX_DELAY);
		} else {
				// CRC错误，快速闪烁报�?
				while(1) {
						HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
						HAL_Delay(100);
				}
		}
		
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//		//DWT CYCCNT测试代码
//		dht11_hal_delay_init();//初始�?
//		uint32_t start=DWT->CYCCNT;
//		dht11_hal_delay_us(20);
//		uint32_t delay_result=DWT->CYCCNT-start;
//		char ch[20];
//		sprintf(ch,"take %u ticks %u us",delay_result,delay_result/(SystemCoreClock/1000000));
//		HAL_UART_Transmit(&huart1,(uint8_t*)ch,strlen(ch),1000);
		//dht11_hal_delay_init();
		dht11_hal_delay_init();
//		DHT11_Data_t DHT11;
//		
//		dht11_read(&DHT11);
//		uint8_t res[2];
//		res[0]=DHT11.humi_int;
//		res[1]=DHT11.temp_int;
//		char ch[20];
//		sprintf(ch,"temp:%d,humi:%d",res[1],res[0]);
//		HAL_UART_Transmit(&huart1,(uint8_t*)ch,strlen(ch),1000);
		
		uint32_t last_time=0;
		DHT11_Data_t DHT11;
  while (1)
  {
			modbus_poll();
		if(HAL_GetTick()-last_time>2000)
		{
			//2sdht11读一次数�?
			if(dht11_read(&DHT11)==DHT11_OK)
			{
				//读取到数据，写入寄存�?
				hold_reg[0]=(DHT11.humi_int<<8)|DHT11.humi_dec;
				hold_reg[1]=(DHT11.	temp_int<<8)|DHT11.temp_dec;
			}
			last_time = HAL_GetTick();
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
