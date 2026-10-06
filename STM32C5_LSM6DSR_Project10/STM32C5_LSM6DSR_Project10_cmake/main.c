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

#define BOOT_TIME 10U
#define BOARD_EXPECTED_ID LSM6DSR_ID
#define FIFO_WATERMARK 128U

static stmdev_ctx_t dev_ctx;
static volatile uint8_t thread_wake = 0;
static int32_t platform_write(void *handle,uint8_t reg,const uint8_t *bufp,uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg,uint8_t *bufp,uint16_t len);
static void platform_delay(uint32_t ms);

/**
 * @brief  EXTI trigger callback
 */
void HAL_EXTI_TriggerCallback(hal_exti_handle_t *hexti, hal_exti_trigger_t trigger) {
    (void)trigger;
    if (HAL_EXTI_GetInstance(hexti) == HAL_EXTI_GPIO_0)
    {
        thread_wake = 1;
    }
}

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions prototype -----------------------------------------------*/

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

    	    printf("HELLO\r\n");

	    HAL_GPIO_WritePin(CS1_PORT, CS1_PIN, HAL_GPIO_PIN_SET);
	    HAL_GPIO_WritePin(SA0_PORT, SA0_PIN, HAL_GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(CS2_PORT, CS2_PIN, HAL_GPIO_PIN_SET);

	    uint8_t whoamI = 0;
	    lsm6dsr_pin_int1_route_t pin_int = {0};

	    /* Initialize driver interface. */
	    dev_ctx.write_reg = platform_write;
	    dev_ctx.read_reg = platform_read;
	    dev_ctx.mdelay = platform_delay;
	    dev_ctx.handle = mx_i2c1_i2c_gethandle();

	    platform_delay(BOOT_TIME);

	    /* Check device ID. */
	    lsm6dsr_device_id_get(&dev_ctx, &whoamI);

	    printf("LSM6DSR_ID=0x%x,id=0x%x\r\n",
	           (unsigned int)BOARD_EXPECTED_ID, (unsigned int)whoamI);

	    if (whoamI != BOARD_EXPECTED_ID)
	        while (1);

	    /* Reset device. */
	    lsm6dsr_reset_set(&dev_ctx, PROPERTY_ENABLE);
	    platform_delay(BOOT_TIME);

	    /* Enable Block Data Update. */
	    lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

	    /* INT1: open-drain + active low. */
	    lsm6dsr_pin_mode_set(&dev_ctx, LSM6DSR_OPEN_DRAIN);
	    lsm6dsr_pin_polarity_set(&dev_ctx, LSM6DSR_ACTIVE_LOW);

      /* Set full scales before starting the sensors. */
      lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g);
      lsm6dsr_gy_full_scale_set(&dev_ctx, LSM6DSR_2000dps);

      /* Enable accelerometer and gyroscope low-pass filters. */
      lsm6dsr_xl_filter_lp2_set(&dev_ctx, PROPERTY_ENABLE);
      lsm6dsr_gy_filter_lp1_set(&dev_ctx, PROPERTY_ENABLE);

      /* Bypass clears FIFO. Watermark counts TAG + 6-byte records,
       * not pairs of accelerometer and gyroscope samples. */
      lsm6dsr_fifo_mode_set(&dev_ctx, LSM6DSR_BYPASS_MODE);
      lsm6dsr_fifo_watermark_set(&dev_ctx, FIFO_WATERMARK);

      /* Batch accelerometer and gyroscope at 52 Hz each. */
      lsm6dsr_fifo_xl_batch_set(&dev_ctx, LSM6DSR_XL_BATCHED_AT_52Hz);
      lsm6dsr_fifo_gy_batch_set(&dev_ctx, LSM6DSR_GY_BATCHED_AT_52Hz);

      /* Route FIFO watermark to INT1. No basic-event latch is needed. */
      pin_int.int1_ctrl.int1_fifo_th = PROPERTY_ENABLE;
      lsm6dsr_pin_int1_route_set(&dev_ctx, &pin_int);

      /* Continuous FIFO: oldest unread data is overwritten when full. */
      lsm6dsr_fifo_mode_set(&dev_ctx, LSM6DSR_STREAM_MODE);

      /* Start accelerometer and gyroscope at 52 Hz. */
      lsm6dsr_xl_power_mode_set(&dev_ctx, LSM6DSR_HIGH_PERFORMANCE_MD);
      lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_52Hz);
      lsm6dsr_gy_power_mode_set(&dev_ctx, LSM6DSR_GY_HIGH_PERFORMANCE);
      lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_52Hz);

      while (1)
      {
          if (thread_wake)
          {
              lsm6dsr_fifo_status2_t status = {0};
              float xl_sum[3] = {0};
              float gy_sum[3] = {0};
              uint16_t xl_count = 0;
              uint16_t gy_count = 0;
              uint16_t num;

              thread_wake = 0;

              /* Reading status alone does not clear a watermark condition. */
              if (lsm6dsr_fifo_status_get(&dev_ctx, &status) != 0)
              {
                  thread_wake = 1;
                  platform_delay(100);
                  continue;
              }

              if (status.fifo_ovr_ia)
                  printf("FIFO overrun: samples lost\r\n");

              if (lsm6dsr_fifo_data_level_get(&dev_ctx, &num) != 0)
              {
                  thread_wake = 1;
                  platform_delay(100);
                  continue;
              }

              /* Read the records already present in this FIFO snapshot. */
              while (num > 0)
              {
                  lsm6dsr_fifo_tag_t tag = {0};
                  uint8_t fifo_data[6] = {0};
                  int16_t raw[3];

                  if (lsm6dsr_fifo_sensor_tag_get(&dev_ctx, &tag) != 0)
                  {
                      thread_wake = 1;
                      platform_delay(100);
                      break;
                  }

                  if (lsm6dsr_fifo_out_raw_get(&dev_ctx, fifo_data) != 0)
                  {
                      thread_wake = 1;
                      platform_delay(100);
                      break;
                  }
                  num--;

                  /* Decode little-endian axes without unaligned pointers. */
                  raw[0] = (int16_t)((uint16_t)fifo_data[0] |
                                    ((uint16_t)fifo_data[1] << 8));
                  raw[1] = (int16_t)((uint16_t)fifo_data[2] |
                                    ((uint16_t)fifo_data[3] << 8));
                  raw[2] = (int16_t)((uint16_t)fifo_data[4] |
                                    ((uint16_t)fifo_data[5] << 8));

                  switch (tag)
                  {
                      case LSM6DSR_XL_NC_TAG:
                          xl_sum[0] += lsm6dsr_from_fs2g_to_mg(raw[0]);
                          xl_sum[1] += lsm6dsr_from_fs2g_to_mg(raw[1]);
                          xl_sum[2] += lsm6dsr_from_fs2g_to_mg(raw[2]);
                          xl_count++;
                          break;

                      case LSM6DSR_GYRO_NC_TAG:
                          gy_sum[0] += lsm6dsr_from_fs2000dps_to_mdps(raw[0]);
                          gy_sum[1] += lsm6dsr_from_fs2000dps_to_mdps(raw[1]);
                          gy_sum[2] += lsm6dsr_from_fs2000dps_to_mdps(raw[2]);
                          gy_count++;
                          break;

                      default:
                          break;
                  }
              }

              /* Print only after reading the batch to limit UART overhead.
                * These are batch averages, not individual FIFO samples. */
              if (xl_count > 0)
              {
                  printf("XL avg (%u) [mg]: %.2f, %.2f, %.2f\r\n",
                          (unsigned int)xl_count,
                          (double)(xl_sum[0] / xl_count),
                          (double)(xl_sum[1] / xl_count),
                          (double)(xl_sum[2] / xl_count));
              }

              if (gy_count > 0)
              {
                  printf("GY avg (%u) [mdps]: %.2f, %.2f, %.2f\r\n\r\n",
                          (unsigned int)gy_count,
                          (double)(gy_sum[0] / gy_count),
                          (double)(gy_sum[1] / gy_count),
                          (double)(gy_sum[2] / gy_count));
              }

              /* New samples arrived during reads/printing. If still above
                * watermark, keep servicing: INT1 may remain active low and
                * no additional falling edge would be generated. */
              if (lsm6dsr_fifo_status_get(&dev_ctx, &status) != 0)
              {
                  thread_wake = 1;
                  platform_delay(100);
              }
              else if (status.fifo_wtm_ia)
              {
                  thread_wake = 1;
              }
          }
      }
  }
} /* end main */

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len) {
  if (HAL_I2C_MASTER_MemWrite((hal_i2c_handle_t *)handle, LSM6DSR_I2C_ADD_L, reg,
                              HAL_I2C_MEM_ADDR_8BIT, bufp, len, 1000) != HAL_OK)
    return -1;
  return 0;
}

static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len) {
  /* NOTE: the 0x80 read bit is only required on the SPI interface.
   * On I2C the register address must be sent as-is. */
  if (HAL_I2C_MASTER_MemRead((hal_i2c_handle_t *)handle, LSM6DSR_I2C_ADD_L, reg,
                             HAL_I2C_MEM_ADDR_8BIT, bufp, len, 1000) != HAL_OK)
    return -1;
  return 0;
}

static void platform_delay(uint32_t ms) {
  HAL_Delay(ms);
}


