/* USER CODE BEGIN Header */
/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ALIENTEK
 * @brief       camera code
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dcmi.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../BSP/DELAY/delay.h"
#include "../../BSP/LED/led.h"
#include "../../BSP/KEY/key.h"
#include "../../BSP/LCD/lcd.h"
#include "../../BSP/OV2640/ov2640.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern DMA_HandleTypeDef hdma_dcmi;
extern void (*dcmi_rx_callback)(void);
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
	printf("CAMERA Test\r\n");
	printf("ATOM@ALIENTEK\r\n");
	printf("********************************\r\n");
	printf("\r\n");
}

/* OV2640 output data format
 * 0: RGB565 format
 * 1: JPEG format
 */
static uint8_t g_ov_mode;

/* JPEG related Data cache definition */
#define JPEG_BUF_SIZE   (29 * 1024)                                             /* Size of the JPEG data buffer */
#define JPEG_LINE_SIZE  (1 * 1024)                                              /* Size of the double buffer for JPEG data DMA */
static uint32_t g_jpeg_data_buf[JPEG_BUF_SIZE] __attribute__((aligned(4)));     /* Buffer for JPEG data */
static uint32_t g_dcmi_line_buf[2][JPEG_LINE_SIZE] __attribute__((aligned(4))); /* Double buffer for JPEG data DMA */
volatile uint32_t g_jpeg_data_len = 0;                                          /* Length of valid data in the JPEG data buffer */


/* JPEG data acquisition completion flag
 * 0: Acquisition incomplete
 * 1: Acquisition complete, but not yet processed
 * 2: Processing complete, ready to receive next data
 */
volatile uint8_t g_jpeg_data_ok = 0;

static const char *jpeg_size_tbl[13] = {"QQVGA", "QCIF", "QVGA", "WGVGA", "CIF", "VGA", "SVGA", "XGA", "WXGA", "SVGA", "WXGA+", "SXGA", "UXGA"};

static const uint16_t jpeg_img_size_tbl[][2] = {
    { 160,  120},   /* QQVGA */
    { 176,  144},   /* QCIF */
    { 320,  240},   /* QVGA */
    { 400,  240},   /* WGVGA */
    { 352,  288},   /* CIF */
    { 640,  480},   /* VGA */
    { 800,  600},   /* SVGA */
    {1024,  768},   /* XGA */
    {1280,  800},   /* WXGA */
    {1280,  960},   /* XVGA */
    {1440,  900},   /* WXGA+ */
    {1280, 1024},   /* SXGA */
    {1600, 1200},   /* UXGA */
};

/**
 * @bref        Process JPEG data
 * @note        Called in DCMI capture interrupt
 * @param       None
 * @retval      None
 */
void jpeg_data_process(void)
{
	uint16_t rlen;
	uint32_t *pbuf;
	uint16_t i;

	if (g_ov_mode != 0)                                                         /* JPEG mode */
	{
	    if (g_jpeg_data_ok == 0)                                                /* JPEG data not yet captured */
	    {
	        __HAL_DMA_DISABLE(&hdma_dcmi);                              		/* Disable DMA stream used for DCMI output */
	        while (DMA2_Stream1->CR & 0x01);                                    /* Wait for DMA configuration */

	        rlen = JPEG_LINE_SIZE - __HAL_DMA_GET_COUNTER(&hdma_dcmi);  		/* Calculate remaining length */
	        pbuf = g_jpeg_data_buf + g_jpeg_data_len;                           /* Move to the end of valid data in JPEG data buffer to add new JPEG data */

	        if (DMA2_Stream1->CR & (1 << 19))                                   /* DMA is processing storage */
	        {
	            for (i=0; i<rlen; i++)
	            {
	                pbuf[i] = g_dcmi_line_buf[1][i];
	            }
	        }
	        else                                                                /* DMA is processing storage */
	        {
	            for (i=0; i<rlen; i++)
	            {
	                pbuf[i] = g_dcmi_line_buf[0][i];
	            }
	        }
	        g_jpeg_data_len += rlen;                                            /* Update the length of valid data in the JPEG data buffer */
	        g_jpeg_data_ok = 1;                                                 /* Mark JPEG data as captured */
	    }
	    else if (g_jpeg_data_ok == 2)                                           /* Previously captured JPEG data has been processed */
	    {
	        __HAL_DMA_SET_COUNTER(&hdma_dcmi, JPEG_LINE_SIZE);          		/* Set the number of data items to be transferred by DMA */
	        __HAL_DMA_ENABLE(&hdma_dcmi);                               		/* Enable DMA stream */
	        g_jpeg_data_len = 0;                                                /* Start receiving new JPEG data */
	        g_jpeg_data_ok = 0;                                                 /* Mark JPEG data as not yet captured */
	    }
	}
	else                                                                		/* RGB565 mode */
	{
	    lcd_set_cursor(0, 0);                                           		/* Set cursor position */
	    lcd_write_ram_prepare();                                        		/* Prepare to write to GRAM */
	}
}

