## ADC_Temperature example<a name="brief"></a>

### 1 Brief
The function of this program is that ADC collects the voltage on channel 16, converts the voltage value into the temperature value through the algorithm, and displays the converted temperature value on the lcd module screen.
### 2 Hardware Hookup
The hardware resources used in this example are:
+ LED0 - PE5
+ USART1 - PA9/PA10
+ ADC1 - Channel16
+ ALIENTEK  2.8/3.5/4.3/7 inch TFTLCD module

The ADC used in this example is an on-chip resource of STM32F407, so there is no corresponding connection schematic.

### 3 STM32CubeIDE Configuration


We copy the **11_tftlcd** project and name both the project and the.ioc file **18_adc_temperature**.Next we start the ADC configuration by double-clicking the **18_adc_temperature.ioc** file.

The ADC channel 16 used in this example is set as shown below.

<img src="../../1_docs/3_figures/18_adc_temperature/01_config.png">

<img src="../../1_docs/3_figures/18_adc_temperature/02_parameter.png">

Click **File > Save**, and you will be asked to generate code.Click **Yes**.

##### code
We add some code to adc.c, starting with ADC1's initialization function ``MX_ADC1_Init``, as follows.
###### adc.c

We added three separate functions as follows.
```c#
uint32_t adc_get_result(void)
{
    HAL_ADC_Start(&hadc1);                            /* Turn on the ADC */
    HAL_ADC_PollForConversion(&hadc1, 10);            /* Round robin transformation */
    return (uint16_t)HAL_ADC_GetValue(&hadc1);        /* Returns the result of the last ADC1 rule group transformation */
}

/**
 * @brief   Takes the transformation value of the channel ch, takes it times, and averages it
 * @param   times : Get the number of times
 * @retval  channel ch the average of times conversion results
 */
uint32_t adc_get_result_average(uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;

    for (t = 0; t < times; t++)     /* Get The times data */
    {
        temp_val += adc_get_result();
        HAL_Delay(5);               /* delay 5ms */
    }

    return temp_val / times;        /* Returning the average */
}

/**
 * @brief  Gets the internal temperature sensor temperature value
 * @param  none
 * @retval The temperature (expanded by 100 times in degrees Celsius).
 */
short adc_get_temperature(void)
{
    uint32_t adcx;
    short result;
    double temperature;

    adcx = adc_get_result_average(10);  /* The internal temperature sensor channel was read and 10 times were averaged */
    temperature = (float)adcx * (3.3 / 4095);            /* Convert to voltage values */
    temperature = (temperature - 0.76) * 400 + 25;       /* calculating temperature */
    result = temperature *= 100;                         /* 100 times bigger. */
    return result;
}
```
After obtaining the output voltage of the temperature sensor, the above code converts the voltage value to the actual temperature value through the temperature conversion formula.

###### main.c
Here's the main function.
```c#
int main(void)
{
  /* USER CODE BEGIN 1 */
  int16_t temperature;
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
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  lcd_init();                                         /* Initialize LCD */
  stm32f407vg_show_mesg();

  lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
  lcd_show_string(30, 70, 200, 16, 16, "Temperature TEST", RED);
  lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);

  lcd_show_string(30, 110, 200, 16, 16, "TEMPERATE: 00.00C", BLUE);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    temperature = adc_get_temperature();  /* Get the internal temperature sensor temperature value */

    if (temperature < 0)
    {
      temperature = -temperature;
      lcd_show_string(30 + 10 * 8, 110, 16, 16, 16, "-", BLUE);
    }
    else
    {
      lcd_show_string(30 + 10 * 8, 110, 16, 16, 16, " ", BLUE);
    }

    lcd_show_xnum(30 + 11 * 8, 110, temperature / 100, 2, 16, 0, BLUE);
    lcd_show_xnum(30 + 14 * 8, 110, temperature % 100, 2, 16, 0x80, BLUE);

    LED0_TOGGLE();             /* flashing LED0 indicates that the system is running */
    HAL_Delay(1000);           /* delay 1000ms */
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
```
This code is relatively simple, constantly calling the ``adc_get_temperature`` function to obtain the temperature value collected by the internal temperature sensor, and the temperature value is displayed in real time through the lcd module screen.


### 4 Running
#### 4.1 Compile & Download
After the compilation is complete, connect the DAP and the Mini Board, and then connect to the computer together to download the program to the Mini Board.
#### 4.2 Phenomenon
Press the **RESET** button to begin running the program on your Mini Board, observe the LED0 flashing on the Mini Board, open the serial port and the host computer **ATK-XCOM** can see the prompt information of the example, indicating that the code download is successful. The temperature value of the internal temperature sensor on the Mini Board will be displayed on the lcd screen, as shown in the following figure.

<img src="../../1_docs/3_figures/18_adc_temperature/03_lcd.png">

[jump to title](#brief)