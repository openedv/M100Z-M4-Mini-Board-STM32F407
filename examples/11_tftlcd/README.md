## TFTLCD example<a name="brief"></a>

### 1 Brief
In this code we will use the TFTLCD interface on the Mini Board's baseboard to illuminate the TFTLCD and implement functions such as displaying ASCII characters and colors.
### 2 Hardware Hookup
The hardware resources used in this example are:
+ LED0 - PE5
+ ALIENTEK  2.8/3.5/4.3/7 inch TFTLCD module

The connection of this interface to the onboard MCU is shown in the schematic diagram below:

<img src="../../1_docs/3_figures/11_tftlcd/01_sch.png">

### 3 STM32CubeIDE Configuration

Let's copy the project from  **04_uart** and name both the project and the.ioc file **11_tftlcd**. Next we start the TFTLCD_MCU configuration by double-clicking the **11_tftlcd.ioc** file.

At the **Pinout & Configuration** section, configure the backlight pin **PB5** for the LCD, as shown in the following figure:

<img src="../../1_docs/3_figures/11_tftlcd/02_gpio.png">

Click **System Core->GPIO**, and config the PB5.

<img src="../../1_docs/3_figures/11_tftlcd/03_parameter.png">

Next, configure the parameters of the FSMC in **Connectivity->FSMC**.

<img src="../../1_docs/3_figures/11_tftlcd/04_fsmc.png">

Next, enable the pins used by the LCD to correspond one by one with the schematic diagram, that is, check them all.

<img src="../../1_docs/3_figures/11_tftlcd/05_view.png">

Click **File > Save**, and you will be asked to generate code.Click **Yes**.

##### code
Create a new folder named LCD, then create four files within the LCD folder: lcd.c, lcd.h, lcd_ex.c, and lcdfont.h.

###### lcd.h
```c#
#define LCD_FSMC_NEX         1              /* When FSMC_NE1 is connected to LCD_CS, the value range can only be: 1~4 */
#define LCD_FSMC_AX          19             /* Use FSMC_A19 connected to LCD_RS, the value range is: 0 ~ 25 */

/* LCD important parameter set */
typedef struct
{
    uint16_t width;     /* LCD width */
    uint16_t height;    /* LCD height */
    uint16_t id;        /* LCD ID */
    uint8_t dir;        /* Landscape or portrait control: 0, portrait; 1. Landscape. */
    uint16_t wramcmd;   /* Start writing gram instructions */
    uint16_t setxcmd;   /* Set the x-coordinate command */
    uint16_t setycmd;   /* Set the y-coordinate command */
} _lcd_dev;
```
The above is a structure ``_lcd_dev`` in lcd.h, which is used to store some important parameter information of the LCD. There are also some variable declarations and details can be viewed by opening the code.