/**
 * @bref 	JPEG data processing callback
 * @note 	Used only when using dual buffer mode, called in DMA transfer completion interrupt
 * @param    None
 * @retval   None
 */
static void jpeg_dcmi_rx_callback(void)
{
	uint16_t i;
	volatile uint32_t *pbuf;

	pbuf = g_jpeg_data_buf + g_jpeg_data_len;           /* Move to the end of valid data */
	if (DMA2_Stream1->CR & (1 << 19))                   /* DMA is processing storage */
	{
	    for (i=0; i<JPEG_LINE_SIZE; i++)                /* Copy DMA storage */
	    {
	        pbuf[i] = g_dcmi_line_buf[0][i];
	    }
	    g_jpeg_data_len += JPEG_LINE_SIZE;              /* Update the length of valid data in the JPEG data buffer */
	}
	else                                                /* DMA is processing storage */
	{
	    for (i=0; i<JPEG_LINE_SIZE; i++)                /* Copy DMA storage to JPEG data buffer */
	    {
	        pbuf[i] = g_dcmi_line_buf[1][i];
	    }
	    g_jpeg_data_len += JPEG_LINE_SIZE;              /* Update the length of valid data in the JPEG data buffer */
	}
}

/**
 * @bref 	JPEG mode test
 * @note 	gets the JPEG data from the OV2640 output and sends it to the PC via USART2
 * @param   None
 * @retval  None
 */
