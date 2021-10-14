/*
 * led_driver.c
 *
 *  Created on: Jan 9, 2021
 *      Author: ductu
 */
/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "imu_read_task.h"
// #include "../user_driver/LSM6DSL_ACC_GYRO_Driver.h"
#include "../user_driver/LSM6DSLSensor.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
#define GYRO_THRESS_HIT_DETECT 4.00
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static void imu_task(void *pvParameters);
static void display_6D(void);
static bool hammer_hit_detection(int32_t acc_z);
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void imu_read_task(void)
{
    xTaskCreatePinnedToCore(imu_task, "imu_task", 4 * 1024, NULL, 3 | portPRIVILEGE_BIT, NULL, 1);
}
/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void imu_task(void *pvParameters)
{
    uint8_t buffer_who_am_i = 0x00;
    int32_t accelerometer[3];
    vTaskDelay(1000);
    // init acc sensor
    LSM6DSLStatusTypeDef ret_1 = LSM6DSLSensor_begin();
    if (ret_1 == LSM6DSL_STATUS_ERROR)
        APP_LOGE("Init LSM6DSL err");
    else if (ret_1 == LSM6DSL_STATUS_OK)
        APP_LOGI("Init LSM6DSL oke done");
    ret_1 = LSM6DSLSensor_ReadID(&buffer_who_am_i);
    if (ret_1 == LSM6DSL_STATUS_ERROR)
        APP_LOGE("ID err");
    else if (ret_1 == LSM6DSL_STATUS_OK)
        APP_LOGI("ID ok : %x", buffer_who_am_i);
    LSM6DSLSensor_Enable_X();
    bool hit;
    while (1)
    {
        LSM6DSLSensor_Get_X_Axes(accelerometer);
        // printf("Acc z[mg]: %d.%d\r\n", accelerometer[2] / 1000, abs((accelerometer[2] % 1000)));
        if (deive_data.sensor.buttons_hold == true)
        {
            hit = hammer_hit_detection(accelerometer[2]);
            if (hit == true)
            {
                APP_LOGI("hit detection");
                deive_data.sensor.hammer_detect = true;
                deive_data.sensor.vibration_active = true;
            }
        }
        memset(accelerometer, 0x00, sizeof(accelerometer));

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
uint32_t hit_detect_time = 0;
uint8_t hit_detect_state = 0;
static bool hammer_hit_detection(int32_t acc_z)
{
    bool status = false;
    switch (hit_detect_state)
    {
    case 0:
        if (abs(acc_z) > (GYRO_THRESS_HIT_DETECT * 1000))
        {
            hit_detect_state = 1;
            hit_detect_time = usertimer_gettick();
        }
        break;
    case 1:
        if (usertimer_gettick() - hit_detect_time > 300)
        {
            hit_detect_state = 0;
            status = true;
        }
        break;
    default:
        break;
    }
    return status;
}
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void display_6D(void)
{
    char report[256];
    uint8_t xl = 0;
    uint8_t xh = 0;
    uint8_t yl = 0;
    uint8_t yh = 0;
    uint8_t zl = 0;
    uint8_t zh = 0;

    LSM6DSLSensor_Get_6D_Orientation_XL(&xl);
    LSM6DSLSensor_Get_6D_Orientation_XH(&xh);
    LSM6DSLSensor_Get_6D_Orientation_YL(&yl);
    LSM6DSLSensor_Get_6D_Orientation_YH(&yh);
    LSM6DSLSensor_Get_6D_Orientation_ZL(&zl);
    LSM6DSLSensor_Get_6D_Orientation_ZH(&zh);

    if (xl == 0 && yl == 0 && zl == 0 && xh == 0 && yh == 1 && zh == 0)
    {
        sprintf(report, "\r\n  ________________  "
                        "\r\n |                | "
                        "\r\n |  *             | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |________________| \r\n");
    }

    else if (xl == 1 && yl == 0 && zl == 0 && xh == 0 && yh == 0 && zh == 0)
    {
        sprintf(report, "\r\n  ________________  "
                        "\r\n |                | "
                        "\r\n |             *  | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |________________| \r\n");
    }

    else if (xl == 0 && yl == 0 && zl == 0 && xh == 1 && yh == 0 && zh == 0)
    {
        sprintf(report, "\r\n  ________________  "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |  *             | "
                        "\r\n |________________| \r\n");
    }

    else if (xl == 0 && yl == 1 && zl == 0 && xh == 0 && yh == 0 && zh == 0)
    {
        sprintf(report, "\r\n  ________________  "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |                | "
                        "\r\n |             *  | "
                        "\r\n |________________| \r\n");
    }

    else if (xl == 0 && yl == 0 && zl == 0 && xh == 0 && yh == 0 && zh == 1)
    {
        sprintf(report, "\r\n  __*_____________  "
                        "\r\n |________________| \r\n");
    }

    else if (xl == 0 && yl == 0 && zl == 1 && xh == 0 && yh == 0 && zh == 0)
    {
        sprintf(report, "\r\n  ________________  "
                        "\r\n |________________| "
                        "\r\n    *               \r\n");
    }

    else
    {
        sprintf(report, "None of the 6D orientation axes is set in LSM6DSL - accelerometer.\r\n");
    }
    printf("%s", report);
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
