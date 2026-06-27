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
#include "string.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef void (*CommandHandler_t)(uint32_t value);

typedef struct
{
    const char *name;
    CommandHandler_t handler;
} CommandTable_t;

typedef enum
{
    MOTOR_IDLE = 0,
    MOTOR_START,
    MOTOR_RUN,
    MOTOR_STOP,
    MOTOR_RAMP,
    MOTOR_DIR_CHANGE,
		MOTOR_AUTO

}MotorState_t;
typedef enum
{
    MOTOR_EVENT_NONE = 0,

    MOTOR_EVENT_START,
    MOTOR_EVENT_STOP,

    MOTOR_EVENT_FORWARD,
    MOTOR_EVENT_REVERSE,

    MOTOR_EVENT_SET_FREQ,
    MOTOR_EVENT_SET_DUTY,

    MOTOR_EVENT_AUTO_ON,
    MOTOR_EVENT_AUTO_OFF,

    MOTOR_EVENT_MARK

}MotorEvent_t;

typedef struct
{
    uint32_t currentFreq;
    uint32_t targetFreq;
    uint32_t requestedFreq;

    uint8_t currentDir;
    uint8_t targetDir;
    uint8_t nextDir;

    uint8_t duty;

    uint8_t enable;
	
	  uint8_t motorDiarection;

    MotorState_t state;

    MotorEvent_t event;

    uint32_t eventValue;

}Motor_t;

Motor_t Motor;
typedef enum
{
    PARSER_OK = 0,
    PARSER_ERROR,
    PARSER_UNKNOWN_CMD,
    PARSER_INVALID_VALUE
} Parser_Status_t;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FORWD_COMM	"STFWD"
#define REV_COMM		"STBWD"
#define FREQ_COMM		"STSPD"
#define DUTY_COMM		"dutyy"
#define START_COMM	"STSTR"
#define STOP_COMM		"STSTP"
#define MARK_COMM		"STMAR"
#define AUTO_COMM		"STATO"
#define SP_AUTO_COMM		"STQCC"
#define SP_AUTO_COMM1		"STMAN"
#define COMMAND_COUNT \
    (sizeof(CommandTable) / sizeof(CommandTable[0]))
/* how to add NEW COMMANED */
// in Parse_Command add in last if condition 
//else if(String_Compare(command, FORWD_COMM))		<----------	FORWD_COMM = you'r commnad 
//    {
//        if(value)																<----------  value     = after '$' you'r number
//        {
//						Motor_Stop();
//            Motor_Forward();
//            Motor_Start();
//        }
//       
//    }
		
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static Parser_Status_t status;
		// confige for pwm channle 
uint16_t pwm_pre = 72 -1;
uint16_t pwm_freq = 100 -1;
uint16_t pwm_duty = 50;
		// for uart data receive 
uint8_t rxByte;
char rxBuffer1[50];
char rxBuffer[50];
uint8_t rxIndex = 0;
uint8_t commandReady = 0;
		// FLAG DOR ALL COMMANED
uint8_t  AutoFlag = 0;
uint8_t	relay_onoff = 0;
uint32_t relay_tick = 0;
		// FLAG FOR COMMANED FUCTION
uint8_t  Mark_Relay = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
void Cmd_Frequency(uint32_t value);
void Cmd_Duty(uint32_t value);
void Cmd_Forward(uint32_t value);
void Cmd_Reverse(uint32_t value);
void Cmd_Start(uint32_t value);
void Cmd_Stop(uint32_t value);
void Cmd_Auto(uint32_t value);
void Cmd_Mark(uint32_t value);
void Cmd_stop_Auto(uint32_t value);
void Cmd_stop_Auto1(uint32_t value);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static const CommandTable_t CommandTable[]=
{
    {FREQ_COMM,Cmd_Frequency},
    {DUTY_COMM,Cmd_Duty},
    {FORWD_COMM,Cmd_Forward},
    {REV_COMM,Cmd_Reverse},
    {START_COMM,Cmd_Start},
    {STOP_COMM,Cmd_Stop},
    {AUTO_COMM,Cmd_Auto},
    {MARK_COMM,Cmd_Mark},
		{SP_AUTO_COMM,Cmd_stop_Auto},
		{SP_AUTO_COMM1,Cmd_stop_Auto1},
};

Motor_t Motor =
{
    .currentFreq = 0,
    .targetFreq = 10000,
    .requestedFreq = 10000,

    .currentDir = 0,
    .targetDir = 0,
    .nextDir = 0,

    .duty = 50,

    .enable = 0,

    .state = MOTOR_IDLE
};

