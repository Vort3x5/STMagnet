/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "qmc5883p.h"
#include "ssd1306.h"
#include "filters.h"
#include "calibration.h"
#include <stdint.h>
#include <stdio.h>

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

/* USER CODE BEGIN PV */

QMC5883P_t magnetometer;
LowPassFilter_t filter;
MagCalibration_t mag_cal;
CalibrationData_t cal_data;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void I2C_Scan(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int _write(int file, char *ptr, int len)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, HAL_MAX_DELAY);
	return len;
}

void I2C_Scan(void)
{
	HAL_StatusTypeDef result;
	uint8_t found = 0;

	printf("Scanning I2C bus... \r\n");

	for (uint8_t i = 1; i < 128; ++i)
	{
		result = HAL_I2C_IsDeviceReady(&hi2c1, i << 1, 1, 10);
		if (result == HAL_OK)
		{
			printf("  Device found at addr: 0x%02X\r\n", i);
			++found;
		}
	}

	if (!found)
		printf("  No I2C devices found! check wiring.\r\n");
	else
		printf("  Total devices found: %d\r\n", found);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
    HAL_StatusTypeDef status;
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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_Delay(2000);
	printf("I2C1 State before init: %d\r\n", hi2c1.State);
	printf("I2C1 Error Code: 0x%08lX\r\n", hi2c1.ErrorCode);

	// Check if I2C is ready
	status = HAL_I2C_GetState(&hi2c1);
	printf("I2C GetState: %d\r\n", status);

// Try I2C scan to see if bus works
	printf("\r\nTrying I2C bus scan...\r\n");
    I2C_Scan();
    printf("\r\n");
    
    printf("Initializing QMC5883P magnetometer...\r\n");
    status = QMC5883P_Init(&magnetometer, &hi2c1);
    
    if (status == HAL_OK) 
	{
        printf("SUCCESS: Magnetometer initialized!\r\n");
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    } 
	else 
	{
        printf("ERROR: Failed to initialize magnetometer (code: %d)\r\n", status);
        while(1) 
		{
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            HAL_Delay(200);
        }
    }
	printf("Initializing OLED display...\r\n");
	if (SSD1306_Init(&hi2c1)) 
	{
		printf("OLED initialized!\r\n");
		SSD1306_Clear();
		SSD1306_GotoXY(10, 0);
		SSD1306_Puts("Magnetometer", SSD1306_WHITE);
		SSD1306_GotoXY(30, 16);
		SSD1306_UpdateScreen();
		HAL_Delay(2000);
	} 
	else
		printf("OLED initialization failed!\r\n");
		
	Calibration_Init(&mag_cal);
	Calibration_LoadDefaults(&mag_cal);
	LowPassFilter_Init(&filter, 0.1f);

	printf("Using saved calibration values\r\n");
	SSD1306_Clear();
	SSD1306_GotoXY(10, 20);
	SSD1306_Puts("Magnetometer", SSD1306_WHITE);
	SSD1306_GotoXY(30, 32);
	SSD1306_Puts("READY!", SSD1306_WHITE);
	SSD1306_UpdateScreen();
	HAL_Delay(1000);

	printf("Starting measurements...\r\n");
	printf("Move a magnet near the sensor to see values change!\r\n\r\n");
	HAL_Delay(1000);
   /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
		if (QMC5883P_ReadRaw(&magnetometer) == HAL_OK)
		{
			float cal_x, cal_y, cal_z;
			Calibration_Apply(&mag_cal,
							 magnetometer.x, magnetometer.y, magnetometer.z,
							 &cal_x, &cal_y, &cal_z);
			
			LowPassFilter_Update(&filter, (int16_t)cal_x, (int16_t)cal_y, (int16_t)cal_z);
			Vector3f_t filtered = filter.filtered;
			
			float field_strength = Calibration_GetFieldStrength(filtered.x, filtered.y, filtered.z);
			
			SSD1306_Clear();
			SSD1306_GotoXY(0, 0);
			SSD1306_Puts("MAGNETOMETER", SSD1306_WHITE);
			
			SSD1306_Printf(0, 16, SSD1306_WHITE, "X:%7.0f", filtered.x);
			SSD1306_Printf(0, 28, SSD1306_WHITE, "Y:%7.0f", filtered.y);
			SSD1306_Printf(0, 40, SSD1306_WHITE, "Z:%7.0f", filtered.z);
			
			SSD1306_DrawLine(0, 50, 127, 50, SSD1306_WHITE);
			
			SSD1306_Printf(0, 54, SSD1306_WHITE, "%.1fuT", field_strength);
			
			if (mag_cal.is_calibrated) 
			{
				SSD1306_GotoXY(90, 54);
				SSD1306_Puts("CAL", SSD1306_WHITE);
			} 
			else 
			{
				SSD1306_GotoXY(90, 54);
				SSD1306_Puts("RAW", SSD1306_WHITE);
			}
			
			SSD1306_UpdateScreen();
			
			printf("X:%7.1f Y:%7.1f Z:%7.1f | %.1f uT | %s\r\n",
				   filtered.x, filtered.y, filtered.z,
				   field_strength,
				   mag_cal.is_calibrated ? "CAL" : "RAW");
			
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		}
		HAL_Delay(100);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
