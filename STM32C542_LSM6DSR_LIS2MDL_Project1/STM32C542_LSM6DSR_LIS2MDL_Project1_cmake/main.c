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
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
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
static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static int16_t data_raw_temperature;
static float_t acceleration_mg[3];
static float_t angular_rate_mdps[3];
static float_t temperature_degC;
static uint8_t whoamI, rst;
static uint8_t tx_buffer[1000];

/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void tx_com( uint8_t *tx_buffer, uint16_t len );
static void platform_delay(uint32_t ms);
static void platform_init(void *handle);


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
	  /* Initialize mems driver interface */
	  dev_ctx.write_reg = platform_write;
	  dev_ctx.read_reg = platform_read;
	  dev_ctx.mdelay = platform_delay;
	  dev_ctx.handle = mx_i2c1_i2c_gethandle();
	  /* Init test platform */
//	  platform_init(dev_ctx.handle);

	  /* Wait sensor boot time */
	  platform_delay(BOOT_TIME);
	  /* Check device ID */
	  lsm6dsr_device_id_get(&dev_ctx, &whoamI);
	  printf("LSM6DSR_ID=0x%x,id=0x%x\n",LSM6DSR_ID,whoamI);
	  if (whoamI != LSM6DSR_ID)
	    while (1);

	  /* Restore default configuration */
	  lsm6dsr_reset_set(&dev_ctx, PROPERTY_ENABLE);

	  do {
	    lsm6dsr_reset_get(&dev_ctx, &rst);
	  } while (rst);

	  /* Enable Block Data Update */
	  lsm6dsr_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);


	  /* Set Output Data Rate */
	  lsm6dsr_xl_data_rate_set(&dev_ctx, LSM6DSR_XL_ODR_12Hz5);
	  lsm6dsr_gy_data_rate_set(&dev_ctx, LSM6DSR_GY_ODR_12Hz5);
	  /* Set full scale */
	  lsm6dsr_xl_full_scale_set(&dev_ctx, LSM6DSR_2g);
	  lsm6dsr_gy_full_scale_set(&dev_ctx, LSM6DSR_2000dps);

	  /* Configure filtering chain(No aux interface)
	   * Accelerometer - LPF1 + LPF2 path
	   */
	  lsm6dsr_xl_hp_path_on_out_set(&dev_ctx, LSM6DSR_LP_ODR_DIV_100);
	  lsm6dsr_xl_filter_lp2_set(&dev_ctx, PROPERTY_ENABLE);

    while (1) {

        uint8_t reg;
        /* Read output only if new xl value is available */
        lsm6dsr_xl_flag_data_ready_get(&dev_ctx, &reg);

        if (reg) {
          /* Read acceleration field data */
          memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
          lsm6dsr_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
          acceleration_mg[0] =
            lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[0]);
          acceleration_mg[1] =
            lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[1]);
          acceleration_mg[2] =
            lsm6dsr_from_fs2g_to_mg(data_raw_acceleration[2]);
          printf("Acceleration [mg]:%4.2f\t%4.2f\t%4.2f\r\n",
                  acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);
        }

        lsm6dsr_gy_flag_data_ready_get(&dev_ctx, &reg);

        if (reg) {
          /* Read angular rate field data */
          memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
          lsm6dsr_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate);
          angular_rate_mdps[0] =
            lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[0]);
          angular_rate_mdps[1] =
            lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[1]);
          angular_rate_mdps[2] =
            lsm6dsr_from_fs2000dps_to_mdps(data_raw_angular_rate[2]);
          printf("Angular rate [mdps]:%4.2f\t%4.2f\t%4.2f\r\n",
                  angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2]);
        }

        lsm6dsr_temp_flag_data_ready_get(&dev_ctx, &reg);

        if (reg) {
          /* Read temperature data */
          memset(&data_raw_temperature, 0x00, sizeof(int16_t));
          lsm6dsr_temperature_raw_get(&dev_ctx, &data_raw_temperature);
          temperature_degC = lsm6dsr_from_lsb_to_celsius(
                               data_raw_temperature);
          printf( "Temperature [degC]:%6.2f\r\n", temperature_degC);

        }








    }
  }
} /* end main */


/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
    hal_i2c_handle_t *hi2c = (hal_i2c_handle_t *)handle;

    if (HAL_I2C_MASTER_MemWrite(hi2c,
    		LSM6DSR_I2C_ADD_L,
                                reg,
                                HAL_I2C_MEM_ADDR_8BIT,
                                bufp,
                                len,
                                1000) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{

    hal_i2c_handle_t *hi2c = (hal_i2c_handle_t *)handle;

    if (HAL_I2C_MASTER_MemRead(hi2c,
    		LSM6DSR_I2C_ADD_L,
                               reg,
                               HAL_I2C_MEM_ADDR_8BIT,
                               bufp,
                               len,
                               1000) != HAL_OK)
    {
        return -1;
    }

    return 0;
}



/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 *
 */
static void platform_delay(uint32_t ms)
{

  HAL_Delay(ms);
}
