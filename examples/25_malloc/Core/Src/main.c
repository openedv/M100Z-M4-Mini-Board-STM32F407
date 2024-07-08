/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ALIENTEK
 * @brief       malloc code
 * @license     Copyright (C) 2012-2024, ALIENTEK
 ****************************************************************************************************
 * @attention
 *
 * platform     : ALIENTEK M100-STM32F407 board
 * website      : www.alientek.com
 * forum        : www.openedv.com/forum.php
 *
 * change logs  :
 * version      data         notes
 * V1.0         20240409     the first version
 *
 ****************************************************************************************************
 */
 
 /* USER CODE BEGIN Header */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../ATK_Middlewares/MALLOC/malloc.h"
#include "../../BSP/LED/led.h"
#include "../../BSP/KEY/key.h"
#include "../../BSP/LCD/lcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/**
 * @brief   Displays information about the experiment
 * @param   None
 * @retval  None
 */
void stm32f407vg_show_mesg(void)
{
	/* The serial port outputs the experimental information */
	printf("\n");
	printf("********************************\r\n");
	printf("STM32F407\r\n");
	printf("MALLOC Test\r\n");
	printf("ATOM@ALIENTEK\r\n");
	printf("********************************\r\n");
	printf("\r\n");
}
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
  uint8_t t = 0;
  uint8_t key;
  uint8_t *p_sramin = NULL;
  uint8_t *p_sramccm = NULL;
  uint32_t tp_sramin = 0;
  uint32_t tp_sramccm = 0;
  uint8_t paddr[32];
  uint16_t memused;
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
  MX_USART1_UART_Init();
  MX_FSMC_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  lcd_init();                                     /* Initialize LCD */
  my_mem_init(SRAMIN);                            /* Initialize the internal SRAM memory pool */
  my_mem_init(SRAMCCM);                           /* Initialize the CCM memory pool */
  stm32f407vg_show_mesg();

  lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
  lcd_show_string(30, 70, 200, 16, 16, "MALLOC TEST", RED);
  lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

  lcd_show_string(30, 110, 200, 16, 16, "WK_UP:Malloc & WR & Show", RED);
  lcd_show_string(30, 130, 200, 16, 16, "KEY0:Free", RED);

  lcd_show_string(30, 160, 200, 16, 16, "SRAMIN USED:", BLUE);
  lcd_show_string(30, 180, 200, 16, 16, "SRAMCCM USED:", BLUE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  key = key_scan(0);
	  if (key == WKUP_PRES)
	  {
	  	 /* request memory */
	  	 p_sramin = mymalloc(SRAMIN, 2048);
	  	 p_sramccm = mymalloc(SRAMCCM, 2048);

	  	 /* Determine if the memory request was successful */
	  	 if ((p_sramin != NULL) && (p_sramccm != NULL))
	  	 {
	  	     /* Use the requested memory */
	  	     sprintf((char *)p_sramin, "SRAMIN: Malloc Test%03d", t + SRAMIN);
	  	     lcd_show_string(30, 255, 239, 16, 16, (char *)p_sramin, BLUE);
	  	     sprintf((char *)p_sramccm, "SRAMCCM: Malloc Test%03d", t + SRAMCCM);
	  	     lcd_show_string(30, 270, 239, 16, 16, (char *)p_sramccm, BLUE);
	  	 }
	  	 else
	  	 {
	  	     myfree(SRAMIN, p_sramin);
	  	     myfree(SRAMCCM, p_sramccm);
	  	     p_sramin = NULL;
	  	     p_sramccm = NULL;
	  	 }
	  }
	  else if (key == KEY0_PRES)
	  {
	  	  /* free the memory */
	  	  myfree(SRAMIN, p_sramin);
	  	  myfree(SRAMCCM, p_sramccm);
	  	  p_sramin = NULL;
	  	  p_sramccm = NULL;
	  }

	  /* Shows the first memory address requested */
	  if (tp_sramin != (uint32_t)p_sramin || (tp_sramccm != (uint32_t)p_sramccm))
	  {
	  	  tp_sramin = (uint32_t)p_sramin;
	  	  tp_sramccm = (uint32_t)p_sramccm;

	  	  sprintf((char *)paddr, "SRAMIN: Addr: 0x%08X", (unsigned int)p_sramin);
	  	  lcd_show_string(30, 214, 239, 16, 16, (char *)paddr, BLUE);
	  	  sprintf((char *)paddr, "SRAMCCN: Addr: 0x%08X", (unsigned int)p_sramccm);
	  	  lcd_show_string(30, 230, 239, 16, 16, (char *)paddr, BLUE);
	  }
	  else if ((p_sramin == NULL) || (p_sramccm == NULL))
	  {
	  	  lcd_fill(30, 194, 239, 319, WHITE);
	  }

	  if (++t == 20)
	  {
	  	  t = 0;

	  	  /* Shows internal SRAM usage */
	  	  memused = my_mem_perused(SRAMIN);
	  	  sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
	  	  lcd_show_string(30 + 96, 160, 200, 16, 16, (char *)paddr, BLUE);

	  	  /* Show CCM usage */
	  	  memused = my_mem_perused(SRAMCCM);
	  	  sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
	  	  lcd_show_string(30 + 104, 180, 200, 16, 16, (char *)paddr, BLUE);

	  	  LED0_TOGGLE(); /* flashing LED0 indicates that the system is running */
	  }

	  HAL_Delay(10);       /* delay 10ms */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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
