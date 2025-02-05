/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "eeprom.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define MESSAGE_SIZE 64
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

TIM_MasterConfigTypeDef sMasterConfig ={ 0 };
TIM_OC_InitTypeDef sConfigOC ={ 0 };
ADC_ChannelConfTypeDef sConfig ={ 0 };
uint8_t dutyCycle = 0;    // Global variable for setting duty cycle (0 to 100)
uint32_t dutyToPulse;
char rcvbyte;
char msg;  // Buffer for UART messages
uint16_t duty_to_pulse;
uint16_t adc_val = 0;
char output_voltage[20];
char temp_output_voltage[20];
char test_result;
int eeprom_page_count = 32;  // Example loop count
char test_instruction[100] ={ 0 };
float voltage;
float temp_voltage;
uint16_t temp_adc_val = 0;
volatile uint32_t toggle = 0;
static int count_torch;
/* USER CODE END PV */
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_TIM4_TEST2_Init(void);
void MX_TIM4_TEST3_Init(void);
void MX_TIM4_TEST5_Init(void);
void MX_TIM4_TEST6_Init(void);
void MX_USART1_UART_Init(void);
void MX_TIM3_Init(void);
void adcChannel_config(uint8_t channel);
/* USER CODE BEGIN PFP */
int DutytoPulse(uint8_t duty_percentage, uint16_t init_period);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void adcChannel_config(uint8_t channel)
{
    sConfig.Channel = channel;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	static int count = 0;

	if (htim == &htim3) {
		if (count == 0 && count_torch != 0)
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, 1);

		else if (count == count_torch)
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, 0);

		count++;

		if (count == 4)
			count = 0;
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    printf("Received:\n");
    if (huart->Instance == USART1)
    {
        printf("Received: %c\n", rcvbyte);
        msg = rcvbyte;
// Restart UART reception
        HAL_UART_Receive_IT(&huart1, (uint8_t*) &rcvbyte, 1);
    }
}

//void SysTick_Handler(void)
//{
//    toggle = 1; // Set flag to toggle GPIO
//}

int _write(int file, char *ptr, int len)
{
    for (int i = 0; i < len; i++)
    {
        ITM_SendChar(*ptr++);
    }
    return len;
}

int DutytoPulse(uint8_t duty_percentage, uint16_t init_period)
{
    return (init_period * duty_percentage) / 100;
}

void CLIPrint(char *fmt, ...)
{
    char buf[1000];
    va_list vl;

    va_start(vl, fmt);
    vsnprintf(buf, sizeof(buf), fmt, vl);
    va_end(vl);

    HAL_UART_Transmit(&huart1, (uint8_t *)buf, strlen(buf), 100);
}