void Debug_Print(char *msg)
{
    HAL_UART_Transmit(&huart2,
                      (uint8_t*)msg,
                      strlen(msg),
                      100);
}

void Motor_CalculatePWM(uint32_t freq, uint32_t *psc, uint32_t *arr)
{
    uint32_t timerClock = 72000000;

    *psc = 0;

    while(1)
    {
        *arr = (timerClock / ((*psc + 1) * freq)) - 1;

        if(*arr <= 65535)
        {
            break;
        }

        (*psc)++;

        if(*psc > 65535)
        {
            *psc = 0;
            *arr = 0;
            break;
        }
    }
}

void Motor_UpdatePWM(void)
{
    uint32_t psc;
    uint32_t arr;
    uint32_t ccr;

    Motor_CalculatePWM(Motor.currentFreq, &psc, &arr);

    TIM1->PSC  = psc;
    TIM1->ARR  = arr;

    ccr = ((arr + 1) * Motor.duty) / 100;

    TIM1->CCR1 = ccr;

    TIM1->CNT  = 0;

    TIM1->EGR  = TIM_EGR_UG;
}

void Motor_Stop(void)
{
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);

    HAL_GPIO_WritePin(PWM_EN_GPIO_Port,
                      PWM_EN_Pin,
                      GPIO_PIN_RESET);

    Motor.enable = 0;

    Debug_Print("MOTOR OFF\r\n");
}

void Motor_SetFrequency(uint32_t freq)
{
    if(freq == 0)
    {
        TIM1->CCR1 = 0;
        Motor.currentFreq = 0;
        return;
    }

    Motor.currentFreq = freq;

    Motor_UpdatePWM();
}

void Motor_SetDuty(uint8_t duty)
{
    if(duty > 100)
    {
        Debug_Print("ERR: DUTY\r\n");
        return;
    }

    Motor.duty = duty;

    Motor_UpdatePWM();

    Debug_Print("OK: DUTY SET\r\n");
}

void Motor_Start(void)
{
    HAL_GPIO_WritePin(PWM_EN_GPIO_Port,
                      PWM_EN_Pin,
                      GPIO_PIN_SET);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

    Motor.enable = 1;

    Debug_Print("MOTOR ON\r\n");
}



void Motor_Forward(void)
{
    HAL_GPIO_WritePin(PWM_DIR_GPIO_Port,
                      PWM_DIR_Pin,
                      GPIO_PIN_SET);

    Debug_Print("DIR FWD\r\n");
}

void Motor_Reverse(void)
{
    HAL_GPIO_WritePin(PWM_DIR_GPIO_Port,
                      PWM_DIR_Pin,
                      GPIO_PIN_RESET);

    Debug_Print("DIR REV\r\n");
}

uint8_t String_Compare(const char *str1, const char *str2)
{
    while(*str1 && *str2)
    {
        if(*str1 != *str2)
            return 0;

        str1++;
        str2++;
    }

    return (*str1 == *str2);
}

void String_Copy(char *str1, char *str2)
{
    while(*str1)
    {
        *str2 = *str1;

        str1++;
        str2++;
    }
		*str2 = '\0';

}

uint8_t String_To_Int(char *str, uint32_t *value)
{
    *value = 0;

    if(*str == '\0')
        return 0;

    while(*str)
    {
        if(*str < '0' || *str > '9')
            return 0;

        *value = (*value * 10) + (*str - '0');

        str++;
    }

    return 1;
}

//===============================================

