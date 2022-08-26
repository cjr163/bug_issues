/**
  ******************************************************************************
  * @file    cs32l010_hal.c
	* @author  MCU Software Team
	* @Version V1.0.0
  * @Date    21-Oct-2019
  * @brief   HAL module driver.
  *          This is the common part of the HAL initialization
  *
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
    [..]
    The common HAL driver contains a set of generic and common APIs that can be
    used by the PPP peripheral drivers and the user to start using the HAL.
    [..]
    The HAL contains two APIs' categories:
         (+) Common HAL APIs
         (+) Services HAL APIs

  @endverbatim
  ******************************************************************************

  */

/* Includes ------------------------------------------------------------------*/
#include "cs32l010_hal.h"


/** @addtogroup CS32L010_HAL_Driver
  * @{
  */


/** @defgroup HAL HAL
  * @brief HAL module driver.
  * @{
  */

#ifdef HAL_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @defgroup HAL_Private_Constants HAL Private Constants
  * @{
  */
/**
 * @brief CS32L010 HAL Driver version number V1.0.0
   */
#define __CS32L010_HAL_VERSION_MAIN   (0x01U) /*!< [31:24] main version */
#define __CS32L010_HAL_VERSION_SUB1   (0x00U) /*!< [23:16] sub1 version */
#define __CS32L010_HAL_VERSION_SUB2   (0x00U) /*!< [15:8]  sub2 version */
#define __CS32L010_HAL_VERSION_RC     (0x00U) /*!< [7:0]  release candidate */
#define __CS32L010_HAL_VERSION         ((__CS32L010_HAL_VERSION_MAIN << 24)\
                                        |(__CS32L010_HAL_VERSION_SUB1 << 16)\
                                        |(__CS32L010_HAL_VERSION_SUB2 << 8 )\
                                        |(__CS32L010_HAL_VERSION_RC))

#define IDCODE_DEVID_MASK    (0x00000FFFU)



/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/** @defgroup HAL_Private_Variables HAL Private Variables
  * @{
  */
__IO uint32_t uwTick;
uint32_t uwTickPrio   = (1UL << __NVIC_PRIO_BITS); /* Invalid PRIO */
HAL_TickFreqTypeDef uwTickFreq = HAL_TICK_FREQ_DEFAULT;  /* 1KHz */

/**
  * @}
  */
/* Private function prototypes -----------------------------------------------*/
/* Exported functions ---------------------------------------------------------*/

/** @defgroup HAL_Exported_Functions HAL Exported Functions
  * @{
  */

/** @defgroup HAL_Exported_Functions_Group1 Initialization and de-initialization Functions
 *  @brief    Initialization and de-initialization functions
 *
@verbatim
 ===============================================================================
              ##### Initialization and de-initialization functions #####
 ===============================================================================
   [..]  This section provides functions allowing to:
      (+) Initializes the Flash interface, the NVIC allocation and initial clock
          configuration. It initializes the systick also when timeout is needed.
      (+) de-Initializes common part of the HAL.
      (+) Configure The time base source to have 1ms time base with a dedicated
          Tick interrupt priority.
        (++) SysTick timer is used by default as source of time base, but user
             can eventually implement his proper time base source (a general purpose
             timer for example or other time source).
        (++) Time base configuration function (HAL_InitTick ()) is called automatically
             at the beginning of the program after reset by HAL_Init() or at any time
             when clock is configured, by HAL_RCC_ClockConfig().
        (++) Source of time base is configured  to generate interrupts at regular
             time intervals. Care must be taken if HAL_Delay() is called from a
             peripheral ISR process, the Tick interrupt line must have higher priority
            (numerically lower) than the peripheral interrupt. Otherwise the caller
            ISR process will be blocked.
       (++) functions affecting time base configurations are declared as __weak
             to make override possible in case of other implementations in user file.
@endverbatim
  * @{
  */