###### lcd.c
Let's focus on the following LCD functions, as follows:
```c#
void lcd_init(void)
{
    HAL_Delay(50);        /* After initializing the FSMC, a certain amount of time must be waited before the initialization can start */

    /* Try a read of 9341 ID */
    lcd_wr_regno(0XD3);
    lcddev.id = lcd_rd_data();  /* dummy read */
    lcddev.id = lcd_rd_data();  /* Read to 0X00 */
    lcddev.id = lcd_rd_data();  /* Read 0X93 */
    lcddev.id <<= 8;
    lcddev.id |= lcd_rd_data(); /* Read 0X41 */

    if (lcddev.id != 0X9341)    /* Not 9341, try to see if it's ST7789 */
    {
        lcd_wr_regno(0X04);
        lcddev.id = lcd_rd_data();      /* dummy read */
        lcddev.id = lcd_rd_data();      /* Read to 0X85 */
        lcddev.id = lcd_rd_data();      /* Read 0X85 */
        lcddev.id <<= 8;
        lcddev.id |= lcd_rd_data();     /* Read 0X52 */

        if (lcddev.id == 0X8552)        /* Convert the ID of 8552 to 7789 */
        {
            lcddev.id = 0x7789;
        }

        if (lcddev.id != 0x7789)        /* Also not ST7789, try not NT35310 */
        {
            lcd_wr_regno(0xD4);
            lcddev.id = lcd_rd_data();  /* dummy read */
            lcddev.id = lcd_rd_data();  /* Read back 0x01 */
            lcddev.id = lcd_rd_data();  /* Read back 0x53 */
            lcddev.id <<= 8;
            lcddev.id |= lcd_rd_data(); /* Read back 0x10 */

            if (lcddev.id != 0x5310)    /* Also not NT35310, try to see if it is ST7796 */
            {
                lcd_wr_regno(0XD3);
                lcddev.id = lcd_rd_data();  /* dummy read */
                lcddev.id = lcd_rd_data();  /* Read to 0X00 */
                lcddev.id = lcd_rd_data();  /* Read 0X77 */
                lcddev.id <<= 8;
                lcddev.id |= lcd_rd_data(); /* Read 0X96 */

                if (lcddev.id != 0x7796)    /* Also not ST7796, try to see if it is NT35510 */
                {
                    /* Send key (provided by manufacturer) */
                    lcd_write_reg(0xF000, 0x0055);
                    lcd_write_reg(0xF001, 0x00AA);
                    lcd_write_reg(0xF002, 0x0052);
                    lcd_write_reg(0xF003, 0x0008);
                    lcd_write_reg(0xF004, 0x0001);

                    lcd_wr_regno(0xC500);       /* Read the lower eight bits of the ID */
                    lcddev.id = lcd_rd_data();  /* Read back 0x55 */
                    lcddev.id <<= 8;

                    lcd_wr_regno(0xC501);       /* Read the high eight bits of the ID */
                    lcddev.id |= lcd_rd_data(); /* Read back 0x10 */

                    HAL_Delay(5);                /* Wait 5ms, because instruction 0XC501 is a software reset instruction for 1963, wait 5ms for 1963 reset to complete */

                    if (lcddev.id != 0x5510)    /* Also not NT5510, try to see if it is ILI9806 */
                    {
                        lcd_wr_regno(0XD3);
                        lcddev.id = lcd_rd_data();  /* dummy read */
                        lcddev.id = lcd_rd_data();  /* Read back 0X00 */
                        lcddev.id = lcd_rd_data();  /* Read back 0X98 */
                        lcddev.id <<= 8;
                        lcddev.id |= lcd_rd_data(); /* Read back 0X06 */

                        if (lcddev.id != 0x9806)    /* Not ILI9806 either, try to see if it's SSD1963 */
                        {
                            lcd_wr_regno(0xA1);
                            lcddev.id = lcd_rd_data();
                            lcddev.id = lcd_rd_data();  /* Read back 0x57 */
                            lcddev.id <<= 8;
                            lcddev.id |= lcd_rd_data(); /* Read back 0x61 */

                            if (lcddev.id == 0x5761) lcddev.id = 0x1963; /* The ID read back by SSD1963 is 5761H, which we force to be 1963 for convenience */
                        }
                    }
                }
            }
        }
    }

    if (lcddev.id == 0X7789)
    {
        lcd_ex_st7789_reginit();    /* Perform ST7789 initialization */
    }
    else if (lcddev.id == 0X9341)
    {
        lcd_ex_ili9341_reginit();   /* Perform ILI9341 initialization */
    }
    else if (lcddev.id == 0x5310)
    {
        lcd_ex_nt35310_reginit();   /* Perform NT35310 initialization */
    }
    else if (lcddev.id == 0x7796)
    {
        lcd_ex_st7796_reginit();    /* Perform ST7796 initialization */
    }
    else if (lcddev.id == 0x5510)
    {
        lcd_ex_nt35510_reginit();   /* Perform NT35510 initialization */
    }
    else if (lcddev.id == 0x9806)
    {
        lcd_ex_ili9806_reginit();   /* Perform ILI9806 initialization */
    }
    else if (lcddev.id == 0x1963)
    {
        lcd_ex_ssd1963_reginit();   /* Perform SSD1963 initialization */
        lcd_ssd_backlight_set(100); /* The backlight is set to its brightest */
    }

    lcd_display_dir(0); /* Portrait is the default */
    LCD_BL(1);          /* Light up the backlight */
    lcd_clear(WHITE);
}

/**
 * @brief   LCD write data
 * @param   data: data to be written
 * @retval  None
 */
void lcd_wr_data(volatile uint16_t data)
{
    data = data;
    LCD->LCD_RAM = data;
}

/**
 * @brief   LCD write register number/address function
 * @param   regno: register number/address
 * @retval  None
 */
void lcd_wr_regno(volatile uint16_t regno)
{
    regno = regno;
    LCD->LCD_REG = regno;   /* Writes the register sequence number to be written */

}

/**
 * @brief   LCD write register function
 * @param   regno: register number/address
 * @param   data: data to be written
 * @retval  None
 */
void lcd_write_reg(uint16_t regno, uint16_t data)
{
    LCD->LCD_REG = regno;   /* Writes the register sequence number to be written */
    LCD->LCD_RAM = data;    /* Write in data */
}

/**
 * @brief   LCD delay function
 * @param   i: The value of the delay.
 * @retval  None
 */
static void lcd_opt_delay(uint32_t i)
{
    while (i--);
}

/**
 * @brief   LCD read register function
 * @param   None.
 * @retval  The data read
 */
static uint16_t lcd_rd_data(void)
{
    volatile uint16_t ram;
    ram = LCD->LCD_RAM;
    return ram;
}

/**
 * @brief   LCD ready to write GRAM
 * @param   None.
 * @retval  None.
 */
void lcd_write_ram_prepare(void)
{
    LCD->LCD_REG = lcddev.wramcmd;
}
```
First is the LCD initialization function, which executes different code based on the model of the control IC, thus improving the overall versatility of the program. Following are some read and write functions for the LCD.