void Cmd_Frequency(uint32_t value)
{
    Motor.event = MOTOR_EVENT_SET_FREQ;
    Motor.eventValue = value;
}
void Cmd_Duty(uint32_t value)
{
    Motor.event = MOTOR_EVENT_SET_DUTY;
    Motor.eventValue = value;
}
void Cmd_Forward(uint32_t value)
{
    if(value)
        return;

    
		Motor.event = MOTOR_EVENT_FORWARD;
}
void Cmd_Reverse(uint32_t value)
{
    if(value)
        return;

    Motor.event = MOTOR_EVENT_REVERSE;
}
void Cmd_Start(uint32_t value)
{
    if(value)
        return;

    Motor.event = MOTOR_EVENT_START;
}
void Cmd_Stop(uint32_t value)
{
    if(value)
        return;

    Motor.event = MOTOR_EVENT_STOP;
}
void Cmd_Auto(uint32_t value)
{
    if(!value)
        {
						AutoFlag = 1;
					Motor.event = MOTOR_EVENT_AUTO_ON;
        }
}
void Cmd_Mark(uint32_t value)
{
   if(!value)
        {
						Mark_Relay = 1;
					Motor.event = MOTOR_EVENT_MARK;
        }
}
void Cmd_stop_Auto(uint32_t value)
{
    if(!value)
        {
						AutoFlag = 0;
					Motor.event = MOTOR_EVENT_AUTO_OFF;
        }
}
void Cmd_stop_Auto1(uint32_t value)
{
   if(!value)
        {
						Mark_Relay = 0;
					Motor.event = MOTOR_EVENT_AUTO_OFF;
        }
}
static void Motor_ProcessEvent(void)
{
    switch(Motor.event)
    {
        case MOTOR_EVENT_START:
					
						if(Motor.state == MOTOR_AUTO && (Motor.currentDir != Motor.nextDir)) Motor.state = MOTOR_RAMP;

            else Motor.state = MOTOR_START;

            break;

        case MOTOR_EVENT_STOP:

            if(Motor.state != MOTOR_AUTO)
						{
							Motor.state = MOTOR_STOP;
							
						}

            break;

        case MOTOR_EVENT_FORWARD:

            if(AutoFlag)
            {
                Motor.nextDir = 1;
                Motor.state = MOTOR_AUTO;
            }
            else
            {
                Motor.targetDir = 1;
                Motor.state = MOTOR_DIR_CHANGE;
            }

            break;

        case MOTOR_EVENT_REVERSE:

            if(AutoFlag)
            {
                Motor.nextDir = 0;
                Motor.state = MOTOR_AUTO;
            }
            else
            {
                Motor.targetDir = 0;
                Motor.state = MOTOR_DIR_CHANGE;
            }

            break;

        case MOTOR_EVENT_SET_FREQ:

            Motor.requestedFreq = Motor.eventValue;

            //if(Motor.state != MOTOR_STOP)
                Motor.targetFreq = Motor.eventValue;

            break;

        case MOTOR_EVENT_SET_DUTY:

            Motor_SetDuty((uint8_t)Motor.eventValue);

            break;
				
				case MOTOR_EVENT_MARK:

            relay_onoff = 1;
						relay_tick = HAL_GetTick();
						HAL_GPIO_WritePin(GPIOB,RELAY2_Pin|MARKING_Pin, GPIO_PIN_SET);

            break;


        default:
            break;
    }

    Motor.event = MOTOR_EVENT_NONE;
}


Parser_Status_t Parse_Command(char *rx)
{
    char command[10] = {0};
    char valueStr[10] = {0};

    uint8_t i = 0;
    uint8_t j = 0;
    uint32_t value;


    /* Check start */
    if(rx[0] != '(')
        return PARSER_ERROR;


    /* Extract command */
    i = 1;

    while(rx[i] &&
          rx[i] != '$' &&
          rx[i] != '&' &&
          j < sizeof(command)-1)
    {
        command[j++] = rx[i++];
    }

    command[j] = '\0';


    /* Check separator */
    if(rx[i] != '$' && rx[i] != '&')
        return PARSER_ERROR;


    i++;
    j = 0;


    /* Extract value */
    while(rx[i] &&
          rx[i] != ')' &&
          j < sizeof(valueStr)-1)
    {
        valueStr[j++] = rx[i++];
    }

    valueStr[j] = '\0';


    /* Check end */
    if(rx[i] != ')')
        return PARSER_ERROR;


    /* Convert value */
    if(String_To_Int(valueStr, &value) == 0)
        return PARSER_INVALID_VALUE;


    /* Execute command */

   for(i=0;i<COMMAND_COUNT;i++)
	 {
			 if(String_Compare(command,CommandTable[i].name))
			 {
						CommandTable[i].handler(value);
						return PARSER_OK;
				}
		}


    return PARSER_OK;
}

void Motor_RampFrequency(void)
{
    static uint32_t lastTick = 0;

    if((HAL_GetTick() - lastTick) < 10)
        return;

    lastTick = HAL_GetTick();

    if(Motor.targetFreq > Motor.currentFreq)
    {
        Motor.currentFreq += 100;

        if((Motor.targetFreq - Motor.currentFreq) < 100)
            Motor.currentFreq = Motor.targetFreq;
				Motor_SetFrequency(Motor.currentFreq);
    }
    else if(Motor.targetFreq < Motor.currentFreq)
    {
        if((Motor.currentFreq - Motor.targetFreq) > 100)
            Motor.currentFreq -= 100;
        else
            Motor.currentFreq = Motor.targetFreq;
				Motor_SetFrequency(Motor.currentFreq);
    }

}
//void smuth_setFreq(void)
//{
//    static uint32_t lastTick = 0;
//    uint32_t step;

