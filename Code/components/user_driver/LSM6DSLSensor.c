/**
 ******************************************************************************
 * @file    LSM6DSLSensor.cpp
 * @author  AST
 * @version V1.0.0
 * @date    7 September 2017
 * @brief   Implementation of an LSM6DSL Inertial Measurement Unit (IMU) 6 axes
 *          sensor.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "LSM6DSLSensor.h"
#include "../peripheral/user_i2c.h"
#include "../../Common.h"
/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
#define I2C_MASTER_NUM 0 /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_SCL_PIN 22
#define I2C_SDA_PIN 21
#define I2C_FREQ_100K 100000

/* Private functions ---------------------------------------------------------*/
static LSM6DSLStatusTypeDef Set_X_ODR_When_Enabled(float odr);
static LSM6DSLStatusTypeDef Set_G_ODR_When_Enabled(float odr);
static LSM6DSLStatusTypeDef Set_X_ODR_When_Disabled(float odr);
static LSM6DSLStatusTypeDef Set_G_ODR_When_Disabled(float odr);

static uint8_t X_isEnabled;
static float X_Last_ODR;
static uint8_t G_isEnabled;
static float G_Last_ODR;

/**
 * @brief  Configure the sensor in order to be used
 * @retval 0 in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_begin()
{
    X_isEnabled = 0;
    G_isEnabled = 0;

    int ret = i2c_master_init(I2C_MASTER_NUM, I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_100K);
    if (ret == ESP_OK)
        APP_LOGI("LSM6DSL_ACC_GYRO_Init = %d", ret);
    else
        APP_LOGI("LSM6DSL_ACC_GYRO_Init err");

    /* Enable register address automatically incremented during a multiple byte
     access with a serial interface. */
    if (LSM6DSL_ACC_GYRO_W_IF_Addr_Incr(NULL, LSM6DSL_ACC_GYRO_IF_INC_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable BDU */
    if (LSM6DSL_ACC_GYRO_W_BDU(NULL, LSM6DSL_ACC_GYRO_BDU_BLOCK_UPDATE) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* FIFO mode selection */
    if (LSM6DSL_ACC_GYRO_W_FIFO_MODE(NULL, LSM6DSL_ACC_GYRO_FIFO_MODE_BYPASS) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Output data rate selection - power down. */
    if (LSM6DSL_ACC_GYRO_W_ODR_XL(NULL, LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Full scale selection. */
    if (LSM6DSLSensor_Set_X_FS(8.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Output data rate selection - power down */
    if (LSM6DSL_ACC_GYRO_W_ODR_G(NULL, LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Full scale selection. */
    if (LSM6DSLSensor_Set_G_FS(2000.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    X_Last_ODR = 104.0f;

    X_isEnabled = 0;

    G_Last_ODR = 104.0f;

    G_isEnabled = 0;

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Disable the sensor and relative resources
 * @retval 0 in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_end()
{
    /* Disable both acc and gyro */
    if (LSM6DSLSensor_Disable_X() != LSM6DSL_STATUS_OK)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if (LSM6DSLSensor_Disable_G() != LSM6DSL_STATUS_OK)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Enable LSM6DSL Accelerator
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_X(void)
{
    /* Check if the component is already enabled */
    if (X_isEnabled == 1)
    {
        return LSM6DSL_STATUS_OK;
    }

    /* Output data rate selection. */
    if (Set_X_ODR_When_Enabled(X_Last_ODR) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    X_isEnabled = 1;

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Enable LSM6DSL Gyroscope
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_G(void)
{
    /* Check if the component is already enabled */
    if (G_isEnabled == 1)
    {
        return LSM6DSL_STATUS_OK;
    }

    /* Output data rate selection. */
    if (Set_G_ODR_When_Enabled(G_Last_ODR) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    G_isEnabled = 1;

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Disable LSM6DSL Accelerator
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Disable_X(void)
{
    /* Check if the component is already disabled */
    if (X_isEnabled == 0)
    {
        return LSM6DSL_STATUS_OK;
    }

    /* Store actual output data rate. */
    if (LSM6DSLSensor_Get_X_ODR(&X_Last_ODR) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Output data rate selection - power down. */
    if (LSM6DSL_ACC_GYRO_W_ODR_XL(NULL, LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    X_isEnabled = 0;

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Disable LSM6DSL Gyroscope
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Disable_G(void)
{
    /* Check if the component is already disabled */
    if (G_isEnabled == 0)
    {
        return LSM6DSL_STATUS_OK;
    }

    /* Store actual output data rate. */
    if (LSM6DSLSensor_Get_G_ODR(&G_Last_ODR) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Output data rate selection - power down */
    if (LSM6DSL_ACC_GYRO_W_ODR_G(NULL, LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    G_isEnabled = 0;

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read ID of LSM6DSL Accelerometer and Gyroscope
 * @param  p_id the pointer where the ID of the device is stored
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_ReadID(uint8_t *p_id)
{
    if (!p_id)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Read WHO AM I register */
    if (LSM6DSL_ACC_GYRO_R_WHO_AM_I(NULL, p_id) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read data from LSM6DSL Accelerometer
 * @param  pData the pointer where the accelerometer data are stored
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_X_Axes(int32_t *pData)
{
    int16_t dataRaw[3];
    float sensitivity = 0;

    /* Read raw data from LSM6DSL output register. */
    if (LSM6DSLSensor_Get_X_AxesRaw(dataRaw) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Get LSM6DSL actual sensitivity. */
    if (LSM6DSLSensor_Get_X_Sensitivity(&sensitivity) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Calculate the data. */
    pData[0] = (int32_t)(dataRaw[0] * sensitivity);
    pData[1] = (int32_t)(dataRaw[1] * sensitivity);
    pData[2] = (int32_t)(dataRaw[2] * sensitivity);

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read data from LSM6DSL Gyroscope
 * @param  pData the pointer where the gyroscope data are stored
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_G_Axes(int32_t *pData)
{
    int16_t dataRaw[3];
    float sensitivity = 0;

    /* Read raw data from LSM6DSL output register. */
    if (LSM6DSLSensor_Get_G_AxesRaw(dataRaw) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Get LSM6DSL actual sensitivity. */
    if (LSM6DSLSensor_Get_G_Sensitivity(&sensitivity) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Calculate the data. */
    pData[0] = (int32_t)(dataRaw[0] * sensitivity);
    pData[1] = (int32_t)(dataRaw[1] * sensitivity);
    pData[2] = (int32_t)(dataRaw[2] * sensitivity);

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read Accelerometer Sensitivity
 * @param  pfData the pointer where the accelerometer sensitivity is stored
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_X_Sensitivity(float *pfData)
{
    LSM6DSL_ACC_GYRO_FS_XL_t fullScale;

    /* Read actual full scale selection from sensor. */
    if (LSM6DSL_ACC_GYRO_R_FS_XL(NULL, &fullScale) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Store the sensitivity based on actual full scale. */
    switch (fullScale)
    {
    case LSM6DSL_ACC_GYRO_FS_XL_2g:
        *pfData = (float)LSM6DSL_ACC_SENSITIVITY_FOR_FS_2G;
        break;
    case LSM6DSL_ACC_GYRO_FS_XL_4g:
        *pfData = (float)LSM6DSL_ACC_SENSITIVITY_FOR_FS_4G;
        break;
    case LSM6DSL_ACC_GYRO_FS_XL_8g:
        *pfData = (float)LSM6DSL_ACC_SENSITIVITY_FOR_FS_8G;
        break;
    case LSM6DSL_ACC_GYRO_FS_XL_16g:
        *pfData = (float)LSM6DSL_ACC_SENSITIVITY_FOR_FS_16G;
        break;
    default:
        *pfData = -1.0f;
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read Gyroscope Sensitivity
 * @param  pfData the pointer where the gyroscope sensitivity is stored
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_G_Sensitivity(float *pfData)
{
    LSM6DSL_ACC_GYRO_FS_125_t fullScale125;
    LSM6DSL_ACC_GYRO_FS_G_t fullScale;

    /* Read full scale 125 selection from sensor. */
    if (LSM6DSL_ACC_GYRO_R_FS_125(NULL, &fullScale125) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if (fullScale125 == LSM6DSL_ACC_GYRO_FS_125_ENABLED)
    {
        *pfData = (float)LSM6DSL_GYRO_SENSITIVITY_FOR_FS_125DPS;
    }

    else
    {

        /* Read actual full scale selection from sensor. */
        if (LSM6DSL_ACC_GYRO_R_FS_G(NULL, &fullScale) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }

        /* Store the sensitivity based on actual full scale. */
        switch (fullScale)
        {
        case LSM6DSL_ACC_GYRO_FS_G_245dps:
            *pfData = (float)LSM6DSL_GYRO_SENSITIVITY_FOR_FS_245DPS;
            break;
        case LSM6DSL_ACC_GYRO_FS_G_500dps:
            *pfData = (float)LSM6DSL_GYRO_SENSITIVITY_FOR_FS_500DPS;
            break;
        case LSM6DSL_ACC_GYRO_FS_G_1000dps:
            *pfData = (float)LSM6DSL_GYRO_SENSITIVITY_FOR_FS_1000DPS;
            break;
        case LSM6DSL_ACC_GYRO_FS_G_2000dps:
            *pfData = (float)LSM6DSL_GYRO_SENSITIVITY_FOR_FS_2000DPS;
            break;
        default:
            *pfData = -1.0f;
            return LSM6DSL_STATUS_ERROR;
        }
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read raw data from LSM6DSL Accelerometer
 * @param  pData the pointer where the accelerometer raw data are stored
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_X_AxesRaw(int16_t *pData)
{
    uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};

    /* Read output registers from LSM6DSL_ACC_GYRO_OUTX_L_XL to LSM6DSL_ACC_GYRO_OUTZ_H_XL. */
    if (LSM6DSL_ACC_GYRO_GetRawAccData(NULL, regValue) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Format the data. */
    pData[0] = ((((int16_t)regValue[1]) << 8) + (int16_t)regValue[0]);
    pData[1] = ((((int16_t)regValue[3]) << 8) + (int16_t)regValue[2]);
    pData[2] = ((((int16_t)regValue[5]) << 8) + (int16_t)regValue[4]);

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read raw data from LSM6DSL Gyroscope
 * @param  pData the pointer where the gyroscope raw data are stored
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_G_AxesRaw(int16_t *pData)
{
    uint8_t regValue[6] = {0, 0, 0, 0, 0, 0};

    /* Read output registers from LSM6DSL_ACC_GYRO_OUTX_L_G to LSM6DSL_ACC_GYRO_OUTZ_H_G. */
    if (LSM6DSL_ACC_GYRO_GetRawGyroData(NULL, regValue) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Format the data. */
    pData[0] = ((((int16_t)regValue[1]) << 8) + (int16_t)regValue[0]);
    pData[1] = ((((int16_t)regValue[3]) << 8) + (int16_t)regValue[2]);
    pData[2] = ((((int16_t)regValue[5]) << 8) + (int16_t)regValue[4]);

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read LSM6DSL Accelerometer output data rate
 * @param  odr the pointer to the output data rate
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_X_ODR(float *odr)
{
    LSM6DSL_ACC_GYRO_ODR_XL_t odr_low_level;

    if (LSM6DSL_ACC_GYRO_R_ODR_XL(NULL, &odr_low_level) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    switch (odr_low_level)
    {
    case LSM6DSL_ACC_GYRO_ODR_XL_POWER_DOWN:
        *odr = 0.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_XL_13Hz:
        *odr = 13.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_XL_26Hz:
        *odr = 26.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_XL_52Hz:
        *odr = 52.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_XL_104Hz:
        *odr = 104.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_XL_208Hz:
        *odr = 208.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_XL_416Hz:
        *odr = 416.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_XL_833Hz:
        *odr = 833.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_XL_1660Hz:
        *odr = 1660.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_XL_3330Hz:
        *odr = 3330.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_XL_6660Hz:
        *odr = 6660.0f;
        break;
    default:
        *odr = -1.0f;
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read LSM6DSL Gyroscope output data rate
 * @param  odr the pointer to the output data rate
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_G_ODR(float *odr)
{
    LSM6DSL_ACC_GYRO_ODR_G_t odr_low_level;

    if (LSM6DSL_ACC_GYRO_R_ODR_G(NULL, &odr_low_level) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    switch (odr_low_level)
    {
    case LSM6DSL_ACC_GYRO_ODR_G_POWER_DOWN:
        *odr = 0.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_G_13Hz:
        *odr = 13.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_G_26Hz:
        *odr = 26.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_G_52Hz:
        *odr = 52.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_G_104Hz:
        *odr = 104.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_G_208Hz:
        *odr = 208.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_G_416Hz:
        *odr = 416.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_G_833Hz:
        *odr = 833.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_G_1660Hz:
        *odr = 1660.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_G_3330Hz:
        *odr = 3330.0f;
        break;
    case LSM6DSL_ACC_GYRO_ODR_G_6660Hz:
        *odr = 6660.0f;
        break;
    default:
        *odr = -1.0f;
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Set LSM6DSL Accelerometer output data rate
 * @param  odr the output data rate to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_X_ODR(float odr)
{
    if (X_isEnabled == 1)
    {
        if (Set_X_ODR_When_Enabled(odr) == LSM6DSL_STATUS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
    }
    else
    {
        if (Set_X_ODR_When_Disabled(odr) == LSM6DSL_STATUS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Set LSM6DSL Accelerometer output data rate when enabled
 * @param  odr the output data rate to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef Set_X_ODR_When_Enabled(float odr)
{
    LSM6DSL_ACC_GYRO_ODR_XL_t new_odr;

    new_odr = (odr <= 13.0f)     ? LSM6DSL_ACC_GYRO_ODR_XL_13Hz
              : (odr <= 26.0f)   ? LSM6DSL_ACC_GYRO_ODR_XL_26Hz
              : (odr <= 52.0f)   ? LSM6DSL_ACC_GYRO_ODR_XL_52Hz
              : (odr <= 104.0f)  ? LSM6DSL_ACC_GYRO_ODR_XL_104Hz
              : (odr <= 208.0f)  ? LSM6DSL_ACC_GYRO_ODR_XL_208Hz
              : (odr <= 416.0f)  ? LSM6DSL_ACC_GYRO_ODR_XL_416Hz
              : (odr <= 833.0f)  ? LSM6DSL_ACC_GYRO_ODR_XL_833Hz
              : (odr <= 1660.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_1660Hz
              : (odr <= 3330.0f) ? LSM6DSL_ACC_GYRO_ODR_XL_3330Hz
                                 : LSM6DSL_ACC_GYRO_ODR_XL_6660Hz;

    if (LSM6DSL_ACC_GYRO_W_ODR_XL(NULL, new_odr) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Set LSM6DSL Accelerometer output data rate when disabled
 * @param  odr the output data rate to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef Set_X_ODR_When_Disabled(float odr)
{
    X_Last_ODR = (odr <= 13.0f)     ? 13.0f
                 : (odr <= 26.0f)   ? 26.0f
                 : (odr <= 52.0f)   ? 52.0f
                 : (odr <= 104.0f)  ? 104.0f
                 : (odr <= 208.0f)  ? 208.0f
                 : (odr <= 416.0f)  ? 416.0f
                 : (odr <= 833.0f)  ? 833.0f
                 : (odr <= 1660.0f) ? 1660.0f
                 : (odr <= 3330.0f) ? 3330.0f
                                    : 6660.0f;

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Set LSM6DSL Gyroscope output data rate
 * @param  odr the output data rate to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_G_ODR(float odr)
{
    if (G_isEnabled == 1)
    {
        if (Set_G_ODR_When_Enabled(odr) == LSM6DSL_STATUS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
    }
    else
    {
        if (Set_G_ODR_When_Disabled(odr) == LSM6DSL_STATUS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Set LSM6DSL Gyroscope output data rate when enabled
 * @param  odr the output data rate to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef Set_G_ODR_When_Enabled(float odr)
{
    LSM6DSL_ACC_GYRO_ODR_G_t new_odr;

    new_odr = (odr <= 13.0f)     ? LSM6DSL_ACC_GYRO_ODR_G_13Hz
              : (odr <= 26.0f)   ? LSM6DSL_ACC_GYRO_ODR_G_26Hz
              : (odr <= 52.0f)   ? LSM6DSL_ACC_GYRO_ODR_G_52Hz
              : (odr <= 104.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_104Hz
              : (odr <= 208.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_208Hz
              : (odr <= 416.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_416Hz
              : (odr <= 833.0f)  ? LSM6DSL_ACC_GYRO_ODR_G_833Hz
              : (odr <= 1660.0f) ? LSM6DSL_ACC_GYRO_ODR_G_1660Hz
              : (odr <= 3330.0f) ? LSM6DSL_ACC_GYRO_ODR_G_3330Hz
                                 : LSM6DSL_ACC_GYRO_ODR_G_6660Hz;

    if (LSM6DSL_ACC_GYRO_W_ODR_G(NULL, new_odr) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Set LSM6DSL Gyroscope output data rate when disabled
 * @param  odr the output data rate to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef Set_G_ODR_When_Disabled(float odr)
{
    G_Last_ODR = (odr <= 13.0f)     ? 13.0f
                 : (odr <= 26.0f)   ? 26.0f
                 : (odr <= 52.0f)   ? 52.0f
                 : (odr <= 104.0f)  ? 104.0f
                 : (odr <= 208.0f)  ? 208.0f
                 : (odr <= 416.0f)  ? 416.0f
                 : (odr <= 833.0f)  ? 833.0f
                 : (odr <= 1660.0f) ? 1660.0f
                 : (odr <= 3330.0f) ? 3330.0f
                                    : 6660.0f;

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read LSM6DSL Accelerometer full scale
 * @param  fullScale the pointer to the full scale
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_X_FS(float *fullScale)
{
    LSM6DSL_ACC_GYRO_FS_XL_t fs_low_level;

    if (LSM6DSL_ACC_GYRO_R_FS_XL(NULL, &fs_low_level) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    switch (fs_low_level)
    {
    case LSM6DSL_ACC_GYRO_FS_XL_2g:
        *fullScale = 2.0f;
        break;
    case LSM6DSL_ACC_GYRO_FS_XL_4g:
        *fullScale = 4.0f;
        break;
    case LSM6DSL_ACC_GYRO_FS_XL_8g:
        *fullScale = 8.0f;
        break;
    case LSM6DSL_ACC_GYRO_FS_XL_16g:
        *fullScale = 16.0f;
        break;
    default:
        *fullScale = -1.0f;
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Read LSM6DSL Gyroscope full scale
 * @param  fullScale the pointer to the full scale
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_G_FS(float *fullScale)
{
    LSM6DSL_ACC_GYRO_FS_G_t fs_low_level;
    LSM6DSL_ACC_GYRO_FS_125_t fs_125;

    if (LSM6DSL_ACC_GYRO_R_FS_125(NULL, &fs_125) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }
    if (LSM6DSL_ACC_GYRO_R_FS_G(NULL, &fs_low_level) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if (fs_125 == LSM6DSL_ACC_GYRO_FS_125_ENABLED)
    {
        *fullScale = 125.0f;
    }

    else
    {
        switch (fs_low_level)
        {
        case LSM6DSL_ACC_GYRO_FS_G_245dps:
            *fullScale = 245.0f;
            break;
        case LSM6DSL_ACC_GYRO_FS_G_500dps:
            *fullScale = 500.0f;
            break;
        case LSM6DSL_ACC_GYRO_FS_G_1000dps:
            *fullScale = 1000.0f;
            break;
        case LSM6DSL_ACC_GYRO_FS_G_2000dps:
            *fullScale = 2000.0f;
            break;
        default:
            *fullScale = -1.0f;
            return LSM6DSL_STATUS_ERROR;
        }
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Set LSM6DSL Accelerometer full scale
 * @param  fullScale the full scale to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_X_FS(float fullScale)
{
    LSM6DSL_ACC_GYRO_FS_XL_t new_fs;

    new_fs = (fullScale <= 2.0f)   ? LSM6DSL_ACC_GYRO_FS_XL_2g
             : (fullScale <= 4.0f) ? LSM6DSL_ACC_GYRO_FS_XL_4g
             : (fullScale <= 8.0f) ? LSM6DSL_ACC_GYRO_FS_XL_8g
                                   : LSM6DSL_ACC_GYRO_FS_XL_16g;

    if (LSM6DSL_ACC_GYRO_W_FS_XL(NULL, new_fs) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief  Set LSM6DSL Gyroscope full scale
 * @param  fullScale the full scale to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_G_FS(float fullScale)
{
    LSM6DSL_ACC_GYRO_FS_G_t new_fs;

    if (fullScale <= 125.0f)
    {
        if (LSM6DSL_ACC_GYRO_W_FS_125(NULL, LSM6DSL_ACC_GYRO_FS_125_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
    }
    else
    {
        new_fs = (fullScale <= 245.0f)    ? LSM6DSL_ACC_GYRO_FS_G_245dps
                 : (fullScale <= 500.0f)  ? LSM6DSL_ACC_GYRO_FS_G_500dps
                 : (fullScale <= 1000.0f) ? LSM6DSL_ACC_GYRO_FS_G_1000dps
                                          : LSM6DSL_ACC_GYRO_FS_G_2000dps;

        if (LSM6DSL_ACC_GYRO_W_FS_125(NULL, LSM6DSL_ACC_GYRO_FS_125_DISABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        if (LSM6DSL_ACC_GYRO_W_FS_G(NULL, new_fs) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Enable free fall detection
 * @note This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
*/
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Free_Fall_Detection_Int1_Pin(void)
{
    return LSM6DSLSensor_Enable_Free_Fall_Detection(LSM6DSL_INT1_PIN);
}

/**
 * @brief Enable free fall detection
 * @param int_pin the interrupt pin to be used
 * @note This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
*/
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Free_Fall_Detection(LSM6DSL_Interrupt_Pin_t int_pin)
{
    /* Output Data Rate selection */
    if (LSM6DSLSensor_Set_X_ODR(416.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Full scale selection */
    if (LSM6DSL_ACC_GYRO_W_FS_XL(NULL, LSM6DSL_ACC_GYRO_FS_XL_2g) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* FF_DUR setting */
    if (LSM6DSL_ACC_GYRO_W_FF_Duration(NULL, 0x06) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* WAKE_DUR setting */
    if (LSM6DSL_ACC_GYRO_W_WAKE_DUR(NULL, 0x00) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* TIMER_HR setting */
    if (LSM6DSL_ACC_GYRO_W_TIMER_HR(NULL, LSM6DSL_ACC_GYRO_TIMER_HR_6_4ms) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* SLEEP_DUR setting */
    if (LSM6DSL_ACC_GYRO_W_SLEEP_DUR(NULL, 0x00) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* FF_THS setting */
    if (LSM6DSL_ACC_GYRO_W_FF_THS(NULL, LSM6DSL_ACC_GYRO_FF_THS_312mg) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable basic Interrupts */
    if (LSM6DSL_ACC_GYRO_W_BASIC_INT(NULL, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable free fall event on either INT1 or INT2 pin */
    switch (int_pin)
    {
    case LSM6DSL_INT1_PIN:
        if (LSM6DSL_ACC_GYRO_W_FFEvOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_FF_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    case LSM6DSL_INT2_PIN:
        if (LSM6DSL_ACC_GYRO_W_FFEvOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_FF_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Disable free fall detection
 * @param None
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
*/
LSM6DSLStatusTypeDef LSM6DSLSensor_Disable_Free_Fall_Detection(void)
{
    /* Disable free fall event on INT1 pin */
    if (LSM6DSL_ACC_GYRO_W_FFEvOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_FF_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable free fall event on INT2 pin */
    if (LSM6DSL_ACC_GYRO_W_FFEvOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_FF_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable basic Interrupts */
    if (LSM6DSL_ACC_GYRO_W_BASIC_INT(NULL, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* FF_DUR setting */
    if (LSM6DSL_ACC_GYRO_W_FF_Duration(NULL, 0x00) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* FF_THS setting */
    if (LSM6DSL_ACC_GYRO_W_FF_THS(NULL, LSM6DSL_ACC_GYRO_FF_THS_156mg) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Set the free fall detection threshold for LSM6DSL accelerometer sensor
 * @param thr the threshold to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_Free_Fall_Threshold(uint8_t thr)
{

    if (LSM6DSL_ACC_GYRO_W_FF_THS(NULL, (LSM6DSL_ACC_GYRO_FF_THS_t)thr) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Enable the pedometer feature for LSM6DSL accelerometer sensor
 * @note This function sets the LSM6DSL accelerometer ODR to 26Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Pedometer(void)
{
    /* Output Data Rate selection */
    if (LSM6DSLSensor_Set_X_ODR(26.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Full scale selection. */
    if (LSM6DSLSensor_Set_X_FS(2.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Set pedometer threshold. */
    if (LSM6DSLSensor_Set_Pedometer_Threshold(LSM6DSL_PEDOMETER_THRESHOLD_MID_HIGH) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable embedded functionalities. */
    if (LSM6DSL_ACC_GYRO_W_FUNC_EN(NULL, LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable pedometer algorithm. */
    if (LSM6DSL_ACC_GYRO_W_PEDO(NULL, LSM6DSL_ACC_GYRO_PEDO_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable pedometer on INT1. */
    if (LSM6DSL_ACC_GYRO_W_STEP_DET_on_INT1(NULL, LSM6DSL_ACC_GYRO_INT1_PEDO_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Disable the pedometer feature for LSM6DSL accelerometer sensor
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Disable_Pedometer(void)
{
    /* Disable pedometer on INT1. */
    if (LSM6DSL_ACC_GYRO_W_STEP_DET_on_INT1(NULL, LSM6DSL_ACC_GYRO_INT1_PEDO_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable pedometer algorithm. */
    if (LSM6DSL_ACC_GYRO_W_PEDO(NULL, LSM6DSL_ACC_GYRO_PEDO_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable embedded functionalities. */
    if (LSM6DSL_ACC_GYRO_W_FUNC_EN(NULL, LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Reset pedometer threshold. */
    if (LSM6DSLSensor_Set_Pedometer_Threshold(0x0) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Get the step counter for LSM6DSL accelerometer sensor
 * @param step_count the pointer to the step counter
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_Step_Counter(uint16_t *step_count)
{
    if (LSM6DSL_ACC_GYRO_Get_GetStepCounter(NULL, (uint8_t *)step_count) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Reset of the step counter for LSM6DSL accelerometer sensor
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Reset_Step_Counter(void)
{
    if (LSM6DSL_ACC_GYRO_W_PedoStepReset(NULL, LSM6DSL_ACC_GYRO_PEDO_RST_STEP_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    // delay(10);

    if (LSM6DSL_ACC_GYRO_W_PedoStepReset(NULL, LSM6DSL_ACC_GYRO_PEDO_RST_STEP_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Set the pedometer threshold for LSM6DSL accelerometer sensor
 * @param thr the threshold to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_Pedometer_Threshold(uint8_t thr)
{
    if (LSM6DSL_ACC_GYRO_W_PedoThreshold(NULL, thr) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Enable the tilt detection for LSM6DSL accelerometer sensor
 * @note This function sets the LSM6DSL accelerometer ODR to 26Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Tilt_Detection_Int1_Pin(void)
{
    return LSM6DSLSensor_Enable_Tilt_Detection(LSM6DSL_INT1_PIN);
}

/**
 * @brief Enable the tilt detection for LSM6DSL accelerometer sensor
 * @param int_pin the interrupt pin to be used
 * @note This function sets the LSM6DSL accelerometer ODR to 26Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Tilt_Detection(LSM6DSL_Interrupt_Pin_t int_pin)
{
    /* Output Data Rate selection */
    if (LSM6DSLSensor_Set_X_ODR(26.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Full scale selection. */
    if (LSM6DSLSensor_Set_X_FS(2.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable embedded functionalities */
    if (LSM6DSL_ACC_GYRO_W_FUNC_EN(NULL, LSM6DSL_ACC_GYRO_FUNC_EN_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable tilt calculation. */
    if (LSM6DSL_ACC_GYRO_W_TILT(NULL, LSM6DSL_ACC_GYRO_TILT_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable tilt detection on either INT1 or INT2 pin */
    switch (int_pin)
    {
    case LSM6DSL_INT1_PIN:
        if (LSM6DSL_ACC_GYRO_W_TiltEvOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_TILT_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    case LSM6DSL_INT2_PIN:
        if (LSM6DSL_ACC_GYRO_W_TiltEvOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_TILT_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Disable the tilt detection for LSM6DSL accelerometer sensor
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Disable_Tilt_Detection(void)
{
    /* Disable tilt event on INT1. */
    if (LSM6DSL_ACC_GYRO_W_TiltEvOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_TILT_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable tilt event on INT2. */
    if (LSM6DSL_ACC_GYRO_W_TiltEvOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_TILT_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable tilt calculation. */
    if (LSM6DSL_ACC_GYRO_W_TILT(NULL, LSM6DSL_ACC_GYRO_TILT_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable embedded functionalities */
    if (LSM6DSL_ACC_GYRO_W_FUNC_EN(NULL, LSM6DSL_ACC_GYRO_FUNC_EN_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Enable the wake up detection for LSM6DSL accelerometer sensor
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Wake_Up_Detection_Int2_PIN(void)
{
    return LSM6DSLSensor_Enable_Wake_Up_Detection(LSM6DSL_INT2_PIN);
}

/**
 * @brief Enable the wake up detection for LSM6DSL accelerometer sensor
 * @param int_pin the interrupt pin to be used
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Wake_Up_Detection(LSM6DSL_Interrupt_Pin_t int_pin)
{
    /* Output Data Rate selection */
    if (LSM6DSLSensor_Set_X_ODR(416.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Full scale selection. */
    if (LSM6DSLSensor_Set_X_FS(2.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* WAKE_DUR setting */
    if (LSM6DSL_ACC_GYRO_W_WAKE_DUR(NULL, 0x00) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Set wake up threshold. */
    if (LSM6DSL_ACC_GYRO_W_WK_THS(NULL, 0x02) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable basic Interrupts */
    if (LSM6DSL_ACC_GYRO_W_BASIC_INT(NULL, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable wake up detection on either INT1 or INT2 pin */
    switch (int_pin)
    {
    case LSM6DSL_INT1_PIN:
        if (LSM6DSL_ACC_GYRO_W_WUEvOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_WU_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    case LSM6DSL_INT2_PIN:
        if (LSM6DSL_ACC_GYRO_W_WUEvOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_WU_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Disable the wake up detection for LSM6DSL accelerometer sensor
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Disable_Wake_Up_Detection(void)
{
    /* Disable wake up event on INT1 */
    if (LSM6DSL_ACC_GYRO_W_WUEvOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_WU_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable wake up event on INT2 */
    if (LSM6DSL_ACC_GYRO_W_WUEvOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_WU_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable basic Interrupts */
    if (LSM6DSL_ACC_GYRO_W_BASIC_INT(NULL, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* WU_DUR setting */
    if (LSM6DSL_ACC_GYRO_W_WAKE_DUR(NULL, 0x00) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* WU_THS setting */
    if (LSM6DSL_ACC_GYRO_W_WK_THS(NULL, 0x00) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Set the wake up threshold for LSM6DSL accelerometer sensor
 * @param thr the threshold to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_Wake_Up_Threshold(uint8_t thr)
{
    if (LSM6DSL_ACC_GYRO_W_WK_THS(NULL, thr) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Enable the single tap detection for LSM6DSL accelerometer sensor
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Single_Tap_Detection_Int1_PIN(void)
{
    return LSM6DSLSensor_Enable_Single_Tap_Detection(LSM6DSL_INT1_PIN);
}

/**
 * @brief Enable the single tap detection for LSM6DSL accelerometer sensor
 * @param int_pin the interrupt pin to be used
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Single_Tap_Detection(LSM6DSL_Interrupt_Pin_t int_pin)
{
    /* Output Data Rate selection */
    if (LSM6DSLSensor_Set_X_ODR(416.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Full scale selection. */
    if (LSM6DSLSensor_Set_X_FS(2.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable X direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_X_EN(NULL, LSM6DSL_ACC_GYRO_TAP_X_EN_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable Y direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_Y_EN(NULL, LSM6DSL_ACC_GYRO_TAP_Y_EN_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable Z direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_Z_EN(NULL, LSM6DSL_ACC_GYRO_TAP_Z_EN_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Set tap threshold. */
    if (LSM6DSLSensor_Set_Tap_Threshold(LSM6DSL_TAP_THRESHOLD_MID_LOW) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Set tap shock time window. */
    if (LSM6DSLSensor_Set_Tap_Shock_Time(LSM6DSL_TAP_SHOCK_TIME_MID_HIGH) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Set tap quiet time window. */
    if (LSM6DSLSensor_Set_Tap_Quiet_Time(LSM6DSL_TAP_QUIET_TIME_MID_LOW) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* _NOTE_: Tap duration time window - don't care for single tap. */

    /* _NOTE_: Single/Double Tap event - don't care of this flag for single tap. */

    /* Enable basic Interrupts */
    if (LSM6DSL_ACC_GYRO_W_BASIC_INT(NULL, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable single tap on either INT1 or INT2 pin */
    switch (int_pin)
    {
    case LSM6DSL_INT1_PIN:
        if (LSM6DSL_ACC_GYRO_W_SingleTapOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    case LSM6DSL_INT2_PIN:
        if (LSM6DSL_ACC_GYRO_W_SingleTapOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Disable the single tap detection for LSM6DSL accelerometer sensor
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Disable_Single_Tap_Detection(void)
{
    /* Disable single tap interrupt on INT1 pin. */
    if (LSM6DSL_ACC_GYRO_W_SingleTapOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable single tap interrupt on INT2 pin. */
    if (LSM6DSL_ACC_GYRO_W_SingleTapOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable basic Interrupts */
    if (LSM6DSL_ACC_GYRO_W_BASIC_INT(NULL, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Reset tap threshold. */
    if (LSM6DSLSensor_Set_Tap_Threshold(0x0) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Reset tap shock time window. */
    if (LSM6DSLSensor_Set_Tap_Shock_Time(0x0) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Reset tap quiet time window. */
    if (LSM6DSLSensor_Set_Tap_Quiet_Time(0x0) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* _NOTE_: Tap duration time window - don't care for single tap. */

    /* _NOTE_: Single/Double Tap event - don't care of this flag for single tap. */

    /* Disable Z direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_Z_EN(NULL, LSM6DSL_ACC_GYRO_TAP_Z_EN_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable Y direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_Y_EN(NULL, LSM6DSL_ACC_GYRO_TAP_Y_EN_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable X direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_X_EN(NULL, LSM6DSL_ACC_GYRO_TAP_X_EN_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Enable the double tap detection for LSM6DSL accelerometer sensor
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Double_Tap_Detection_Int1_Pin(void)
{
    return LSM6DSLSensor_Enable_Double_Tap_Detection(LSM6DSL_INT1_PIN);
}

/**
 * @brief Enable the double tap detection for LSM6DSL accelerometer sensor
 * @param int_pin the interrupt pin to be used
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_Double_Tap_Detection(LSM6DSL_Interrupt_Pin_t int_pin)
{
    /* Output Data Rate selection */
    if (LSM6DSLSensor_Set_X_ODR(416.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Full scale selection. */
    if (LSM6DSLSensor_Set_X_FS(2.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable X direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_X_EN(NULL, LSM6DSL_ACC_GYRO_TAP_X_EN_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable Y direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_Y_EN(NULL, LSM6DSL_ACC_GYRO_TAP_Y_EN_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable Z direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_Z_EN(NULL, LSM6DSL_ACC_GYRO_TAP_Z_EN_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Set tap threshold. */
    if (LSM6DSLSensor_Set_Tap_Threshold(LSM6DSL_TAP_THRESHOLD_MID_LOW) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Set tap shock time window. */
    if (LSM6DSLSensor_Set_Tap_Shock_Time(LSM6DSL_TAP_SHOCK_TIME_HIGH) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Set tap quiet time window. */
    if (LSM6DSLSensor_Set_Tap_Quiet_Time(LSM6DSL_TAP_QUIET_TIME_HIGH) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Set tap duration time window. */
    if (LSM6DSLSensor_Set_Tap_Duration_Time(LSM6DSL_TAP_DURATION_TIME_MID) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Single and double tap enabled. */
    if (LSM6DSL_ACC_GYRO_W_SINGLE_DOUBLE_TAP_EV(NULL, LSM6DSL_ACC_GYRO_SINGLE_DOUBLE_TAP_DOUBLE_TAP) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable basic Interrupts */
    if (LSM6DSL_ACC_GYRO_W_BASIC_INT(NULL, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable double tap on either INT1 or INT2 pin */
    switch (int_pin)
    {
    case LSM6DSL_INT1_PIN:
        if (LSM6DSL_ACC_GYRO_W_TapEvOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_TAP_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    case LSM6DSL_INT2_PIN:
        if (LSM6DSL_ACC_GYRO_W_TapEvOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_TAP_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Disable the double tap detection for LSM6DSL accelerometer sensor
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Disable_Double_Tap_Detection(void)
{
    /* Disable double tap interrupt on INT1 pin. */
    if (LSM6DSL_ACC_GYRO_W_TapEvOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_TAP_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable double tap interrupt on INT2 pin. */
    if (LSM6DSL_ACC_GYRO_W_TapEvOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_TAP_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable basic Interrupts */
    if (LSM6DSL_ACC_GYRO_W_BASIC_INT(NULL, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Reset tap threshold. */
    if (LSM6DSLSensor_Set_Tap_Threshold(0x0) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Reset tap shock time window. */
    if (LSM6DSLSensor_Set_Tap_Shock_Time(0x0) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Reset tap quiet time window. */
    if (LSM6DSLSensor_Set_Tap_Quiet_Time(0x0) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Reset tap duration time window. */
    if (LSM6DSLSensor_Set_Tap_Duration_Time(0x0) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Only single tap enabled. */
    if (LSM6DSL_ACC_GYRO_W_SINGLE_DOUBLE_TAP_EV(NULL, LSM6DSL_ACC_GYRO_SINGLE_DOUBLE_TAP_SINGLE_TAP) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable Z direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_Z_EN(NULL, LSM6DSL_ACC_GYRO_TAP_Z_EN_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable Y direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_Y_EN(NULL, LSM6DSL_ACC_GYRO_TAP_Y_EN_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable X direction in tap recognition. */
    if (LSM6DSL_ACC_GYRO_W_TAP_X_EN(NULL, LSM6DSL_ACC_GYRO_TAP_X_EN_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Set the tap threshold for LSM6DSL accelerometer sensor
 * @param thr the threshold to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_Tap_Threshold(uint8_t thr)
{
    if (LSM6DSL_ACC_GYRO_W_TAP_THS(NULL, thr) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Set the tap shock time window for LSM6DSL accelerometer sensor
 * @param time the shock time window to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_Tap_Shock_Time(uint8_t time)
{
    if (LSM6DSL_ACC_GYRO_W_SHOCK_Duration(NULL, time) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Set the tap quiet time window for LSM6DSL accelerometer sensor
 * @param time the quiet time window to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_Tap_Quiet_Time(uint8_t time)
{
    if (LSM6DSL_ACC_GYRO_W_QUIET_Duration(NULL, time) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Set the tap duration of the time window for LSM6DSL accelerometer sensor
 * @param time the duration of the time window to be set
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Set_Tap_Duration_Time(uint8_t time)
{
    if (LSM6DSL_ACC_GYRO_W_DUR(NULL, time) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Enable the 6D orientation detection for LSM6DSL accelerometer sensor
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_6D_Orientation_Int1_PIN(void)
{
    return LSM6DSLSensor_Enable_6D_Orientation(LSM6DSL_INT1_PIN);
}

/**
 * @brief Enable the 6D orientation detection for LSM6DSL accelerometer sensor
 * @param int_pin the interrupt pin to be used
 * @note  This function sets the LSM6DSL accelerometer ODR to 416Hz and the LSM6DSL accelerometer full scale to 2g
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Enable_6D_Orientation(LSM6DSL_Interrupt_Pin_t int_pin)
{
    /* Output Data Rate selection */
    if (LSM6DSLSensor_Set_X_ODR(416.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Full scale selection. */
    if (LSM6DSLSensor_Set_X_FS(2.0f) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Set 6D threshold. */
    if (LSM6DSL_ACC_GYRO_W_SIXD_THS(NULL, LSM6DSL_ACC_GYRO_SIXD_THS_60_degree) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable basic Interrupts */
    if (LSM6DSL_ACC_GYRO_W_BASIC_INT(NULL, LSM6DSL_ACC_GYRO_BASIC_INT_ENABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Enable 6D orientation on either INT1 or INT2 pin */
    switch (int_pin)
    {
    case LSM6DSL_INT1_PIN:
        if (LSM6DSL_ACC_GYRO_W_6DEvOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_6D_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    case LSM6DSL_INT2_PIN:
        if (LSM6DSL_ACC_GYRO_W_6DEvOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_6D_ENABLED) == MEMS_ERROR)
        {
            return LSM6DSL_STATUS_ERROR;
        }
        break;

    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Disable the 6D orientation detection for LSM6DSL accelerometer sensor
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Disable_6D_Orientation(void)
{
    /* Disable 6D orientation interrupt on INT1 pin. */
    if (LSM6DSL_ACC_GYRO_W_6DEvOnInt1(NULL, LSM6DSL_ACC_GYRO_INT1_6D_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable 6D orientation interrupt on INT2 pin. */
    if (LSM6DSL_ACC_GYRO_W_6DEvOnInt2(NULL, LSM6DSL_ACC_GYRO_INT2_6D_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Disable basic Interrupts */
    if (LSM6DSL_ACC_GYRO_W_BASIC_INT(NULL, LSM6DSL_ACC_GYRO_BASIC_INT_DISABLED) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    /* Reset 6D threshold. */
    if (LSM6DSL_ACC_GYRO_W_SIXD_THS(NULL, LSM6DSL_ACC_GYRO_SIXD_THS_80_degree) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Get the 6D orientation XL axis for LSM6DSL accelerometer sensor
 * @param xl the pointer to the 6D orientation XL axis
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_6D_Orientation_XL(uint8_t *xl)
{
    LSM6DSL_ACC_GYRO_DSD_XL_t xl_raw;

    if (LSM6DSL_ACC_GYRO_R_DSD_XL(NULL, &xl_raw) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    switch (xl_raw)
    {
    case LSM6DSL_ACC_GYRO_DSD_XL_DETECTED:
        *xl = 1;
        break;
    case LSM6DSL_ACC_GYRO_DSD_XL_NOT_DETECTED:
        *xl = 0;
        break;
    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Get the 6D orientation XH axis for LSM6DSL accelerometer sensor
 * @param xh the pointer to the 6D orientation XH axis
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_6D_Orientation_XH(uint8_t *xh)
{
    LSM6DSL_ACC_GYRO_DSD_XH_t xh_raw;

    if (LSM6DSL_ACC_GYRO_R_DSD_XH(NULL, &xh_raw) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    switch (xh_raw)
    {
    case LSM6DSL_ACC_GYRO_DSD_XH_DETECTED:
        *xh = 1;
        break;
    case LSM6DSL_ACC_GYRO_DSD_XH_NOT_DETECTED:
        *xh = 0;
        break;
    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Get the 6D orientation YL axis for LSM6DSL accelerometer sensor
 * @param yl the pointer to the 6D orientation YL axis
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_6D_Orientation_YL(uint8_t *yl)
{
    LSM6DSL_ACC_GYRO_DSD_YL_t yl_raw;

    if (LSM6DSL_ACC_GYRO_R_DSD_YL(NULL, &yl_raw) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    switch (yl_raw)
    {
    case LSM6DSL_ACC_GYRO_DSD_YL_DETECTED:
        *yl = 1;
        break;
    case LSM6DSL_ACC_GYRO_DSD_YL_NOT_DETECTED:
        *yl = 0;
        break;
    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Get the 6D orientation YH axis for LSM6DSL accelerometer sensor
 * @param yh the pointer to the 6D orientation YH axis
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_6D_Orientation_YH(uint8_t *yh)
{
    LSM6DSL_ACC_GYRO_DSD_YH_t yh_raw;

    if (LSM6DSL_ACC_GYRO_R_DSD_YH(NULL, &yh_raw) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    switch (yh_raw)
    {
    case LSM6DSL_ACC_GYRO_DSD_YH_DETECTED:
        *yh = 1;
        break;
    case LSM6DSL_ACC_GYRO_DSD_YH_NOT_DETECTED:
        *yh = 0;
        break;
    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Get the 6D orientation ZL axis for LSM6DSL accelerometer sensor
 * @param zl the pointer to the 6D orientation ZL axis
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_6D_Orientation_ZL(uint8_t *zl)
{
    LSM6DSL_ACC_GYRO_DSD_ZL_t zl_raw;

    if (LSM6DSL_ACC_GYRO_R_DSD_ZL(NULL, &zl_raw) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    switch (zl_raw)
    {
    case LSM6DSL_ACC_GYRO_DSD_ZL_DETECTED:
        *zl = 1;
        break;
    case LSM6DSL_ACC_GYRO_DSD_ZL_NOT_DETECTED:
        *zl = 0;
        break;
    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Get the 6D orientation ZH axis for LSM6DSL accelerometer sensor
 * @param zh the pointer to the 6D orientation ZH axis
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_6D_Orientation_ZH(uint8_t *zh)
{
    LSM6DSL_ACC_GYRO_DSD_ZH_t zh_raw;

    if (LSM6DSL_ACC_GYRO_R_DSD_ZH(NULL, &zh_raw) == MEMS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    switch (zh_raw)
    {
    case LSM6DSL_ACC_GYRO_DSD_ZH_DETECTED:
        *zh = 1;
        break;
    case LSM6DSL_ACC_GYRO_DSD_ZH_NOT_DETECTED:
        *zh = 0;
        break;
    default:
        return LSM6DSL_STATUS_ERROR;
    }

    return LSM6DSL_STATUS_OK;
}

/**
 * @brief Get the status of all hardware events for LSM6DSL accelerometer sensor
 * @param status the pointer to the status of all hardware events
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_Get_Event_Status(LSM6DSL_Event_Status_t *status)
{
    uint8_t Wake_Up_Src = 0, Tap_Src = 0, D6D_Src = 0, Func_Src = 0, Md1_Cfg = 0, Md2_Cfg = 0, Int1_Ctrl = 0;

    memset((void *)status, 0x0, sizeof(LSM6DSL_Event_Status_t));

    if (LSM6DSLSensor_ReadReg(LSM6DSL_ACC_GYRO_WAKE_UP_SRC, &Wake_Up_Src) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if (LSM6DSLSensor_ReadReg(LSM6DSL_ACC_GYRO_TAP_SRC, &Tap_Src) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if (LSM6DSLSensor_ReadReg(LSM6DSL_ACC_GYRO_D6D_SRC, &D6D_Src) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if (LSM6DSLSensor_ReadReg(LSM6DSL_ACC_GYRO_FUNC_SRC, &Func_Src) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if (LSM6DSLSensor_ReadReg(LSM6DSL_ACC_GYRO_MD1_CFG, &Md1_Cfg) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if (LSM6DSLSensor_ReadReg(LSM6DSL_ACC_GYRO_MD2_CFG, &Md2_Cfg) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if (LSM6DSLSensor_ReadReg(LSM6DSL_ACC_GYRO_INT1_CTRL, &Int1_Ctrl) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if ((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_FF_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_FF_MASK))
    {
        if ((Wake_Up_Src & LSM6DSL_ACC_GYRO_FF_EV_STATUS_MASK))
        {
            status->FreeFallStatus = 1;
        }
    }

    if ((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_WU_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_WU_MASK))
    {
        if ((Wake_Up_Src & LSM6DSL_ACC_GYRO_WU_EV_STATUS_MASK))
        {
            status->WakeUpStatus = 1;
        }
    }

    if ((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_SINGLE_TAP_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_SINGLE_TAP_MASK))
    {
        if ((Tap_Src & LSM6DSL_ACC_GYRO_SINGLE_TAP_EV_STATUS_MASK))
        {
            status->TapStatus = 1;
        }
    }

    if ((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_TAP_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_TAP_MASK))
    {
        if ((Tap_Src & LSM6DSL_ACC_GYRO_DOUBLE_TAP_EV_STATUS_MASK))
        {
            status->DoubleTapStatus = 1;
        }
    }

    if ((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_6D_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_6D_MASK))
    {
        if ((D6D_Src & LSM6DSL_ACC_GYRO_D6D_EV_STATUS_MASK))
        {
            status->D6DOrientationStatus = 1;
        }
    }

    if ((Int1_Ctrl & LSM6DSL_ACC_GYRO_INT1_PEDO_MASK))
    {
        if ((Func_Src & LSM6DSL_ACC_GYRO_PEDO_EV_STATUS_MASK))
        {
            status->StepStatus = 1;
        }
    }

    if ((Md1_Cfg & LSM6DSL_ACC_GYRO_INT1_TILT_MASK) || (Md2_Cfg & LSM6DSL_ACC_GYRO_INT2_TILT_MASK))
    {
        if ((Func_Src & LSM6DSL_ACC_GYRO_TILT_EV_STATUS_MASK))
        {
            status->TiltStatus = 1;
        }
    }

    return LSM6DSL_STATUS_OK;
}


/**
 * @brief Read the data from register
 * @param reg register address
 * @param data register data
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_ReadReg( uint8_t reg, uint8_t *data )
{

  if ( LSM6DSL_ACC_GYRO_ReadReg( NULL, reg, data, 1 ) == MEMS_ERROR )
  {
    return LSM6DSL_STATUS_ERROR;
  }

  return LSM6DSL_STATUS_OK;
}

/**
 * @brief Write the data to register
 * @param reg register address
 * @param data register data
 * @retval LSM6DSL_STATUS_OK in case of success, an error code otherwise
 */
LSM6DSLStatusTypeDef LSM6DSLSensor_WriteReg( uint8_t reg, uint8_t data )
{

  if ( LSM6DSL_ACC_GYRO_WriteReg( NULL, reg, &data, 1 ) == MEMS_ERROR )
  {
    return LSM6DSL_STATUS_ERROR;
  }

  return LSM6DSL_STATUS_OK;
}


LSM6DSLStatusTypeDef LSM6DSLSensor_GetTemp(uint16_t *temp)
{
    uint8_t temp_low;
    uint8_t temp_high;

    if (LSM6DSLSensor_ReadReg(LSM6DSL_ACC_GYRO_OUT_TEMP_L, &temp_low) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    if (LSM6DSLSensor_ReadReg(LSM6DSL_ACC_GYRO_OUT_TEMP_L, &temp_high) == LSM6DSL_STATUS_ERROR)
    {
        return LSM6DSL_STATUS_ERROR;
    }

    *temp = (temp_high << 8) | temp_low;
    return LSM6DSL_STATUS_OK;
}