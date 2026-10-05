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
#define BOOT_TIME 10 // ms

/* Private variables ---------------------------------------------------------*/
static uint8_t whoamI;

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void platform_delay(uint32_t ms);

static volatile uint8_t thread_wake = 0;

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len) {
  if (HAL_I2C_MASTER_MemWrite((hal_i2c_handle_t *)handle, LSM6DSR_I2C_ADD_L,
                              reg, HAL_I2C_MEM_ADDR_8BIT, bufp, len, 1000) != HAL_OK) {
    return -1;
  }
  return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len) {
  if (HAL_I2C_MASTER_MemRead((hal_i2c_handle_t *)handle, LSM6DSR_I2C_ADD_L,
                             reg, HAL_I2C_MEM_ADDR_8BIT, bufp, len, 1000) != HAL_OK) {
    return -1;
  }
  return 0;
}

static void platform_delay(uint32_t ms) {
  HAL_Delay(ms);
}

void HAL_EXTI_TriggerCallback(hal_exti_handle_t *hexti,
                              hal_exti_trigger_t trigger)
{
    if (HAL_EXTI_GetInstance(hexti) == HAL_EXTI_GPIO_0)
    {
        thread_wake = 1;
    }
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

    stmdev_ctx_t dev_ctx;
    lsm6dsr_pin_int1_route_t pin_int = {0};  /* 字段: md1_cfg.int1_single_tap/int1_double_tap */

	  /* Initialize mems driver interface */
	  dev_ctx.write_reg = platform_write;
	  dev_ctx.read_reg = platform_read;
	  dev_ctx.mdelay = platform_delay;
	  dev_ctx.handle = mx_i2c1_i2c_gethandle();

	  /* Init test platform */
      // platform_init(dev_ctx.handle);

	  /* Wait sensor boot time */
	  platform_delay(BOOT_TIME);

	  /* Check device ID */
	  lsm6dsr_device_id_get(&dev_ctx, &whoamI);
	  printf("LSM6DSR_ID=0x%x,id=0x%x\n",LSM6DSR_ID,whoamI);
	  if (whoamI != LSM6DSR_ID)
	    while (1);

	  /* Restore default configuration */
	  lsm6dsr_reset_set(&dev_ctx, PROPERTY_ENABLE);
	  platform_delay(BOOT_TIME);

	  /* Disable I3C interface */
	  lsm6dsr_i3c_disable_set(&dev_ctx, LSM6DSR_I3C_DISABLE);

	  /* Enable Block Data Update */
	  lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

	  /* Set Output Data Rate.
	   * Tap recognition requires the accelerometer ODR to be equal or
	   * greater than 416 Hz.
	   */
	  lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_416Hz);
	  lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_12Hz5);

	  /* Set full scale.
	   * 2g gives the finest tap threshold resolution (FS/32 = 62.5 mg/LSB),
	   * which greatly improves tap detection sensitivity.
	   */
	  lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g);
	  lsm6dsr_gy_full_scale_set(&dev_ctx, LSM6DSR_2000dps);

	  /* Accelerometer in high-performance mode for full tap bandwidth */
	  lsm6dsr_xl_power_mode_set(&dev_ctx, LSM6DSR_HIGH_PERFORMANCE_MD);

    /* INT1/INT2: open-drain + active low (matches external pull-up + PB0 falling edge) */
    lsm6dsr_pin_mode_set(&dev_ctx, LSM6DSR_OPEN_DRAIN);
    lsm6dsr_pin_polarity_set(&dev_ctx, LSM6DSR_ACTIVE_LOW);

      /* Enable Z-axis Tap detection */
      lsm6dsr_tap_detection_on_x_set(&dev_ctx, PROPERTY_DISABLE);
      lsm6dsr_tap_detection_on_y_set(&dev_ctx, PROPERTY_DISABLE);
      lsm6dsr_tap_detection_on_z_set(&dev_ctx, PROPERTY_ENABLE);

      /* Set Tap threshold (1 LSB = FS/32 = 62.5 mg at 2g)
       * 0x02 -> 125 mg, 4x more sensitive than the ST example (0x08 = 500 mg)
       */
      lsm6dsr_tap_threshold_x_set(&dev_ctx, 0);
      lsm6dsr_tap_threshold_y_set(&dev_ctx, 0);
      lsm6dsr_tap_threshold_z_set(&dev_ctx, 0x08);

      /* Set Tap time windows (ST example values): shock / quiet / double-tap gap */
      lsm6dsr_tap_shock_set(&dev_ctx, 0x03);
      lsm6dsr_tap_quiet_set(&dev_ctx, 0x03);
      lsm6dsr_tap_dur_set(&dev_ctx, 0x07);

      /* Enable Single Tap and Double Tap */
      lsm6dsr_tap_mode_set(&dev_ctx, LSM6DSR_BOTH_SINGLE_DOUBLE);

      /* Route Single Tap and Double Tap to INT1 */
      pin_int.md1_cfg.int1_single_tap = PROPERTY_ENABLE;
      pin_int.md1_cfg.int1_double_tap = PROPERTY_ENABLE;
      lsm6dsr_pin_int1_route_set(&dev_ctx, &pin_int);

      /* All interrupt signals notification mode: pulsed.
       * Pulsed is used instead of latched: with a latched active-low INT1 an
       * event latched during power-up would hold the line low and no further
       * falling edge would ever wake the MCU.
       */
      lsm6dsr_int_notification_set(&dev_ctx, LSM6DSR_ALL_INT_PULSED);

      /* Let the tap high-pass filter and accelerometer output settle,
       * then discard the false single-tap generated at start-up. */
      platform_delay(200);

      {
        lsm6dsr_all_sources_t clr = {0};
        lsm6dsr_all_sources_get(&dev_ctx, &clr);
      }

      /* Drop the EXTI edge latched while the sensor was being configured */
      HAL_EXTI_ClearPending(mx_gpio_default_exti0_gethandle(),
                            HAL_EXTI_TRIGGER_FALLING);
      thread_wake = 0;

	  while (1) {

		if (thread_wake)
		{
		  lsm6dsr_all_sources_t status = {0};

		  thread_wake = 0;

		  /* Read interrupt source */
		  lsm6dsr_all_sources_get(&dev_ctx, &status);

		  if (status.tap_src.double_tap)
		  {
			  printf("%s\r\n", "Double Tap");
		  }
		  else if (status.tap_src.single_tap)
		  {
			  printf("%s\r\n", "Single Tap");
		  }
	    }
	  }
  }
} /* end main */