//    if((HAL_GetTick() - lastTick) < 10)
//        return;

//    lastTick = HAL_GetTick();

//    step = pwmFrequency / 20;   // 5%

//    if(step < 1)
//        step = 1;

//    if(step > 500)
//        step = 500;             // safety limit

//    if(targetFreq > pwmFrequency)
//    {
//        pwmFrequency += step;

//        if(pwmFrequency > targetFreq)
//            pwmFrequency = targetFreq;
//    }
//    else if(targetFreq < pwmFrequency)
//    {
//        if((pwmFrequency - targetFreq) > step)
//            pwmFrequency -= step;
//        else
//            pwmFrequency = targetFreq;
//    }

//    Motor_SetFrequency(pwmFrequency);
//}

void Motor_Task(void)
{
    Motor_ProcessEvent();

    switch(Motor.state)
    {
        case MOTOR_IDLE:
            break;

        case MOTOR_START:
						

            Motor.targetFreq = Motor.requestedFreq;

            Motor_Start();

            Motor.state = MOTOR_RUN;

            break;

        case MOTOR_RUN:

            Motor_RampFrequency();
						
						if(Motor.currentFreq == Motor.targetFreq) Motor.state = MOTOR_IDLE;

            break;

        case MOTOR_STOP:

            Motor.targetFreq = 0;

            Motor_RampFrequency();

            if(Motor.currentFreq == 0)
            {
                Motor_Stop();

                Motor.state = MOTOR_IDLE;
            }

            break;

        case MOTOR_DIR_CHANGE:

            Motor.targetFreq = 0;

            Motor_RampFrequency();

            if(Motor.currentFreq == 0)
            {
                Motor.currentDir = Motor.targetDir;

                if(Motor.currentDir)
								{
                    Motor_Forward();
								}
                else
								{
                    Motor_Reverse();
								}

                if(AutoFlag)
                {
                    Motor.state = MOTOR_START;
                }
                else
                {
                    Motor_Stop();

                    Motor.state = MOTOR_IDLE;
                }
            }

            break;

        case MOTOR_RAMP:

            Motor.targetDir = Motor.nextDir;

            Motor.state = MOTOR_DIR_CHANGE;

            break;

        default:

           // Motor.state = MOTOR_IDLE;

            break;
    }
}

void Relay_OFF(void)
{
	if((HAL_GetTick() - relay_tick) > 1000)
	{
		relay_onoff = 0;
		HAL_GPIO_WritePin(GPIOB,RELAY2_Pin|MARKING_Pin, GPIO_PIN_RESET);
	}
}

void Comand_Receive(void)
{
			commandReady = 0;
			String_Copy(rxBuffer1,rxBuffer);
			
			if(Motor.state == MOTOR_IDLE || Motor.state == MOTOR_AUTO) status = Parse_Command(rxBuffer);
			if(!commandReady)
			{
				if(!String_Compare(rxBuffer,rxBuffer1)) commandReady = 1;
			}
}
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
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	
	HAL_UART_Transmit(&huart2,(uint8_t*)"UART OK\r\n",9,100);
	HAL_UART_Receive_IT(&huart2, &rxByte, 1);
	HAL_GPIO_WritePin(PWM_DIR_GPIO_Port,PWM_DIR_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PWM_EN_GPIO_Port, PWM_EN_Pin,GPIO_PIN_SET);
	Motor.state = MOTOR_STOP;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
		
		Motor_Task();

		if(commandReady)
			Comand_Receive();
		
		if(relay_onoff)
			Relay_OFF();
		//HAL_Delay(5);

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
  htim1.Init.Prescaler = pwm_pre;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = pwm_freq;
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
  sConfigOC.Pulse = pwm_duty;
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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, PWM_DIR_Pin|PWM_EN_Pin|RELAY2_Pin|MARKING_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PWM_DIR_Pin PWM_EN_Pin */
  GPIO_InitStruct.Pin = PWM_DIR_Pin|PWM_EN_Pin|RELAY2_Pin|MARKING_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        if(rxByte == ')')
        {
            rxBuffer1[rxIndex] = rxByte;
						rxBuffer1[++rxIndex] = '\0';
            rxIndex = 0;
            commandReady = 1;
        }
        else
        {
            if(rxIndex < sizeof(rxBuffer1)-1)
						{
								rxBuffer1[rxIndex++] = rxByte;
						}
						else
						{
								rxIndex = 0;
								//Debug_Print("ERR: RX Overflow\r\n");
						}
        }

        // Restart UART interrupt
        HAL_UART_Receive_IT(&huart2, &rxByte, 1);
    }
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
