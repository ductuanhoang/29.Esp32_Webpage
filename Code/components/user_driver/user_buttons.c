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
#include "user_buttons.h"
#include "../../Common.h"
/***********************************************************************************************************************
* Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
* Typedef definitions
***********************************************************************************************************************/
#define TIME_BUTTONS_DETEC 100 //ms
#define TIME_BUTTONS_HOLD_DETECT 3000 //ms
/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static void buttons_check(void);

/***********************************************************************************************************************
* Exported global variables and functions (to be accessed by other files)
***********************************************************************************************************************/


pHardButtonEventHandler _buttons_call_back = NULL;
/***********************************************************************************************************************
* Imported global variables and functions (from other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
* Function Name: 
* Description  :
* Arguments    : none
* Return Value : none
***********************************************************************************************************************/
void buttons_process(void)
{
    buttons_check();
}

void buttons_set_callback(pHardButtonEventHandler cb)
{
    if (cb != NULL)
        _buttons_call_back = cb;
    else APP_LOGE("error resgistor buttons");
}


void buttons_gpio_init(void)
{    
    gpio_config_t io_conf;

    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //bit mask of the pins, use GPIO0 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    esp_err_t error = gpio_config(&io_conf); //configure GPIO with the given settings

    if (error != ESP_OK)
    {
        APP_LOGE("error configuring inputs\n");
    }
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
static uint32_t time_buttons_previous = 0;
static uint8_t buttons_state = 0;
static void buttons_check(void)
{
    switch (buttons_state)
    {
    case 0:
        time_buttons_previous = usertimer_gettick();
        if (gpio_get_level(GPIO_USER_BUTTON) == 0)
        {
            buttons_state = 1;
        }
        break;
    case 1:
        if (gpio_get_level(GPIO_USER_BUTTON) == 0)
        {
            if (usertimer_gettick() - time_buttons_previous > TIME_BUTTONS_HOLD_DETECT)
            {
                // call back function
                _buttons_call_back(0, kBUTTONS_EVENT_HOLD);
                buttons_state = 0;
            }
        }
        else if (gpio_get_level(GPIO_USER_BUTTON) == 1)
        {
            if (usertimer_gettick() - time_buttons_previous > TIME_BUTTONS_DETEC)
            {
                // call back function
                _buttons_call_back(0, kBUTTONS_EVENT_PRESS);
                buttons_state = 0;
            }
            buttons_state = 0;
        }
        break;

    default:
        break;
    }
}
/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
