/**
  ******************************************************************************
  * @file           : mx_gpio_default.c
  * @brief          : gpio_default Peripheral initialization
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the mx_stm32c5xx_hal_drivers_license.md file
  * in the same directory as the generated code.
  * If no mx_stm32c5xx_hal_drivers_license.md file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mx_gpio_default.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported variables by reference -------------------------------------------*/
static hal_exti_handle_t hEXTI0;

/******************************************************************************/
/* Exported functions for GPIO in HAL layer                                   */
/******************************************************************************/
system_status_t mx_gpio_default_init(void)
{
  hal_gpio_config_t  gpio_config;

  HAL_RCC_GPIOA_EnableClock();

  HAL_RCC_GPIOB_EnableClock();

  /*
    GPIO pin labels :
    PA1   ---------> SA0
    PA6   ---------> CS2
    */
  /* Configure PA1, PA6 GPIO pins in output mode */
  gpio_config.mode            = HAL_GPIO_MODE_OUTPUT;
  gpio_config.speed           = HAL_GPIO_SPEED_FREQ_LOW;
  gpio_config.pull            = HAL_GPIO_PULL_NO;
  gpio_config.output_type     = HAL_GPIO_OUTPUT_PUSHPULL;
  gpio_config.init_state      = HAL_GPIO_PIN_RESET;
  if (HAL_GPIO_Init(HAL_GPIOA, SA0_PIN | CS2_PIN, &gpio_config) != HAL_OK)
  {
    return SYSTEM_PERIPHERAL_ERROR;
  }

  /* Configure PB0 GPIO pin in input mode */
  gpio_config.mode            = HAL_GPIO_MODE_INPUT;
  gpio_config.pull            = HAL_GPIO_PULL_NO;
  if (HAL_GPIO_Init(HAL_GPIOB, HAL_GPIO_PIN_0, &gpio_config) != HAL_OK)
  {
    return SYSTEM_PERIPHERAL_ERROR;
  }

  hal_exti_config_t exti_config;

  /* Initialize the EXTI for line 0 */
  HAL_EXTI_Init(&hEXTI0, HAL_EXTI_LINE_0);

  /* Set the trigger as FALLING for the GPIOB */
  exti_config.trigger   = HAL_EXTI_TRIGGER_FALLING;
  exti_config.gpio_port = HAL_EXTI_GPIOB;
  HAL_EXTI_SetConfig(&hEXTI0, &exti_config);

  /* Enable the INTERRUPT mode */
  HAL_EXTI_Enable(&hEXTI0, HAL_EXTI_MODE_INTERRUPT);

  /* Set line 0 Interrupt priority */
  HAL_CORTEX_NVIC_SetPriority(EXTI0_IRQn, HAL_CORTEX_NVIC_PREEMP_PRIORITY_0, HAL_CORTEX_NVIC_SUB_PRIORITY_0);
  HAL_CORTEX_NVIC_EnableIRQ(EXTI0_IRQn);

  /*
    GPIO pin labels :
    PB2   ---------> CS1
    */
  /* Configure PB2 GPIO pin in output mode */
  gpio_config.mode            = HAL_GPIO_MODE_OUTPUT;
  gpio_config.speed           = HAL_GPIO_SPEED_FREQ_LOW;
  gpio_config.pull            = HAL_GPIO_PULL_NO;
  gpio_config.output_type     = HAL_GPIO_OUTPUT_PUSHPULL;
  gpio_config.init_state      = CS1_INIT_STATE;
  if (HAL_GPIO_Init(CS1_PORT, CS1_PIN, &gpio_config) != HAL_OK)
  {
    return SYSTEM_PERIPHERAL_ERROR;
  }

  return SYSTEM_OK;
}

system_status_t mx_gpio_default_deinit(void)
{
  /* De-initialize the EXTI for GPIOB line0 */
  HAL_EXTI_DeInit(&hEXTI0);

  /* set line 0 Interrupt priority */
  HAL_CORTEX_NVIC_DisableIRQ(EXTI0_IRQn);

  /* De-initialize pins of GPIOA port */
  HAL_GPIO_DeInit(HAL_GPIOA, SA0_PIN | CS2_PIN);

  /* De-initialize pins of GPIOB port */
  HAL_GPIO_DeInit(HAL_GPIOB, HAL_GPIO_PIN_0 | CS1_PIN);

  return SYSTEM_OK;
}

hal_exti_handle_t *mx_gpio_default_exti0_gethandle(void)
{
  return &hEXTI0;
}

/******************************************************************************/
/*                            EXTI Line0 interrupt                            */
/******************************************************************************/
void EXTI0_IRQHandler(void)
{
  HAL_EXTI_IRQHandler(&hEXTI0);
}