/**
  * @brief  初始化HAL库 它必须是主程序中执行的第一条指令
  * @note   配置SysTick每1毫秒产生一个中断，由HIRC计时(在此阶段，时钟尚未配置，因此系统在4mhz的内部HIRC运行)。
  *         设置“NVIC组优先级”为“4”。
  *         调用用户文件“cs32l010_hal_msp.c”中定义的HAL_MspInit()回调函数来执行全局低级硬件初始化
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_Init(void)
{
  /* Use systick as time base source and configure 1ms tick (default clock after Reset is HSI) */
  HAL_InitTick(TICK_INT_PRIORITY);

  /* Init the low level hardware */
  HAL_MspInit();

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief This function de-Initializes common part of the HAL and stops the systick.
  *        of time base.
  * @note This function is optional.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DeInit(void)
{
  /* Reset of all peripherals */
	__HAL_RCC_CRC_FORCE_RESET();
  __HAL_RCC_CRC_RELEASE_RESET();

  __HAL_RCC_GPIOD_FORCE_RESET();
  __HAL_RCC_GPIOD_RELEASE_RESET();

	__HAL_RCC_GPIOC_FORCE_RESET();
  __HAL_RCC_GPIOC_RELEASE_RESET();

	__HAL_RCC_GPIOB_FORCE_RESET();
  __HAL_RCC_GPIOB_RELEASE_RESET();

	__HAL_RCC_GPIOA_FORCE_RESET();
  __HAL_RCC_GPIOA_RELEASE_RESET();

	__HAL_RCC_DBG_FORCE_RESET();
  __HAL_RCC_DBG_RELEASE_RESET();

	__HAL_RCC_BEEP_FORCE_RESET();
  __HAL_RCC_BEEP_RELEASE_RESET();

	__HAL_RCC_LVDVC_FORCE_RESET();
  __HAL_RCC_LVDVC_RELEASE_RESET();

	__HAL_RCC_CLKTRIM_FORCE_RESET();
  __HAL_RCC_CLKTRIM_RELEASE_RESET();

	__HAL_RCC_AWK_FORCE_RESET();
  __HAL_RCC_AWK_RELEASE_RESET();

	__HAL_RCC_ADC_FORCE_RESET();
  __HAL_RCC_ADC_RELEASE_RESET();

	__HAL_RCC_WWDG_FORCE_RESET();
  __HAL_RCC_WWDG_RELEASE_RESET();

	__HAL_RCC_TIM2_FORCE_RESET();
  __HAL_RCC_TIM2_RELEASE_RESET();

	__HAL_RCC_TIM1_FORCE_RESET();
  __HAL_RCC_TIM1_RELEASE_RESET();

	__HAL_RCC_OWRIE_FORCE_RESET();
  __HAL_RCC_OWRIE_RELEASE_RESET();

	__HAL_RCC_PCA_FORCE_RESET();
  __HAL_RCC_PCA_RELEASE_RESET();

	__HAL_RCC_SYSCON_FORCE_RESET();
  __HAL_RCC_SYSCON_RELEASE_RESET();

	__HAL_RCC_BASETIM_FORCE_RESET();
  __HAL_RCC_BASETIM_RELEASE_RESET();

	__HAL_RCC_LPTIM_FORCE_RESET();
  __HAL_RCC_LPTIM_RELEASE_RESET();

	__HAL_RCC_SPI_FORCE_RESET();
  __HAL_RCC_SPI_RELEASE_RESET();

	__HAL_RCC_LPUART_FORCE_RESET();
  __HAL_RCC_LPUART_RELEASE_RESET();

	__HAL_RCC_I2C_FORCE_RESET();
  __HAL_RCC_I2C_RELEASE_RESET();

	__HAL_RCC_UART1_FORCE_RESET();
  __HAL_RCC_UART1_RELEASE_RESET();

	__HAL_RCC_UART0_FORCE_RESET();
  __HAL_RCC_UART0_RELEASE_RESET();

  /* De-Init the low level hardware */
  HAL_MspDeInit();

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  Initialize the MSP.
  * @retval None
  */
__weak void HAL_MspInit(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_MspInit could be implemented in the user fil;e
   */
}

/**
  * @brief  DeInitializes the MSP.
  * @retval None
  */
__weak void HAL_MspDeInit(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_MspDeInit could be implemented in the user file
   */
}

/**
  * @brief 本功能用于配置时基准的来源。 时间源被配置为具有1ms的时间基数，并具有专用的Tick中断优先级。 SysTick
  * @note 在程序开始时，HAL_Init()重置之后，或者HAL_RCC_ClockConfig()重新配置时钟时，会自动调用此函数。
  * @note 如果从外围ISR调用HAL_Delay()，必须注意。SysTick中断必须具有比外围中断更高的优先级(数字上更低)。
            否则，调用方ISR进程将被阻塞。 该函数被声明为__weak，以便在用户文件中其他实现时被覆盖。
  * @param TickPriority 中断优先级 Tick interrupt priority.
  * @retval HAL status
  */
__weak HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  /* Configure the SysTick to have interrupt in 1ms time basis*/
  if (HAL_SYSTICK_Config(SystemCoreClock / (1000U / uwTickFreq)) > 0U)
  {
    return HAL_ERROR;
  }

  /* Configure the SysTick IRQ priority */
  if (TickPriority < (1UL << __NVIC_PRIO_BITS))
  {
    HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority);
    uwTickPrio = TickPriority;
  }
  else
  {
    return HAL_ERROR;
  }

  /* Return function status */
  return HAL_OK;
}

