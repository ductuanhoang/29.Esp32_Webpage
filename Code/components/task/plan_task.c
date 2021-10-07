/*
 * plan_task.c
 *
 *  Created on: Jan 7, 2021
 *      Author: ductu
 */
/***********************************************************************************************************************
* Pragma directive
***********************************************************************************************************************/

/***********************************************************************************************************************
* Includes <System Includes>
***********************************************************************************************************************/
#include "mqtt_task.h"
#include "plan_task.h"
#include "../../Common.h"
#include "driver/gpio.h"
#include "../user_driver/user_buttons.h"
#include "../user_driver/user_vibration_motor.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static void PlantControl_Task(void *pvParameters);
static void user_buttons_callback(uint8_t _button_number, e_BUTTON_EVENT _event);

/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/
extern void flash_erase_all_partions(void);
/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void plan_task(void)
{
	xTaskCreatePinnedToCore(PlantControl_Task, "plant_task", 6 * 1024, NULL, 2 | portPRIVILEGE_BIT, NULL, 1);
}

bool mqtt_start_first_time = false;
/***********************************************************************************************************************
* Static Functions
***********************************************************************************************************************/
static void PlantControl_Task(void *pvParameters)
{
	buttons_set_callback(user_buttons_callback);
	while (1)
	{
		buttons_process();
		if ((mqtt_start_first_time == false) && (deive_data.wifi_status == true))
		{
			mqtt_start_first_time = true;
		}
		// check button here
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}


static void user_buttons_callback(uint8_t _button_number, e_BUTTON_EVENT _event)
{
	if (_event == kBUTTONS_EVENT_PRESS)
	{
		APP_LOGI("buttons call = %d -- %d", _button_number, _event);
		deive_data.sensor.hammer_detect = 1;
		vibration_set_duty(deive_data.sensor.vibration_level);
	}
	else if( _event == kBUTTONS_EVENT_HOLD)
	{
		flash_erase_all_partions();
		esp_restart();
		APP_LOGI("Erase all flash and smart config again");
	}

}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