###### main.c
Your finished main() function should look like the following:
```c#
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t x = 0;
  uint8_t lcd_id[12];
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
  /* USER CODE BEGIN 2 */

  lcd_init();                                         /* Initialize LCD */
  stm32f407vg_show_mesg();
  sprintf((char *)lcd_id, "LCD ID:%04X", lcddev.id);  /* Print the LCD ID to the lcd id array */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    switch (x)
    {
      case 0:lcd_clear(WHITE);break;
      case 1:lcd_clear(BLACK);break;
      case 2:lcd_clear(BLUE);break;
      case 3:lcd_clear(RED);break;
      case 4:lcd_clear(MAGENTA);break;
      case 5:lcd_clear(GREEN);break;
      case 6:lcd_clear(CYAN);break;
      case 7:lcd_clear(YELLOW);break;
      case 8:lcd_clear(BRRED);break;
      case 9:lcd_clear(GRAY);break;
      case 10:lcd_clear(LGRAY);break;
      case 11:lcd_clear(BROWN);break;
    }

    lcd_show_string(10, 40, 240, 32, 32, "STM32", RED);
    lcd_show_string(10, 80, 240, 24, 24, "TFTLCD TEST", RED);
    lcd_show_string(10, 110, 240, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(10, 130, 240, 16, 16, (char *)lcd_id, RED); /* Display LCD ID */
    x++;

    if (x == 12)
    {
       x = 0;
    }

    LED0_TOGGLE();          /* flashing LED0 indicates that the system is running */
    HAL_Delay(1000);        /* delay 1000ms */
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
```

### 4 Running
#### 4.1 Compile & Download
After the compilation is complete, connect the DAP and the Mini Board, and then connect to the computer together to download the program to the Mini Board.
#### 4.2 Phenomenon
Press the **RESET** button to begin running the program on your Mini Board, observe the LED0 flashing on the Mini Board, open the serial port and the host computer **ATK-XCOM** can see the prompt information of the example, indicating that the code download is successful. Connecting the LCD module and the Mini Board, the phenomenon is shown as follows. Here a 3.5-inch screen is used as an example.

<img src="../../1_docs/3_figures/11_tftlcd/06_lcd.png">

[jump to title](#brief)