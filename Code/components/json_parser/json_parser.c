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
#include "../../Common.h"
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
    cJSON *jOperation = cJSON_GetObjectItem(root2, "operation");
    cJSON *jId = cJSON_GetObjectItem(root2, "id");
    cJSON *value;
    char *operation = (char *)malloc(20 * sizeof(char));
    memset(operation, 0x00, 20 * sizeof(char));
    char *_id = (char *)malloc(20 * sizeof(char));
    memset(_id, 0x00, sizeof(char));
    if( jId)
    {
        _id = cJSON_GetObjectItem(root2, "id")->valuestring;
    }
    if( strcmp(_id, deive_data.mac_add) == 0 )
    {
        if (jOperation)
        {
            operation = cJSON_GetObjectItem(root2, "operation")->valuestring;
            if ((strcmp(operation, TYPE_COMMAND_SETTING) == 0))
            {
                value = cJSON_GetObjectItem(root2, "value");
                if (value)
                {
                    // if (cJSON_GetObjectItem(root2, "value")->valueint != 0)
                    deive_data.sensor.vibration_level = cJSON_GetObjectItem(root2, "value")->valueint;
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
            APP_LOGE("not include operation");
        }
    }
    else
    {
        APP_LOGE("not error id = %s", _id);
    }
    // cJSON_Delete(jOperation);
    cJSON_Delete(root2);
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
