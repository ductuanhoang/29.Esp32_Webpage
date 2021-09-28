#pragma once


#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
typedef enum
{
	kBUTTONS_EVENT_PRESS,
	kBUTTONS_EVENT_HOLD
}e_BUTTON_EVENT;
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef void (*pHardButtonEventHandler)(int, e_BUTTON_EVENT);
/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/
void buttons_set_callback(pHardButtonEventHandler cb);

void buttons_process(void);

void buttons_gpio_init(void);
#ifdef __cplusplus
}
#endif