/**
  * @}
  */ /* End of group HAL_Exported_Functions_Group1 */

/** @defgroup HAL_Exported_Functions_Group2 HAL Control functions
  *  @brief    HAL Control functions
  *
@verbatim
 ===============================================================================
                      ##### HAL Control functions #####
 ===============================================================================
    [..]  This section provides functions allowing to:
      (+) Provide a tick value in millisecond
      (+) Provide a blocking delay in millisecond
      (+) Suspend the time base source interrupt
      (+) Resume the time base source interrupt
      (+) Get the HAL API driver version
      (+) Get the device identifier
      (+) Get the device revision identifier
      (+) Enable/Disable Debug module during SLEEP mode
      (+) Enable/Disable Debug module during STOP mode
      (+) Enable/Disable Debug module during STANDBY mode

@endverbatim
  * @{
  */

/**
  * @brief This function is called to increment a global variable "uwTick"
  *        used as application time base.
  * @note In the default implementation, this variable is incremented each 1ms
  *       in SysTick ISR.
  * @note This function is declared as __weak to be overwritten in case of other
  *      implementations in user file.
  * @retval None
  */
__weak void HAL_IncTick(void)
{
  uwTick += uwTickFreq;
}

/**
  * @brief Provides a tick value in millisecond.
  * @note  This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval tick value
  */
__weak uint32_t HAL_GetTick(void)
{
  return uwTick;
}

/**
  * @brief This function returns a tick priority.
  * @retval tick priority
  */
uint32_t HAL_GetTickPrio(void)
{
  return uwTickPrio;
}

/**
  * @brief Set new tick Freq.
  * @retval Status
  */
HAL_StatusTypeDef HAL_SetTickFreq(HAL_TickFreqTypeDef Freq)
{
  HAL_StatusTypeDef status  = HAL_OK;
  assert_param(IS_TICKFREQ(Freq));

  if (uwTickFreq != Freq)
  {
    uwTickFreq = Freq;

    /* Apply the new tick Freq  */
    status = HAL_InitTick(uwTickPrio);
  }

  return status;
}

/**
  * @brief Return tick frequency.
  * @retval tick period in Hz
  */
HAL_TickFreqTypeDef HAL_GetTickFreq(void)
{
  return uwTickFreq;
}