static void jpeg_test(void)
{
	uint8_t msgbuf[15];     	/* Prompt message buffer */
	uint8_t size = 2;       	/* Default to use QVGA */
	uint8_t contrast = 2;   	/* Default to use normal contrast */

    uint8_t *p;
    uint32_t jpeglen;
    uint8_t headok;
    uint32_t i;
    uint32_t jpegstart;
    uint8_t key;

    lcd_clear(WHITE);
    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "OV2640 JPEG Mode", RED);
    lcd_show_string(30, 100, 200, 16, 16, "KEY0:Contrast", RED);  					/* Contrast */
    lcd_show_string(30, 120, 200, 16, 16, "WKUP:Size", RED);       					/* Resolution setting */
    sprintf((char *)msgbuf, "JPEG Size:%s", jpeg_size_tbl[size]);
    lcd_show_string(30, 140, 200, 16, 16, (char*)msgbuf, RED);     					/* Current JPEG resolution */

    ov2640_jpeg_mode();                                             				/* Configure OV2640 to JPEG mode */
    dcmi_rx_callback = jpeg_dcmi_rx_callback;                       				/* DMA transfer completion callback */
    dcmi_dma_init((uint32_t)g_dcmi_line_buf[0],                    					/* Configure DCMI DMA */
                 (uint32_t)g_dcmi_line_buf[1],
                 JPEG_LINE_SIZE,
                 DMA_MDATAALIGN_WORD,
                 DMA_MINC_ENABLE);
    ov2640_outsize_set(jpeg_img_size_tbl[size][0], jpeg_img_size_tbl[size][1]);   	/* Set image output size */
    dcmi_start();   																/* Start DCMI transfer */

    while (1)
    {
        if (g_jpeg_data_ok == 1)                                                    /* JPEG data collection completed */
        {
            p = (uint8_t *)g_jpeg_data_buf;
            printf("g_jpeg_data_len:%d\r\n", g_jpeg_data_len << 2);                 /* Print JPEG data length */
            lcd_show_string(30, 170, 210, 16, 16, "Sending JPEG data...", RED);     /* Prompt for transferring JPEG data */
            jpeglen = 0;
            headok = 0;

            for (i=0; i<((g_jpeg_data_len << 2) - 1); i++)                          /* Traverse the JPEG data to find the JPEG header and footer */
            {
                if ((p[i] == 0xFF) && (p[i + 1] == 0xD8))                           /* Found JPEG data */
                {
                    jpegstart = i;
                    headok = 1;
                }

                if (headok != 0)                                                    /* After finding the JPEG header, start looking for the JPEG footer */
                {
                    if ((p[i] == 0xFF) && (p[i + 1] == 0xD9))
                    {
                        jpeglen = i - jpegstart + 2;
                        break;
                    }
                }
            }

            if (jpeglen != 0)                                                       /* JPEG data is valid */
            {
                p += jpegstart;                                                     /* Move to the position of the JPEG header */
                for (i=0; i<jpeglen; i++)                                           /* Send the entire valid JPEG data (from JPEG header to JPEG footer) */
                {
                    USART2->DR = p[i];
                    while ((USART2->SR & 0X40) == 0);
                    key = key_scan(0);                                              /* Data transmission can be interrupted by any keypress */
                    if (key != 0)
                    {
                        break;
                    }
                }
            }

            if (key != 0)
            {
                lcd_show_string(30, 170, 210, 16, 16, "Quit Sending data   ", RED); /* Prompt for JPEG data transmission */
                switch (key)
                {
                    case KEY0_PRES: /* Adjust contrast */
                    {
                        contrast++;
                        if (contrast > 4)
                        {
                            contrast = 0;
                        }
                        ov2640_contrast(contrast);
                        sprintf((char *)msgbuf, "Contrast:%d", (int8_t)contrast - 2);
                        break;
                    }
                    case WKUP_PRES: /* Set resolution */
                    {
                        size++;
                        if (size > 12)
                        {
                            size = 0;
                        }
                        ov2640_outsize_set(jpeg_img_size_tbl[size][0], jpeg_img_size_tbl[size][1]);
                        sprintf((char *)msgbuf, "JPEG Size:%s", jpeg_size_tbl[size]);
                    }
                    default:
                    {
                        break;
                    }
                }
                lcd_fill(30, 140, 239, 190 + 16, WHITE);
                lcd_show_string(30, 140, 210, 16, 16, (char*)msgbuf, RED);          /* Display prompt content */
                HAL_Delay(800);
            }
            else
            {
                lcd_show_string(30, 170, 210, 16, 16, "Send data complete!!", RED); /* Prompt for JPEG data transmission completion */
            }

            g_jpeg_data_ok = 2;                                                     /* Mark that JPEG data processing is complete and can start receiving the next data */
       }
    }
}


/**
 * @bref        RGB565 Mode test
 * @note        Get the RGB data from the OV2640 output and display it directly on the LCD
 * @param       None
 * @retval      None
 */
