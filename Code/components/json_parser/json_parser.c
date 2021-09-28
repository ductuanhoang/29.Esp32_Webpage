/*
 * json_parser.c
 *
 *  Created on: Nov 16, 2020
 *      Author: Yolo
 */

/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "json_parser.h"
#include "cJson_lib/cJSON.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// #include "../Interface/Logger_File/logger_file.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
static char error_table[5][6] =
    {
        "E0001" // Không có mạng
        "E0002" // Không kết nối được MQTT
        "E0003" // Lỗi cảm biến nhiệt độ/độ ẩm
        "E0004" // Lỗi cảm biến bụi
        "E0005" // Lỗi cảm biến mưa
};
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
typedef enum
{
    E_JOB_ID_01 = 1,
    E_JOB_ID_02 = 2
} job_id_t;
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
#define TYPE_COMMAND_SETTING "setting"
#define TYPE_COMMAND_RESTART "restart"
/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
extern void flash_save_data(void);
/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/
int old_time_interval_check_ota = 0;
int old_time_interval_send_data = 0;
/***********************************************************************************************************************
* Function Name:
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
bool json_parser_job(const char *message, uint16_t length)
{
    APP_LOGD("Serialize.....");

    bool status = true;
    cJSON *root2 = cJSON_ParseWithLength(message, length);
    cJSON *jobId = cJSON_GetObjectItem(root2, "jobId");
    cJSON *value;
    cJSON *jobDocument;
    char *operation = (char *)malloc(10 * sizeof(char));
    memset(operation, 0x00, 10 * sizeof(char));

    if (jobId)
    {
        jobDocument = cJSON_GetObjectItem(root2, "jobDocument");
        //
        if (jobDocument)
        {
            operation = cJSON_GetObjectItem(jobDocument, "operation")->valuestring;
            if ((strcmp(operation, TYPE_COMMAND_SETTING) == 0))
            {
                value = cJSON_GetObjectItem(jobDocument, "value");
                if (value)
                {
                    old_time_interval_check_ota = mqtt_config.time_interval_check_ota;
                    old_time_interval_send_data = mqtt_config.time_interval_send_data;

                    if (cJSON_GetObjectItem(value, "INTERVAL_CHECK_OTA")->valueint != 0)
                        mqtt_config.time_interval_check_ota = cJSON_GetObjectItem(value, "INTERVAL_CHECK_OTA")->valueint;
                    if (cJSON_GetObjectItem(value, "INTERVAL_UPDATE")->valueint != 0)
                        mqtt_config.time_interval_send_data = cJSON_GetObjectItem(value, "INTERVAL_UPDATE")->valueint;

                    if ((old_time_interval_check_ota != mqtt_config.time_interval_send_data) |
                        (old_time_interval_send_data != mqtt_config.time_interval_send_data))
                    {
                        flash_save_data();
                    }
                    APP_LOGI("time check ota config = %d", mqtt_config.time_interval_check_ota);
                    APP_LOGI("time check update data config = %d", mqtt_config.time_interval_send_data);
                }
                else
                {
                    APP_LOGD("unknow value setting");
                    status = false;
                }
            }
            else if ((strcmp(operation, TYPE_COMMAND_RESTART) == 0))
            {
                // restart device
            }
            else
            {
                APP_LOGD("unknow commnad");
                status = false;
            }
        }
    }
    else
    {
        APP_LOGD("thing_token have'nt jobID");
        status = false;
    }

    // coppy to JobId
    if (status == true)
    {
        sprintf(mqtt_config.jobId, "%s", cJSON_GetObjectItem(root2, "jobId")->valuestring);
        APP_LOGI("mqtt_config.jobId = %s", mqtt_config.jobId);
    }
    APP_LOGI("end process 1");
    // cJSON_Delete(value);
    // free(operation);
    // cJSON_Delete(jobDocument);
    // cJSON_Delete(jobId);
    cJSON_Delete(root2);
    APP_LOGI("end process 2");
    return status;
}

/***********************************************************************************************************************
* Function Name:
* Description  :
 {
   "currentSensorStateData":
   {
     "measure_rain": 1,
     "meter_rain": 1.2,
     "measure_pm25": 30,
     "measure_temperature": 23,
     "measure_humidity": 50
   }
 }
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/

// char *json_packet_message_sensor(void)
void json_packet_message_sensor(char *message_packet)
{
    cJSON *root = NULL;
    cJSON *subroot = NULL;
    root = cJSON_CreateObject();
    subroot = cJSON_AddObjectToObject(root, "currentSensorStateData");

    cJSON_AddNumberToObject(subroot, "measure_rain", deive_data.sensor.rain_status);
    cJSON_AddNumberToObject(subroot, "meter_rain", deive_data.sensor.rain_metter);
    cJSON_AddNumberToObject(subroot, "measure_pm25", deive_data.sensor.pm25);
    cJSON_AddNumberToObject(subroot, "measure_pm100", deive_data.sensor.pm10);
    cJSON_AddNumberToObject(subroot, "measure_temperature", deive_data.sensor.temperature);
    cJSON_AddNumberToObject(subroot, "measure_humidity", deive_data.sensor.humidiy);

    // APP_LOGD("message = %s", cJSON_PrintUnformatted(root));

    sprintf(message_packet, "%s", cJSON_PrintUnformatted(root));
    cJSON_free(subroot);
    cJSON_free(root);
    // return cJSON_PrintUnformatted(root);
}

void json_packet_message_error(char *message_packet, uint8_t sensor_error)
{
    cJSON *root = NULL;
    root = cJSON_CreateObject();

    if (sensor_error == 1)
        cJSON_AddStringToObject(root, "errorCode", "E0004"); // dust
    else if (sensor_error == 2)
        cJSON_AddStringToObject(root, "errorCode", "E0003"); // sht
    else if (sensor_error == 3)
        cJSON_AddStringToObject(root, "errorMsg", "E0005"); // rain
    cJSON_AddStringToObject(root, "errorMsg", "Unable to install update");
    APP_LOGD("message = %s", cJSON_PrintUnformatted(root));
    sprintf(message_packet, "%s", cJSON_PrintUnformatted(root));
    cJSON_free(root);
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
