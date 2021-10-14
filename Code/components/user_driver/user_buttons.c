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

/***********************************************************************************************************************
* Private global variables and functions
***********************************************************************************************************************/
static uint32_t u32ButtonData[MAX_BTN_SUPPORT] = {0};
static uint32_t u32ButtonPressEvent;
static uint32_t u32ButtonReleaseEvent;
static uint32_t u32ButtonHoldEvent = 0;
static uint32_t u32ButtonHoldEventExec = 0;
static uint32_t u32ButtonOnHoldEvent = 0;
static uint8_t u8ButtonCount;
static uint8_t u8ButtonStateUpdateFlag;
static uint32_t pu32ButtonHoldTimeCount[MAX_BTN_SUPPORT] = {0};
static uint32_t pu32ButtonOnHoldTimeCount[MAX_BTN_SUPPORT] = {0};
static uint32_t pu32ButtonLastPressTime[MAX_BTN_SUPPORT] = {0};
static uint8_t u8ButtonPressCount[MAX_BTN_SUPPORT] = {0};
static tsButtonConfig *hwParams;
static pHardButtonEventHandler tsCallbackTable[E_EVENT_HARD_MAX] = {0};
static void *pvCustomData[E_EVENT_HARD_MAX] = {0};
static uint32_t (*gettick)(void);

static uint32_t u32ButtonHasHoldEvent = 0;
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
void vHardButtonInit(tsButtonConfig *params, uint8_t u8BtnCount)
{
    uint8_t i = 0;

    if (NULL == params)
    {
        APP_LOGE("Invalid init pointer");
        return;
    }

    if (MAX_BTN_SUPPORT < u8BtnCount)
    {
        APP_LOGE("Not support %d buttons", u8BtnCount);
        return;
    }

    hwParams = params;

    u8ButtonCount = u8BtnCount;

    for (i = 0; i < u8BtnCount; i++)
    {
        /* Set Idle state */
        hwParams[i].u8BtnLastState = hwParams[i].u32IdleLevel;
        u32ButtonData[i] = hwParams[i].u32IdleLevel;
    }
    u8ButtonStateUpdateFlag = 1;
}

void vHardButtonSetGetTickCallback(uint32_t (*gettickCb)(void))
{
    gettick = gettickCb;
}

