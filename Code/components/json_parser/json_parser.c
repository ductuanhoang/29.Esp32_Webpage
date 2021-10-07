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

/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/

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
                    if (cJSON_GetObjectItem(value, "VIBRATION_VALUE")->valueint != 0)
                        deive_data.sensor.vibration_level = cJSON_GetObjectItem(value, "VIBRATION_VALUE")->valueint;

                    APP_LOGI("vibration control = %d", deive_data.sensor.vibration_level);
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
        else
        {
            APP_LOGD("unknow jobDocument");
        }
    }
    else
    {
        APP_LOGD("thing_token have'nt jobID");
        status = false;
    }

    // coppy to JobId
    // if (status == true)
    // {
    //     sprintf(mqtt_config.jobId, "%s", cJSON_GetObjectItem(root2, "jobId")->valuestring);
    //     APP_LOGI("mqtt_config.jobId = %s", mqtt_config.jobId);
    // }
    // APP_LOGI("end process 1");
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
   "smarthammerdata":
   {
     "vibration": 1,
     "acc_detect": 0
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
    subroot = cJSON_AddObjectToObject(root, deive_data.mac_add);

    cJSON_AddNumberToObject(subroot, "vibration", deive_data.sensor.vibration_level);
    cJSON_AddNumberToObject(subroot, "acc_detect", deive_data.sensor.hammer_detect);

    // APP_LOGD("message = %s", cJSON_PrintUnformatted(root));

    sprintf(message_packet, "%s", cJSON_PrintUnformatted(root));
    cJSON_free(subroot);
    cJSON_free(root);
    // return cJSON_PrintUnformatted(root);
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
