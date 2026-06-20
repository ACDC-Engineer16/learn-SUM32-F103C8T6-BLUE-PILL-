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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define PWM_MAX_FREQ 1000000 			//1MHz, setect by PCS = 8-1;, SYS = 8MHz
const char *msg1 = "plaese send like : $xxx:x:xxx$ + Enter\n";
const char *msg2 = "1st = freq\n2nd = 1(if KHz)\n3rd = duty\n";
const char *msg3 = "NOTE : freq = 0, duty > 100 not consider\n";
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
	uint8_t RxData;
	uint8_t pwm_count = 0;
	uint8_t pwm_flag = 0;
	uint32_t DUTY = 0;
	uint8_t u2Data[12];
	/////////////////////////////////////////////
	// timer-1 clock freq is 1MHz. and we have ARR and CAM1 both register is 16-bit resister
	// so , min freq = 1MHz/0xFFFF; is 15Hz ;
	// and also max freq = 50KHz with 20 step of duty controle
	typedef struct{
		uint8_t In_KHz;
		uint8_t U_Duty;
		uint16_t R_Duty;
		uint16_t U_Freq;
		uint32_t R_Freq;
	}PWM_struct;
PWM_struct P_PWM;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
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
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
	P_PWM.In_KHz = 1;
	P_PWM.R_Duty = 0;
	P_PWM.R_Freq = 100;
	P_PWM.U_Duty = 50;
	P_PWM.U_Freq = 10;
	DUTY = 0;
	TIM1->CCR1 = DUTY;
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
	
	HAL_UART_Receive_IT(&huart2, &RxData, 1);
	HAL_UART_Transmit(&huart2,(uint8_t *)msg1,39, 100);
	HAL_UART_Transmit(&huart2,(uint8_t *)msg2,38, 100);
	HAL_UART_Transmit(&huart2,(uint8_t *)msg3,40, 100);
	HAL_UART_Transmit(&huart2,(uint8_t *)"\nEnter value send\n",18, 100);
	//pwm_flag = 2;

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if(pwm_flag)
		{
			if(pwm_flag == 2)
			{
				HAL_UART_Transmit(&huart2,(uint8_t *)"last data not consider\n",23, 100);
				HAL_UART_Transmit(&huart2,(uint8_t *)msg1,39, 100);
				HAL_UART_Transmit(&huart2,(uint8_t *)msg2,38, 100);
				HAL_UART_Transmit(&huart2,(uint8_t *)msg3,40, 100);
				pwm_flag = 0;
				HAL_UART_Transmit(&huart2,(uint8_t *)"\nnext DUTY send \n",17, 100);
				continue;
			}
			if(!(P_PWM.U_Freq))
			{
				HAL_UART_Transmit(&huart2,(uint8_t *)"zero(0) freq not config\n",24, 100);
				pwm_flag = 2;// clearn main if  condition 
				continue;
			}
			if(P_PWM.U_Duty > 100)
			{
				HAL_UART_Transmit(&huart2,(uint8_t *)"duty more then 100 not config\n",30, 100);
				pwm_flag = 2;// clearn main if  condition 
				continue;
			}
			
			P_PWM.R_Freq = P_PWM.U_Freq;
			if(P_PWM.In_KHz)
				P_PWM.R_Freq *= 1000; 				// convert to KHz
			P_PWM.R_Freq = PWM_MAX_FREQ / P_PWM.R_Freq;		// setect ACC value 
			
			P_PWM.R_Duty = (P_PWM.U_Duty * P_PWM.R_Freq) / 100;
			
			TIM1->ARR = P_PWM.R_Freq - 1;
			TIM1->CCR1 = P_PWM.R_Duty;
			
			pwm_flag = 0;// clearn main if  condition 
			HAL_UART_Transmit(&huart2,(uint8_t *)"\nnext DUTY send \n",17, 100);
			
		}
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 8-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LEDOUT_GPIO_Port, LEDOUT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LEDOUT_Pin */
  GPIO_InitStruct.Pin = LEDOUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LEDOUT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	u2Data[pwm_count++] = RxData;
	if(pwm_count == 12)
	{
		if(u2Data[11] != '\n' || u2Data[0] != '$' || u2Data[10] != '$' || u2Data[4] != ':' || u2Data[6] != ':')
		{
			pwm_flag = 2; // error print
		}
		else if (u2Data[11] == '\n' && u2Data[0] == '$' && u2Data[10] == '$' && u2Data[4] == ':' && u2Data[6] == ':'){
					P_PWM.In_KHz = u2Data[5] - '0';
					P_PWM.U_Duty = ((u2Data[7]-'0')*100) +((u2Data[8]-'0')*10) + (u2Data[9] - '0');
					P_PWM.U_Freq = ((u2Data[1]-'0')*100) +((u2Data[2]-'0')*10) + (u2Data[3] - '0');
					pwm_flag = 1;
		HAL_UART_Transmit(&huart2,(uint8_t *)u2Data,11, 100);
		}
		pwm_count = 0;
		RxData = 0;
	}
	if(RxData == '\n')
	{
		pwm_count = 0;
		pwm_flag = 2;
	}
	HAL_UART_Receive_IT(&huart2, &RxData, 1);
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
#ifdef USE_FULL_ASSERT
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