/*
  * @brief This function provides minimum delay (in milliseconds) based
  *        on variable incremented.
  * @note In the default implementation, SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals where uwTick
  *       is incremented.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @param Delay specifies the delay time length, in milliseconds.
  * @retval None
  */
__weak void HAL_Delay(uint32_t Delay)
{
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Delay;

  /* Add a freq to guarantee minimum wait */
  if (wait < HAL_MAX_DELAY)
  {
    wait += (uint32_t)(uwTickFreq);
  }

  while ((HAL_GetTick() - tickstart) < wait)
  {
  }
}

/**
  * @brief Suspend Tick increment.
  * @note In the default implementation , SysTick timer is the source of time base. It is
  *       used to generate interrupts at regular time intervals. Once HAL_SuspendTick()
  *       is called, the SysTick interrupt will be disabled and so Tick increment
  *       is suspended.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval None
  */
__weak void HAL_SuspendTick(void)
{
  /* Disable SysTick Interrupt */
  CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

/**
  * @brief Resume Tick increment.
  * @note In the default implementation, SysTick timer is the source of time base. It is
  *       used to generate interrupts at regular time intervals. Once HAL_ResumeTick()
  *       is called, the SysTick interrupt will be enabled and so Tick increment
  *       is resumed.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval None
  */
__weak void HAL_ResumeTick(void)
{
  /* Enable SysTick Interrupt */
  SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

/**
  * @brief  Returns the HAL revision
  * @retval version 0xXYZR (8bits for each decimal, R for RC)
  */
uint32_t HAL_GetHalVersion(void)
{
  return __CS32L010_HAL_VERSION;
}

/**
  * @brief Returns the device revision identifier.
  * Note:
  *       Address 0x180000F0-0x180000FF are unique ID for CS32L010.
  * 			Refer to function HAL_GetUID(uint32_t *UID)
  * @retval Device revision identifier address, 16 bytes(4 words)
  */
uint32_t HAL_GetREVID(void)
{
  return (0);
}

/**
  * @brief  Returns the device identifier.
  * Note:
  *       Address 0x180000F0-0x180000FF are unique ID for CS32L010.
  * 			Refer to function HAL_GetUID(uint32_t *UID)
  * @retval Device identifier
  */
uint32_t HAL_GetDEVID(void)
{
  return (0);
}

/**
  * @brief  Enable the Debug Module during SLEEP mode
  * @retval None
  */
void HAL_DBGMCU_EnableDBGSleepMode(void)
{
	__HAL_SYSCON_DBGDEEPSLEEP_ENABLE();
}

/**
  * @brief  Disable the Debug Module during SLEEP mode
  * Note:
  * @retval None
  */
void HAL_DBGMCU_DisableDBGSleepMode(void)
{
	__HAL_SYSCON_DBGDEEPSLEEP_DISABLE();
}


/**
  * @brief Return the unique device identifier (UID based on 128 bits)
  * @param UID pointer to 4 words array.
  * Note:
  *       Address 0x180000F0-0x180000FF are unique ID for CS32L010.
  * @retval Device identifier
  */
void HAL_GetUID(uint32_t *UID)
{
  UID[0] = (uint32_t)(READ_REG(*((uint32_t *)UID_BASE)));
  UID[1] = (uint32_t)(READ_REG(*((uint32_t *)(UID_BASE + 4U))));
  UID[2] = (uint32_t)(READ_REG(*((uint32_t *)(UID_BASE + 8U))));
  UID[3] = (uint32_t)(READ_REG(*((uint32_t *)(UID_BASE + 12U))));
}

/**
  * @}
  */ /* End of group HAL_Exported_Functions_Group2 */

/**
  * @}
  */ /* End of group HAL_Exported_Functions */

#endif /* HAL_MODULE_ENABLED */
/**
  * @}
  */ /* End of group HAL */

/**
  * @}
  */ /* group CS32L010_HAL_Driver */