void SendTestResults(uint8_t *buffer )
{
    for (uint8_t i = 0; i < MESSAGE_SIZE; i++)
    {
        if (buffer[i] == 1)
        {
            // Send pass message
            CLIPrint("Passed to write page %d\r\n",i);
        }
        else
        {
            // Send fail message
            CLIPrint("Failed to write page %d\r\n",i);
        }
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
    uint8_t test_repeat = 0;
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
    MX_ADC1_Init();
    MX_USART1_UART_Init();
    HAL_UART_Receive_IT(&huart1, (uint8_t*) &rcvbyte, 1);
    // Configure SysTick to generate an interrupt every 50 µs
//       HAL_SYSTICK_Config(SystemCoreClock / 20000); // 20 kHz frequency (50 µs)
    /* USER CODE BEGIN 2 */

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        if ('1' == msg)
        {
                uint8_t eeprom_test_results[MAX_BUFFER_SIZE] = {0}; // Buffer to store test results
                uint8_t test_page = 0x01;  // Example starting page
                int offset = 0;
                msg=0;
                HAL_UART_DeInit(&huart1);
//                MX_I2C1_Init();
                // Write 0x00 to 32 pages and check
                for (int i = 0; i < eeprom_page_count; i++) {
                    // Perform EEPROM write and read operations here
                    // EEPROM_Write(test_page, buf, length);
                    // EEPROM_Read(test_page, buf, length);

                    // Check result and store in buffer
                    // Assuming EEPROM_Write and EEPROM_Read are correctly implemented
                    if (1) {
                        eeprom_test_results[offset] = 1; // Fail
                    } else {
                        eeprom_test_results[offset] = 0; // Pass
                    }
                    offset++;
                    test_page++;
                    if (offset >= MAX_BUFFER_SIZE) {
                        break;  // Exit if buffer is full
                    }
                }

                // Write 0xFF to 32 pages and check
                for (int i = 0; i < eeprom_page_count; i++) {
                    // Perform EEPROM write and read operations here
                    // EEPROM_Write(test_page, buf, length);
                    // EEPROM_Read(test_page, buf, length);

                    // Check result and store in buffer
                    // Assuming EEPROM_Write and EEPROM_Read are correctly implemented
                    if (0) {
                        eeprom_test_results[offset] = 1; // Fail
                    } else {
                        eeprom_test_results[offset] = 0; // Pass
                    }
                    offset++;
                    test_page++;
                    if (offset >= MAX_BUFFER_SIZE) {
                        break;  // Exit if buffer is full
                    }

			}
//                HAL_I2C_DeInit(&hi2c1);
                MX_USART1_UART_Init();
                // Send the test results via UART
                SendTestResults(eeprom_test_results);
                test_result = 'P';
                HAL_UART_Transmit(&huart1, (uint8_t*) &test_result, 1,
                HAL_MAX_DELAY);
        }
        if ('2' == msg)
        {
            printf("second  test completed\n");
            MX_TIM4_TEST2_Init();
// Start PWM
            HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

// Delay for 2 seconds
            HAL_Delay(2000);

// Stop PWM
            HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);

// Reset msg to prevent re-triggering
            msg = 0;
            test_result = '-';
            HAL_UART_Transmit(&huart1, (uint8_t*) &test_result, 1,
            HAL_MAX_DELAY);
        }
        if ('3' == msg)
        {
            test_repeat = 1;
            msg = 0;
//		                        sprintf(test_instruction, "Turn ON Test jig SW1 and Press 3\r\n");

            MX_TIM4_TEST3_Init();

// Restart the PWM with new settings
            HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

            HAL_UART_Transmit(&huart1, (uint8_t*) test_instruction, strlen(test_instruction), HAL_MAX_DELAY);

            while (test_repeat <= 2)  // Terminate when test_repeat becomes 2
            {
                if ('3' == msg && 1 == test_repeat)
                {
                    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_2);

                    adcChannel_config(ADC_TEST_3);
                    HAL_ADC_Start(&hadc1);

                    HAL_ADC_PollForConversion(&hadc1, 20);
                    adc_val = HAL_ADC_GetValue(&hadc1);
                    float voltage = (adc_val * 3.3f) / 4095.0f; // Calculate voltage
                    printf("ADC: %.2f\n", voltage);
                    sprintf(output_voltage, "ADC: %.2f V\r\n", voltage);
                    HAL_UART_Transmit(&huart1, (uint8_t*) output_voltage, strlen(output_voltage), HAL_MAX_DELAY);
                    test_repeat = 2;
                    msg = 0;

//										sprintf(test_instruction, "Turn OFF Test jig SW1 and Press 3\r\n");
                    HAL_UART_Transmit(&huart1, (uint8_t*) test_instruction, strlen(test_instruction), HAL_MAX_DELAY);

                }

                else if ('3' == msg && 2 == test_repeat)
                {
                    HAL_ADC_PollForConversion(&hadc1, 20);
                    adc_val = HAL_ADC_GetValue(&hadc1);
                    voltage = (adc_val * 3.3f) / 4095.0f; // Calculate voltage
                    printf("ADC: %.2f\n", voltage);
                    sprintf(output_voltage, "ADC: %.2f V\r\n", voltage);
                    HAL_UART_Transmit(&huart1, (uint8_t*) output_voltage, strlen(output_voltage), HAL_MAX_DELAY);
                    test_repeat = 3;
                    msg = 0;
                    HAL_ADC_Stop(&hadc1);
                    printf("Exiting loop on second '3' command.\n");
                    break;  // Exit the loop
                }

            }
            test_repeat = 0;
        }

        if ('4' == msg)
        {
            msg = 0;
            test_repeat = 1;
            while (test_repeat <= 4)  // Terminate when test_repeat becomes 2
            {
                if ('4' == msg && 1 == test_repeat)
                {

//                    MX_ADC1_Init();
                    // Configure the ADC channel
                    adcChannel_config(ADC_TEST_4);
                    HAL_ADC_Start(&hadc1);
                    HAL_ADC_PollForConversion(&hadc1, 20);
                    adc_val = HAL_ADC_GetValue(&hadc1);
                    voltage = (adc_val * 3.3f) / 4095.0f; // Calculate voltage
                    printf("ADC: %.2f\n", voltage);
                    sprintf(output_voltage, "ADC: %.2f V\r\n", voltage);
                    HAL_UART_Transmit(&huart1, (uint8_t*) output_voltage, strlen(output_voltage), HAL_MAX_DELAY);
                    test_repeat = 2;
                    msg = 0;
                    HAL_ADC_Stop(&hadc1);
                }
                else if ('4' == msg && 2 == test_repeat)
                {
//                    sConfig.Channel = ADC_TEST_4;
//                    MX_ADC1_Init();
                    HAL_ADC_Start(&hadc1);
                    HAL_ADC_PollForConversion(&hadc1, 20);
                    adc_val = HAL_ADC_GetValue(&hadc1);
                    voltage = (adc_val * 3.3f) / 4095.0f; // Calculate voltage
                    printf("ADC: %.2f\n", voltage);
                    sprintf(output_voltage, "ADC: %.2f V\r\n", voltage);
                    HAL_UART_Transmit(&huart1, (uint8_t*) output_voltage, strlen(output_voltage), HAL_MAX_DELAY);
                    test_repeat = 3;
                    msg = 0;
                    HAL_ADC_Stop(&hadc1);
                }
                else if ('4' == msg && 3 == test_repeat)
                {
//                    sConfig.Channel = ADC_TEST_4;
//                    MX_ADC1_Init();
                    HAL_ADC_Start(&hadc1);
                    HAL_ADC_PollForConversion(&hadc1, 20);
                    adc_val = HAL_ADC_GetValue(&hadc1);
                    voltage = (adc_val * 3.3f) / 4095.0f; // Calculate voltage
                    printf("ADC: %.2f\n", voltage);
                    sprintf(output_voltage, "ADC: %.2f V\r\n", voltage);
                    HAL_UART_Transmit(&huart1, (uint8_t*) output_voltage, strlen(output_voltage), HAL_MAX_DELAY);
                    test_repeat = 4;
                    msg = 0;
                    HAL_ADC_Stop(&hadc1);
                }
                else if ('4' == msg && 4 == test_repeat)
                {
//                    sConfig.Channel = ADC_TEST_4;
//
//                    MX_ADC1_Init();
//                    // Configure the ADC channel
//                             if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
//                             {
//                                Error_Handler(); // Configuration error
//                             }
                    HAL_ADC_Start(&hadc1);
                    HAL_ADC_PollForConversion(&hadc1, 20);
                    adc_val = HAL_ADC_GetValue(&hadc1);
                    voltage = (adc_val * 3.3f) / 4095.0f; // Calculate voltage
                    printf("ADC: %.2f\n", voltage);
                    sprintf(output_voltage, "ADC: %.2f V\r\n", voltage);
                    HAL_UART_Transmit(&huart1, (uint8_t*) output_voltage, strlen(output_voltage), HAL_MAX_DELAY);
                    test_repeat = 5;
                    msg = 0;
                    HAL_ADC_Stop(&hadc1);
                    printf("Exiting loop on second '4' command.\n");
                    break;  // Exit the loop
                }

            }
            test_repeat = 0;
        }

        if ('5' == msg)
        {
            test_repeat = 1;
            while (1)  // Terminate when test_repeat becomes 2
            {
                if ('5' == msg && 1 == test_repeat)
                {
//                    duty_to_pulse = 398;
//                    MX_TIM4_TEST5_Init();
                	count_torch = 4;
//                    GPIOB->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9); // Clear mode and config bits for PB8
//                    GPIOB->CRH |= GPIO_CRH_MODE9_0; // Set PB8 to output mode, max speed 10 MHz
//                    GPIOB->ODR |= (1 << 9);  // Set PB8 high
                	MX_TIM3_Init();
                    test_repeat = 2;
                    msg = 0;
                }
                else if ('5' == msg && 2 == test_repeat)
                {

//                    GPIOB->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9); // Clear mode and config bits for PB8
//                    GPIOB->CRH |= GPIO_CRH_MODE9_0; // Set PB8 to output mode, max speed 10 MHz
//                    GPIOB->CRH |= GPIO_CRH_CNF9_1; // Set PB8 to alternate function push-pull (AF2 for TIM1_CH1)
                	count_torch = 3;

                    printf("duty:%d\n", (uint16_t) duty_to_pulse);

                    msg=0;
//
// Restart the PWM with new settings
//                    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
                    test_repeat = 3;
//                    msg = 0;
                }
                else if ('5' == msg && 3 == test_repeat)
                {
//                    duty_to_pulse = (uint16_t) DutytoPulse(50, 399);
//                	 duty_to_pulse = 3999;
                	count_torch = 2;
//                    printf("duty:%d\n", (uint16_t) duty_to_pulse);
//                    MX_TIM3_Init();

// Restart the PWM with new settings
//                    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
                    test_repeat = 4;
                    msg = 0;
                }
                else if ('5' == msg && 4 == test_repeat)
                {
//                    duty_to_pulse = (uint16_t) DutytoPulse(25, 399);
//                	duty_to_pulse = 1999;
//                    printf("duty:%d\n", (uint16_t) duty_to_pulse);
                	count_torch = 1;
//                    MX_TIM3_Init();

// Restart the PWM with new settings
//                    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
                    test_repeat = 5;
                    msg = 0;
                }
                else if ('5' == msg && 5 == test_repeat)
                {
// Restart the PWM with new settings
//                    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
//                	HAL_TIM_Base_Stop(&htim3);
                	count_torch = 0;
                    test_repeat = 6;
                    msg = 0;
                    test_result = '-';
                    HAL_UART_Transmit(&huart1, (uint8_t*) &test_result, 1,
                    HAL_MAX_DELAY);
                    break;
                }
            }
            test_repeat = 0;
        }

        if ('6' == msg)
        {
            test_repeat = 1;
            while (test_repeat <= 6)  // Terminate when test_repeat becomes 2
            {
                if ('6' == msg && 1 == test_repeat)
                {
                    duty_to_pulse = 398;
                    MX_TIM4_TEST6_Init();

// Configure PB8 as push-pull output
                    GPIOB->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9); // Clear mode and config bits for PB8
                    GPIOB->CRH |= GPIO_CRH_MODE9_0; // Set PB8 to output mode, max speed 10 MHz
                    GPIOB->ODR |= (1 << 9);  // Set PB6 high
// Restart the PWM with new settings
                    test_repeat = 2;
                    msg = 0;
                }
                else if ('6' == msg && 2 == test_repeat)
                {
// Configure PB8 as alternate function push-pull
                    GPIOB->CRH &= ~(GPIO_CRH_MODE9 | GPIO_CRH_CNF9); // Clear mode and config bits for PB8
                    GPIOB->CRH |= GPIO_CRH_MODE9_0; // Set PB8 to output mode, max speed 10 MHz
                    GPIOB->CRH |= GPIO_CRH_CNF9_1; // Set PB8 to alternate function push-pull (AF2 for TIM1_CH1)
                    duty_to_pulse = (uint16_t) DutytoPulse(75, 399);
                    printf("duty:%d\n", (uint16_t) duty_to_pulse);
                    MX_TIM4_TEST6_Init();

// Restart the PWM with new settings
                    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
                    test_repeat = 3;
                    msg = 0;
                }
                else if ('6' == msg && 3 == test_repeat)
                {
                    duty_to_pulse = (uint16_t) DutytoPulse(50, 399);
                    printf("duty:%d\n", (uint16_t) duty_to_pulse);
                    MX_TIM4_TEST6_Init();

// Restart the PWM with new settings
                    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
                    test_repeat = 4;
                    msg = 0;
                }
                else if ('6' == msg && 4 == test_repeat)
                {
                    duty_to_pulse = (uint16_t) DutytoPulse(25, 399);
                    printf("duty:%d\n", (uint16_t) duty_to_pulse);
                    MX_TIM4_TEST6_Init();

// Restart the PWM with new settings
                    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
                    test_repeat = 5;
                    msg = 0;
                }
                else if ('6' == msg && 5 == test_repeat)
                {
// Restart the PWM with new settings
                    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
                    test_repeat = 6;
                    msg = 0;
                    test_result = '-';
                    HAL_UART_Transmit(&huart1, (uint8_t*) &test_result, 1,
                    HAL_MAX_DELAY);
                    break;
                }
            }
            test_repeat = 0;
        }

        if ('7' == msg)
        {
            msg = 0;
            rcvbyte = 0;
            while (1)
            {
                printf("ADC: %.2f\n", temp_voltage);
                sprintf(temp_output_voltage, "ADC: %.2f V\r\n", temp_voltage);
                HAL_UART_Transmit(&huart1, (uint8_t*) temp_output_voltage, strlen(temp_output_voltage), HAL_MAX_DELAY);
                HAL_Delay(500);  // Wait 500ms before the next iteration

// Check if the '7' command is sent again to exit the loop
                if ('7' == msg)
                {
                    printf("Exiting loop on second '7' command.\n");
                    msg = 0;
                    HAL_ADC_Stop(&hadc1);
                    break;  // Exit the loop
                }
            }
        }


        if ('8' == msg)
        {
            test_repeat = 1;
            msg = 0;
            rcvbyte = 0;
            char warning_status[50];
            if (test_repeat == 1)
            {

                if (temp_voltage > 2)
                {
                    snprintf(warning_status, sizeof(warning_status), "Yes Temperature exceeds %.2fV\r\n ", temp_voltage);
                    HAL_UART_Transmit(&huart1, (uint8_t*) warning_status, strlen(warning_status), HAL_MAX_DELAY);
                }

                else
                {

                    warning_status[0] = '-';
                    HAL_UART_Transmit(&huart1, (uint8_t*) warning_status, 1, HAL_MAX_DELAY);
                }
                printf("Exiting loop on second '8' command.\n");
                //msg = 0;
            }
            test_repeat = 0;
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
    RCC_OscInitTypeDef RCC_OscInitStruct =
    { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct =
    { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit =
    { 0 };

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
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
void MX_ADC1_Init(void)
{

    /* USER CODE BEGIN ADC1_Init 0 */

    /* USER CODE END ADC1_Init 0 */

    /* USER CODE BEGIN ADC1_Init 1 */

    /* USER CODE END ADC1_Init 1 */

    /** Common config
     */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }


    /* USER CODE BEGIN ADC1_Init 2 */
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

    /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */
  if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
  {
      Error_Handler(); // Timer start Error
  }
  /* USER CODE END TIM3_Init 2 */

}


/**
 * @brief TIM4_TEST2 Initialization Function
 * @param None
 * @retval None
 */
void MX_TIM4_TEST2_Init(void)
{

    /* USER CODE BEGIN TIM4_Init 0 */

    /* USER CODE END TIM4_Init 0 */

    /* USER CODE BEGIN TIM4_Init 1 */

    /* USER CODE END TIM4_Init 1 */
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 2665;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 1333;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM4_Init 2 */

    /* USER CODE END TIM4_Init 2 */
    HAL_TIM_MspPostInit(&htim4);

}

/**
 * @brief TIM4_TEST2 Initialization Function
 * @param None
 * @retval None
 */
void MX_TIM4_TEST3_Init(void)
{

    /* USER CODE BEGIN TIM4_Init 0 */

    /* USER CODE END TIM4_Init 0 */

    /* USER CODE BEGIN TIM4_Init 1 */

    /* USER CODE END TIM4_Init 1 */
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 7999;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 1999;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
        Error_Handler();
    }

    // Start TIM4 in interrupt mode

    /* USER CODE BEGIN TIM4_Init 2 */

    /* USER CODE END TIM4_Init 2 */
    HAL_TIM_MspPostInit(&htim4);

}

/**
 * @brief TIM4_TEST2 Initialization Function
 * @param None
 * @retval None
 */
//void MX_TIM4_TEST5_Init(void)
//{
//
//    /* USER CODE BEGIN TIM4_Init 0 */
//
//    /* USER CODE END TIM4_Init 0 */
//
//    /* USER CODE BEGIN TIM4_Init 1 */
//
//    /* USER CODE END TIM4_Init 1 */
//    htim4.Instance = TIM4;
//    htim4.Init.Prescaler = 0;
//    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
//    htim4.Init.Period = 399;
//    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//    if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
//    {
//        Error_Handler();
//    }
//    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
//    {
//        Error_Handler();
//    }
//    sConfigOC.OCMode = TIM_OCMODE_PWM1;
//    sConfigOC.Pulse = duty_to_pulse;
//    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
//    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
//    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
//    {
//        Error_Handler();
//    }
//    /* USER CODE BEGIN TIM4_Init 2 */
//
//    /* USER CODE END TIM4_Init 2 */
//    HAL_TIM_MspPostInit(&htim4);
//
//}

void MX_TIM4_TEST5_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    // Initialize TIM4
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0; // Set prescaler value to divide timer clock
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 33399; // Set period for timer overflow
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.RepetitionCounter = 0; // For advanced timers
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
    {
        Error_Handler(); // Initialization Error
    }

    // Configure clock source if needed
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler(); // Clock configuration Error
    }

    // Configure master mode if needed
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
        Error_Handler(); // Master configuration Error
    }

    // Start TIM4 in interrupt mode
    if (HAL_TIM_Base_Start_IT(&htim4) != HAL_OK)
    {
        Error_Handler(); // Timer start Error
    }
}


/**
 * @brief TIM4_TEST2 Initialization Function
 * @param None
 * @retval None
 */
void MX_TIM4_TEST6_Init(void)
{

    /* USER CODE BEGIN TIM4_Init 0 */

    /* USER CODE END TIM4_Init 0 */

    /* USER CODE BEGIN TIM4_Init 1 */

    /* USER CODE END TIM4_Init 1 */
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 399;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = duty_to_pulse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN TIM4_Init 2 */

    /* USER CODE END TIM4_Init 2 */
    HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    /* USER CODE BEGIN MX_GPIO_Init_1 */
	GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /*Configure GPIO pin Output Level */
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);

      /*Configure GPIO pin : PA12 */
      GPIO_InitStruct.Pin = GPIO_PIN_9;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /* USER CODE BEGIN MX_GPIO_Init_2 */
    /* USER CODE END MX_GPIO_Init_2 */
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
