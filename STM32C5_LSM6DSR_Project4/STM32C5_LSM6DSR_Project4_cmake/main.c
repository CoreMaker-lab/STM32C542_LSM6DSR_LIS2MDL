/**
  ******************************************************************************
  * file           : main.c
  * brief          : Main program body
  *                  Calls target system initialization then loop in main.
  ******************************************************************************
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private functions prototype -----------------------------------------------*/

#include "mx_usart1.h"
#include <stdio.h>
#include <string.h>

#include "lsm6dsr_reg.h"

int _write(int file, char *ptr, int len)
{
    hal_uart_handle_t *huart1 = mx_usart1_uart_gethandle();

    if (huart1 != NULL)
    {
        HAL_UART_Transmit(huart1, ptr, len, 1000);
    }

    return len;
}
/* Private macro -------------------------------------------------------------*/
#define    BOOT_TIME            10 //ms
/* Private variables ---------------------------------------------------------*/
static uint8_t whoamI;
static uint8_t rst;
/* Extern variables ----------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);
static void platform_delay(uint32_t ms);
static stmdev_ctx_t dev_ctx;
static volatile uint8_t thread_wake = 0;
/* STM32 HAL expects the 8-bit I2C address.
 * SA0 = 0 -> 0xD5 (see LSM6DSR_I2C_ADD_L). */
#define LSM6DSR_I2C_ADD  LSM6DSR_I2C_ADD_L

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
  if (HAL_I2C_MASTER_MemWrite((hal_i2c_handle_t *)handle, LSM6DSR_I2C_ADD, reg,
                              HAL_I2C_MEM_ADDR_8BIT, bufp, len, 1000) != HAL_OK)
    return -1;
  return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
  /* NOTE: the 0x80 read bit is only required on the SPI interface.
   * On I2C the register address must be sent as-is. */
  if (HAL_I2C_MASTER_MemRead((hal_i2c_handle_t *)handle, LSM6DSR_I2C_ADD, reg,
                             HAL_I2C_MEM_ADDR_8BIT, bufp, len, 1000) != HAL_OK)
    return -1;
  return 0;
}

static void platform_delay(uint32_t ms) {
  HAL_Delay(ms);
}

/* EXTI line (INT1 on PB0) trigger callback: wake up the main loop. */
void HAL_EXTI_TriggerCallback(hal_exti_handle_t *hexti, hal_exti_trigger_t trigger)
{
  (void)hexti;
  (void)trigger;
  thread_wake = 1;
}

/**
  * brief:  The application entry point.
  * retval: none but we specify int to comply with C99 standard
  */
int main(void)
{
  /** System Init: this code placed in targets folder initializes your system.
    * It calls the initialization (and sets the initial configuration) of the peripherals.
    * You can use STM32CubeMX to generate and call this code or not in this project.
    * It also contains the HAL initialization and the initial clock configuration.
    */
  if (mx_system_init() != SYSTEM_OK)
  {
    return (-1);
  }
  else
  {
    /*
      * You can start your application code here
      */

	  printf("HELLO\n");
	  HAL_GPIO_WritePin(CS1_PORT, CS1_PIN, HAL_GPIO_PIN_SET);
	  HAL_GPIO_WritePin(SA0_PORT, SA0_PIN, HAL_GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(CS2_PORT, CS2_PIN, HAL_GPIO_PIN_SET);

      lsm6dsr_pedo_mode_t pedo_mode = LSM6DSR_PEDO_ADV_FALSE_STEP_REJ;
      lsm6dsr_pin_int1_route_t pin_int1_route = {0};

      uint8_t debounce = 3;
      uint16_t step_count = 0;

      /* Initialize mems driver interface */
      dev_ctx.write_reg = platform_write;
      dev_ctx.read_reg = platform_read;
      dev_ctx.mdelay = platform_delay;
      dev_ctx.handle = mx_i2c1_i2c_gethandle();

      /* Wait sensor boot time */
      platform_delay(BOOT_TIME);

      /* Check device ID */
      lsm6dsr_device_id_get(&dev_ctx, &whoamI);

      printf("LSM6DSR_ID=0x%x,id=0x%x\n",
             LSM6DSR_ID, whoamI);

      if (whoamI != LSM6DSR_ID)
        while (1);

      /* Perform device power-on-reset */
      lsm6dsr_reset_set(&dev_ctx, PROPERTY_ENABLE);

      do {
        lsm6dsr_reset_get(&dev_ctx, &rst);
      } while (rst);

      /* Disable I3C interface */
      lsm6dsr_i3c_disable_set(&dev_ctx, LSM6DSR_I3C_DISABLE);

      /* Enable Block Data Update */
      lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

      /*
       * Pedometer works internally at 26 Hz.
       * Accelerometer ODR must be >= 26 Hz.
       * Use 52 Hz to feed the algorithm with a faster data stream.
       */
      lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_52Hz);
      lsm6dsr_xl_power_mode_set(&dev_ctx, LSM6DSR_HIGH_PERFORMANCE_MD);

      /* Set accelerometer full scale */
      lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g);

      /* Enable latched interrupt notification */
	  lsm6dsr_int_notification_set(&dev_ctx, LSM6DSR_ALL_INT_LATCHED); // LSM6DSR 的所有中断信号配置为 锁存模式
      /* INT1: open-drain + active low (MCU PB0 uses its internal pull-up) */
      lsm6dsr_pin_mode_set(&dev_ctx, LSM6DSR_OPEN_DRAIN);
      lsm6dsr_pin_polarity_set(&dev_ctx, LSM6DSR_ACTIVE_LOW);

      /* Enable pedometer and select working mode */
      lsm6dsr_pedo_sens_set(&dev_ctx, PROPERTY_ENABLE);
      lsm6dsr_pedo_mode_set(&dev_ctx, pedo_mode);

      /*
       * Set pedometer debounce.
       *
       * Default value = 10 steps.
       * Set to 3 steps for easier demonstration.
       */
      lsm6dsr_pedo_debounce_steps_set(&dev_ctx, &debounce);

      /* Reset step counter */
      lsm6dsr_steps_reset(&dev_ctx);

      /* Route Step Detector event to INT1 */
      lsm6dsr_pin_int1_route_get(&dev_ctx, &pin_int1_route);
      pin_int1_route.emb_func_int1.int1_step_detector = PROPERTY_ENABLE;
      lsm6dsr_pin_int1_route_set(&dev_ctx, &pin_int1_route);

      printf("Pedometer start...\r\n");

      while (1) {

          if (thread_wake)
          {
            uint8_t step_det = 0;

            thread_wake = 0;

            /* Read interrupt source (also clears latched status) */
            lsm6dsr_pedo_step_detect_get(&dev_ctx,
                                         &step_det);

            /* Check Step Detector event */
            if (step_det)
            {
              /* Read Step Counter */
              lsm6dsr_number_of_steps_get(&dev_ctx,
                                          &step_count);

              printf("Step detected, Steps = %d\r\n",
                     step_count);
            }
          }
      }
  }
} /* end main */

