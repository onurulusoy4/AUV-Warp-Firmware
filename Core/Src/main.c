/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "adc.h"
#include "crc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "warp_protocol.h" //generated using warp_protocol.proto in warp-firmware\proto repo // ?? 

enum ESCCurrentADCCommand{
  STOP = 0, // stop sending esc current data
  START_CONTINIOUS = 1, // start sending esc current data
  START_ONETIME = 2 // send esc current data
};

uint32_t adcbuffer[10];
float currentbuffer[8];
//char currentDataToSend[75]; // in Amperes, will be parsed with ' ' 
extern uint16_t status;
float constant1 = 3.3 / 4096.0 / 100 * 1000 / 100; // mV/mOhm = Amperes
char encodedADCInfo[75]; //??

struct ChannelData {
  uint16_t ch0; /// us pulse (t_on time, t_off time -> dynamic) f_hz = 50, t_off = 20000 - t_on # ARR -> 20000,
  uint16_t ch1;
  uint16_t ch2;
  uint16_t ch3;
  uint16_t ch4;
  uint16_t ch5;
  uint16_t ch6;
  uint16_t ch7;
};

struct Power {
  float voltage;
  float current;
};

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void DriveMotors(const struct ChannelData command);
void ESCCurrent_Request(enum ESCCurrentADCCommand command);

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

  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_CRC_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
	
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_2);

  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);

  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_4);
	
	const struct ChannelData PWM_ChannelData;// = {1100, 1800, 1300, 1400, 1500, 1400, 1800, 1500};
	
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
		ESCCurrent_Request(status);
		
		DriveMotors(PWM_ChannelData);
		
		HAL_Delay(500);
		

		
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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

void DriveMotors(const struct ChannelData command) {

  uint16_t pulse;

  uint16_t ch0_command = command.ch0;
  pulse = (int) round(1.68 * ch0_command);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);

  uint16_t ch1_command = command.ch1;
  pulse = (int) round(1.68 * ch1_command);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);

  uint16_t ch2_command = command.ch2;
  pulse = (int) round(1.68 * ch2_command);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pulse);

  uint16_t ch3_command = command.ch3;
  pulse = (int) round(1.68 * ch3_command);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);

  uint16_t ch4_command = command.ch4;
  pulse = (int) round(1.68 * ch4_command);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);

  uint16_t ch5_command = command.ch5;
  pulse = (int) round(1.68 * ch5_command);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, pulse);

  uint16_t ch6_command = command.ch6;
  pulse = (int) round(1.68 * ch6_command);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, pulse);

  uint16_t ch7_command = command.ch7;
  pulse = (int) round(1.68 * ch7_command);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, pulse);


}


void ESCCurrent_Request(enum ESCCurrentADCCommand command) {
  if (command == START_ONETIME) {
		
    HAL_ADC_Start_DMA(&hadc1, adcbuffer, 10);
		
		for (uint8_t i=0; i<8; i++)
		{
			currentbuffer[i] = adcbuffer[i] * constant1; 
		}
		
		//6 digits
		//sprintf(currentDataToSend, "%2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f %2.6f\r\n", currentbuffer[0], currentbuffer[1], currentbuffer[2], currentbuffer[3], currentbuffer[4], currentbuffer[5], currentbuffer[6], currentbuffer[7]);
		
		
		
		struct Power battery;
		battery.current = adcbuffer[8] * constant1;
		battery.voltage = adcbuffer[9]* constant1;
		
		encodedADCInfo = warp_protocol.ADC_data.encode(currentbuffer[0], currentbuffer[1], currentbuffer[2], currentbuffer[3], currentbuffer[4], currentbuffer[5], currentbuffer[6], currentbuffer[7], battery.current, battery.voltage); //??
	  
		if (HAL_UART_Transmit(&huart1, (uint8_t*) encodedADCInfo, sizeof(encodedADCInfo), 1000)!= HAL_OK) //??
			Error_Handler();

		
  }
}

void CDC_FS_Receive_CpltCallback(uint8_t *Buf, uint32_t *Len) {
  // Receive Code Here.
}
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

