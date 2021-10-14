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
#include "../user_driver/user_leds.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/
#define BOARD_BTN_CONFIG                                                 \
	{                                                                    \
		/*Last state   Idle level    Btn Type   pin   Callback */        \
		{0, 1, 1, GPIO_USER_BOOT_BUTTON}, /* Rotary Encoder Channel A */ \
			{0, 1, 1, GPIO_USER_BUTTON},  /* Rotary Encoder Channel B */ \
	}
/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static void PlantControl_Task(void *pvParameters);
// static void user_buttons_callback(uint8_t _button_number, e_BUTTON_EVENT _event);
static void user_buttons_setup(void);
static void vsm_btn_event_press(int btn_idx, int event, void *p);
static void vsm_btn_event_release(int btn_idx, int event, void *p);
static void vsm_btn_event_hold(int btn_idx, int event, void *p);
static void vsm_btn_reverse_click(void);
static void vsm_button_server_access(void);

static tsButtonConfig btnParams[] = BOARD_BTN_CONFIG;
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
	uint8_t _old_vibration_level = deive_data.sensor.vibration_level;
	// buttons_set_callback(user_buttons_callback);
	user_buttons_setup();
	led_green(true);

	while (1)
	{
		buttons_process(NULL);
		vsm_btn_reverse_click();
		if ((mqtt_start_first_time == false) && (deive_data.wifi_status == true))
		{
			mqtt_start_first_time = true;
		}
		vsm_button_server_access();
		// process vibration message json mqtt
		if (deive_data.sensor.vibration_level != _old_vibration_level)
		{
			_old_vibration_level = deive_data.sensor.vibration_level;
			vibration_set_duty(deive_data.sensor.vibration_level);
		}
		// check button here
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

static void user_buttons_setup(void)
{
	vHardButtonSetGetTickCallback(usertimer_gettick);
	vHardButtonInit(btnParams, 2);
	vHardButtonSetCallback(E_EVENT_HARD_BUTTON_PRESS, vsm_btn_event_press, NULL);
	vHardButtonSetCallback(E_EVENT_HARD_BUTTON_RELEASE, vsm_btn_event_release, NULL);
	vHardButtonSetCallback(E_EVENT_HARD_BUTTON_HOLD, vsm_btn_event_hold, NULL);
	// vHardButtonSetCallback(E_EVENT_HARD_BUTTON_DOUBLE_CLICK, vsm_btn_multi_click, NULL);
	// vHardButtonSetCallback(E_EVENT_HARD_BUTTON_ON_HOLD, vsm_btn_event_onhold, NULL);
}

uint32_t _time_check_reverse_click = 0;
uint8_t _check_state_reverse_click = 0;
uint8_t _buttons_press_release_old_state;
typedef struct
{
	bool _press_release;
	bool _hold;
	bool _hold_3s;
	bool _hold_1s;
	bool _click;
	bool _reverse;
} user_button_check_t;

user_button_check_t user_button_check;

static void vsm_btn_reverse_click(void)
{
	if (user_button_check._hold == true)
	{
		user_button_check._hold = false;
		_time_check_reverse_click = usertimer_gettick();
		_check_state_reverse_click = 1;
		APP_LOGI("button hold down");
	}
	if ((_buttons_press_release_old_state != user_button_check._press_release) &&
		(user_button_check._press_release == false) &&
		(_check_state_reverse_click == 0))
	{
		user_button_check._click = true;
	}
	else if ((user_button_check._press_release == false) && (_check_state_reverse_click == 1))
	{
		user_button_check._hold_1s = true;
		_check_state_reverse_click = 0;
	}
	else if ((usertimer_gettick() - _time_check_reverse_click > 2000) && (_check_state_reverse_click == 1))
	{
		user_button_check._hold_3s = true;
	}

	if ((user_button_check._hold_3s == true) && (user_button_check._press_release == false)) /// hold buttons in 3s
	{
		user_button_check._hold_3s = false;
		_check_state_reverse_click = 2;
		_time_check_reverse_click = usertimer_gettick();
	}

	if (_check_state_reverse_click == 2)
	{
		if (usertimer_gettick() - _time_check_reverse_click > 500)
		{
			_check_state_reverse_click = 0;
			// user_button_check._hold_1s = true;
		}
		else if (user_button_check._press_release != _buttons_press_release_old_state)
		{
			_time_check_reverse_click = usertimer_gettick();
			user_button_check._reverse = true;
			user_button_check._hold_1s = false;
		}
	}
	_buttons_press_release_old_state = user_button_check._press_release;
}

static void vsm_button_server_access(void)
{
	if (user_button_check._reverse == true)
	{
		APP_LOGI("send reverse to sever");
		user_button_check._reverse = false;
	}
	else if (user_button_check._hold_1s == true)
	{
		user_button_check._hold_1s = false;
		APP_LOGI("button hold up");
		APP_LOGI("strong vibration");
		vibration_set_duty(50);
	}
	else if (user_button_check._click)
	{
		user_button_check._click = false;
		APP_LOGI("click");
	}
}

/***********************************************************************************************************************
* Function Name: vsm_btn_event_release
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
static void vsm_btn_event_press(int btn_idx, int event, void *p)
{
	switch (btn_idx)
	{
	case 0:
		break;
	case 1:
		user_button_check._press_release = true;
		break;
	default:
		break;
	}
}

static void vsm_btn_event_release(int btn_idx, int event, void *p)
{
	switch (btn_idx)
	{
	case 0:
		break;
	case 1:
		user_button_check._press_release = false;
		break;
	default:
		break;
	}
}

static void vsm_btn_event_hold(int btn_idx, int event, void *p)
{
	switch (btn_idx)
	{
	case 0:
		APP_LOGI("factory reset after 2s");
		flash_erase_all_partions();
		vTaskDelay(2000);
		esp_restart();
		break;
	case 1:
		user_button_check._hold = true;
		_time_check_reverse_click = usertimer_gettick();
		deive_data.sensor.buttons_hold = true;
		break;
	default:
		break;
	}
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