static void rgb565_test(void)
{
    uint8_t key;
    uint8_t contrast = 2;   /* Default to normal contrast */
    uint8_t scale = 1;      /* Default to full-screen scaling */
    uint8_t msgbuf[15];     /* Prompt message buffer */

    lcd_clear(WHITE);
    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "OV2640 RGB565 Mode", RED);
    lcd_show_string(30, 100, 200, 16, 16, "KEY0:Contrast", RED);            /* Contrast setting */
    lcd_show_string(30, 160, 200, 16, 16, "WKUP:FullSize/Scale", RED);      /* Scaling setting */

    ov2640_rgb565_mode();                                                   /* Configure OV2640 to RGB565 mode */
    dcmi_dma_init((uint32_t)&(LCD->LCD_RAM),                                /* Configure DCMI DMA */
                 0,
                 1,
                 DMA_MDATAALIGN_HALFWORD,
                 DMA_MINC_DISABLE);
    ov2640_outsize_set(lcddev.width, lcddev.height);                        /* Full screen scaling */
    dcmi_start();                                                           /* Start DCMI transmission */

    while (1)
    {
        key = key_scan(0);

        if (key != 0)
        {
            dcmi_stop();                                                    /* Stop DCMI transmission */
            switch (key)
            {
                case KEY0_PRES:                                             /* Set contrast */
                {
                    contrast++;
                    if (contrast > 4)
                    {
                        contrast = 0;
                    }
                    ov2640_contrast(contrast);
                    sprintf((char *)msgbuf, "Contrast:%d", (int8_t)contrast - 2);
                    break;
                }
                case WKUP_PRES:                                             /* Set scaling */
                {
                    scale = !scale;
                    if (scale == 0)                                         /* No scaling */
                    {
                        ov2640_image_win_set((1600 - lcddev.width) >> 1,
                                             (1200 - lcddev.height) >> 1,
                                             lcddev.width,
                                             lcddev.height);
                        ov2640_outsize_set(lcddev.width, lcddev.height);
                        sprintf((char *)msgbuf, "Full Size 1:1");
                    }
                    else                                                    /* Full screen scaling */
                    {
                        ov2640_image_win_set(0, 0, 1600, 1200);
                        ov2640_outsize_set(lcddev.width, lcddev.height);
                        sprintf((char *)msgbuf, "Scale");
                    }
                }
                default:
                {
                    break;
                }
            }
            lcd_show_string(30, 50, 210, 16, 16, (char*)msgbuf, RED);       /* Display prompt content */
            HAL_Delay(800);
            dcmi_start();                                                   /* Restart DCMI transmission */
        }
        HAL_Delay(10);
    }
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
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  delay_init(168);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_FSMC_Init();
  MX_TIM6_Init();
  MX_USART2_UART_Init();
  MX_DCMI_Init();
  /* USER CODE BEGIN 2 */
  lcd_init();                                         /* Initialize LCD */
  stm32f407vg_show_mesg();
  lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
  lcd_show_string(30, 70, 200, 16, 16, "OV2640 TEST", RED);
  lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

  while (ov2640_init() != 0)                          /* Initializes the OV2640 */
  {
      lcd_show_string(30, 130, 200, 16, 16, "OV2640 ERROR ", RED);
      HAL_Delay(200);
      lcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
      HAL_Delay(500);
      LED0_TOGGLE();
  }
  lcd_show_string(30, 130, 200, 16, 16, "OV2640 OK     ", RED);
  ov2640_flash_intctrl();                             /* Internal control flash */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  t++;
	  key = key_scan(0);

	  if (key == KEY0_PRES)                                       /* RGB565 Mode */
	  {
	     g_ov_mode = 0;
	     break;
	  }
	  else if (key == WKUP_PRES)                                  /* JPEG Mode */
	  {
	     g_ov_mode = 1;
	     break;
	  }

	  if (t == 100)
	  {
	     lcd_show_string(30, 150, 230, 16, 16, "KEY0:RGB565  KEY_UP:JPEG", RED);
	  }

	  else if (t == 200)
	  {
	     t = 0;
	     lcd_fill(30, 150, 230, 150 + 16, WHITE);
	     LED0_TOGGLE();
	  }

	  HAL_Delay(5);

	  if (g_ov_mode == 0)
	  {
	     rgb565_test();                                           /* RGB565 Mode test */
	  }
	  else
	  {
	     jpeg_test();                                             /* JPEG Mode test */
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
