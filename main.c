/*--------------------------------------------------------------------------------
  @Author  jamie 2899987@qq.com
  @Date  2022-08-23 09:31
  @LastEditors  ChenJiaRan
  @LastEditTime  2022-08-30 16:51
  @Description   项目:821008 两C一A 上电20V CS32L010+SW3516P+SW3516P 注意P版本不能单独设置A口功率
  @Version  V1.0
  @Note
 *                      CS32L010 QFN20 3*3
 *                        ---------------
 *     IIC_SDA2 ---------|1(PD4)  (PD3)20|--------- IIC_SCL1
 *     IIC_SCL2 ---------|2(PD5)  (PD2)19|--------- IIC_SDA1
 *              ---------|3(PD6)  (PD1)18|--SWCLK--
 *              ---------|4(NRST) (PC7)17|--SWDIO--
 *              ---------|5(PA1)  (PC6)16|---------
 *
 *      ???     ---------|6(PA2)  (PC5)15|---------
 *              ---------|7(VSS)  (PC4)14|---------
 *              ---------|8(VCAP) (PC3)13|---------
 *      ???     ---------|9(VDD)  (PB4)12|--------- UART0_TX
 *              ---------|10(PA3) (PB5)11|---------
 * 			              ---------------
待处理:

    插着上电断CC不算BUG
    去掉新插入检测,先写好功率,有变功率就换

    降低I2C速率和下拉时间,降功耗,反应不用快
待测试:
    先写功率再断CC 初始化功率二不断CC

待优化:
    ADC*mA函数
    I2C多从
--------------------------------------------------------------------------------*/

#include "all_head.h"
/* Private includes */

// #define PAout(n) ((Bits_16_TypeDef *)(&(GPIOA->ODR)))->bit##n //仅KEIL?

SYSTEM_Tick_st SysTick_c = {0};
// bool power_on_first = false; //上电2秒内所有切换,不检电流不断CC,可能是插着C口上电

void init_sw3516();
void check_Port();
void change_Power();
void cycle_check();

void SystemClock_Config(void);

void LED_IO_Init()
{
    GPIO_InitTypeDef gpioinitstruct = {0};
    gpioinitstruct.Pin = GPIO_PIN_5;
    gpioinitstruct.Mode = GPIO_MODE_OUTPUT;
    gpioinitstruct.OpenDrain = GPIO_PUSHPULL;
    gpioinitstruct.Debounce.Enable = GPIO_DEBOUNCE_DISABLE;
    gpioinitstruct.SlewRate = GPIO_SLEW_RATE_HIGH;
    gpioinitstruct.DrvStrength = GPIO_DRV_STRENGTH_HIGH;
    gpioinitstruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &gpioinitstruct);

    /* Reset PIN to switch off the LED */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
}
u32 test;
/*
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    // __HAL_RCC_GPIOC_CLK_ENABLE();
    // LED_IO_Init(); ///////////////测试用

    __HAL_RCC_GPIOD_CLK_ENABLE();
    CCC_I2C_Init();
    CCC_I2C_Init_COPY_22();
    HAL_Delay(50);

    Switch_Device = SW_IC_1;
    CCC_I2C_ReadReg((u32) & (SW3516H->Chip_ver)); //可以先读一下版本，SW未准备好会卡在这
    Switch_Device = SW_IC_2;
    CCC_I2C_ReadReg((u32) & (SW3516H->Chip_ver)); //可以先读一下版本，SW未准备好会卡在这

    // SW3516H_r0xB7_st PD_Config7;
    // PD_Config7.w = CCC_I2C_ReadReg((u32) & (SW3516H->PD_Config7));
    // test = PD_Config7.w;

    init_sw3516();

    while (1)
    {

        if (SysTick_c.Tick1ms)
        {
            SysTick_c.Tick1ms = false;
        }

        if (SysTick_c.Tick100ms)
        {
            SysTick_c.Tick100ms = false;

            Switch_Device = SW_IC_1;
            SW3516H_Enable_I2C_Write();
            Switch_Device = SW_IC_2;
            SW3516H_Enable_I2C_Write();

            check_Samsung();

            check_Port();
            change_Power();

            cycle_check(); // 周期检查芯片值是否正常

            Check_Curr_After_3S();
        }
    }
}
/*
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{

    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    //配置为内部高速24M
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HIRC;
    RCC_OscInitStruct.HIRCState = RCC_HIRC_ON;
    RCC_OscInitStruct.HIRCCalibrationValue = RCC_HIRCCALIBRATION_4M;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    //初始化CPU、AHB和APB总线时钟     Initializes the CPU, AHB and APB busses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HIRC;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APBCLKDivider = RCC_PCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
}

/*
 * @brief  滑动平均
 * @param  ADVal: 当前采样的AD值
 * @param  pADSum: 累加和指针
 * @param  ADAverage: 上一次平滑滤波的平均
 * @param  Times: 平滑滤波的次数
 * @retval 本次平滑滤波的平均值
 */
uint16_t Slide_Average(uint16_t ADVal, uint32_t *pADSum, uint16_t ADAverage)
{
    if (*pADSum == 0)
        *pADSum = ADVal;

    if (*pADSum > ADAverage)
        *pADSum -= ADAverage;
    else
        *pADSum = 0;

    *pADSum += ADVal;
    // return (*pADSum) / Times;
    return (*pADSum) >> 2; //除4
}

/*
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/*
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
       tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/* Private function -------------------------------------------------------*/