void buttons_process(void *params)
{
    uint8_t i = 0;

    if (NULL == hwParams)
    {
        APP_LOGE("Invalid init pointer");
        return;
    }

    if (MAX_BTN_SUPPORT < u8ButtonCount)
    {
        APP_LOGE("Not support %d buttons", u8ButtonCount);
        return;
    }

    if (u8ButtonStateUpdateFlag)
    {
        // u8ButtonStateUpdateFlag = 1;

        for (i = 0; i < u8ButtonCount; i++)
        {
            u32ButtonData[i] = gpio_get_level(hwParams[i].button_pin);
            if (u32ButtonData[i] ^ hwParams[i].u8BtnLastState)
            {
                // APP_LOGD("u32ButtonData[i] = %d", u32ButtonData[i]);
                if (u32ButtonData[i] != hwParams[i].u32IdleLevel)
                {
                    u32ButtonPressEvent |= 1 << i;
                    pu32ButtonHoldTimeCount[i] = gettick();
                }
                else
                {
                    u32ButtonReleaseEvent |= 1 << i;
                    pu32ButtonHoldTimeCount[i] = 0;
                    pu32ButtonOnHoldTimeCount[i] = 0;
                }
            }
            hwParams[i].u8BtnLastState = u32ButtonData[i];
        }
    }

    for (i = 0; i < u8ButtonCount; i++)
    {
        if (u32ButtonData[i] != hwParams[i].u32IdleLevel)
        {
            if (gettick() - pu32ButtonHoldTimeCount[i] >= BUTTON_HOLD_TIME)
            {
                if (((u32ButtonHoldEvent >> i) & 0x1))
                {
                    if (gettick() - pu32ButtonOnHoldTimeCount[i] >= BUTTON_ON_HOLD_TIME_FIRE_EVENT)
                    {
                        pu32ButtonOnHoldTimeCount[i] = gettick();
                        u32ButtonOnHoldEvent |= (1 << i);
                        u32ButtonHasHoldEvent |= (1 << i);
                    }
                }
                else
                {
                    pu32ButtonOnHoldTimeCount[i] = gettick();
                    u32ButtonHoldEvent |= (1 << i);
                    u32ButtonHoldEventExec &= ~(1 << i); // clear executed flag
                }
            }
        }

        if ((u32ButtonPressEvent >> i) & 0x01)
        {
            if (pu32ButtonLastPressTime[i] == 0)
            {
                pu32ButtonLastPressTime[i] = gettick();
            }

            if (gettick() - pu32ButtonLastPressTime[i] >= BUTTON_DOUBLE_CLICK_TIME)
            {
                u8ButtonPressCount[i] = 0;
            }
            else
            {
                u8ButtonPressCount[i] += 1;

                uint32_t u32Event = E_EVENT_HARD_MAX;

                if (u8ButtonPressCount[i] == 1) // double click
                {
                    u32Event = E_EVENT_HARD_BUTTON_DOUBLE_CLICK;
                }
                else if (u8ButtonPressCount[i] == 2)
                {
                    u32Event = E_EVENT_HARD_BUTTON_TRIPLE_CLICK;
                    u8ButtonPressCount[i] = 0;
                }

                if (u32Event != E_EVENT_HARD_MAX)
                {
                    // VSM_DEBUG_INFO("Hard button event[%d], button idx [%d]", u32Event, i);
                    if (NULL != tsCallbackTable[u32Event])
                    {
                        tsCallbackTable[u32Event](i, u32Event, pvCustomData[u32Event]);
                    }
                }
            }

            pu32ButtonLastPressTime[i] = gettick();

            if (NULL != tsCallbackTable[E_EVENT_HARD_BUTTON_PRESS])
            {
                tsCallbackTable[E_EVENT_HARD_BUTTON_PRESS](i, E_EVENT_HARD_BUTTON_PRESS, pvCustomData[E_EVENT_HARD_BUTTON_PRESS]);
            }
            u32ButtonPressEvent &= ~(1 << i);
        }
        if ((u32ButtonReleaseEvent >> i) & 0x01)
        {
            // APP_DEBUG("Hard button release event, button idx [%d]", i);
            if (NULL != tsCallbackTable[E_EVENT_HARD_BUTTON_RELEASE])
            {
                tsCallbackTable[E_EVENT_HARD_BUTTON_RELEASE](i, E_EVENT_HARD_BUTTON_RELEASE, (void *)u32ButtonHasHoldEvent);
            }
            u32ButtonReleaseEvent &= ~(1 << i);
            u32ButtonHoldEvent &= ~(1 << i); // clear
            uint8_t buffer_value = 0;
            buffer_value = (1 << i);
            u32ButtonHasHoldEvent &= !buffer_value;
        }

        if (!((u32ButtonHoldEventExec >> i) & 0x1) && ((u32ButtonHoldEvent >> i) & 0x1))
        {
            u32ButtonHoldEventExec |= (1 << i);
            // APP_LOGD("E_EVENT_HARD_BUTTON_HOLD exec event %d %d", i, E_EVENT_HARD_BUTTON_HOLD);
            uint32_t u32HoldTime = gettick() - pu32ButtonHoldTimeCount[i];
            if (NULL != tsCallbackTable[E_EVENT_HARD_BUTTON_HOLD])
            {
                tsCallbackTable[E_EVENT_HARD_BUTTON_HOLD](i, E_EVENT_HARD_BUTTON_HOLD, (void *)u32HoldTime);
            }
        }

        if ((u32ButtonOnHoldEvent >> i) & 0x1)
        {
            uint32_t u32HoldTime = gettick() - pu32ButtonHoldTimeCount[i];
            // APP_LOGD(" E_EVENT_HARD_BUTTON_ON_HOLD exec event %d %d", i, E_EVENT_HARD_BUTTON_ON_HOLD);
            if (NULL != tsCallbackTable[E_EVENT_HARD_BUTTON_ON_HOLD])
            {
                tsCallbackTable[E_EVENT_HARD_BUTTON_ON_HOLD](i, E_EVENT_HARD_BUTTON_ON_HOLD, (void *)u32HoldTime);
            }
            u32ButtonOnHoldEvent &= ~(1 << i);
        }
    }
}

void vHardButtonSetCallback(eHardButtonEventType event, pHardButtonEventHandler cb, void *data)
{
    if (NULL != cb)
    {
        tsCallbackTable[event] = cb;
        pvCustomData[event] = data;
    }
}

/***********************************************************************************************************************
* End of file
***********************************************************************************************************************/
