#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "../../Common.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define MAX_BTN_SUPPORT (5u)
#define PERIOD_SCAN_IN_MS (200)
#define BUTTON_HOLD_TIME (500)
#define BUTTON_HOLD_TIME_2 (3000)

#define BUTTON_ON_HOLD_TIME_FIRE_EVENT (200)
#define IDLE_TIME_COUNT_IN_MS (30000)
#define BUTTON_DOUBLE_CLICK_TIME (500)
#define BUTTON_PRESS_DEBOUND_TIME (50)
	/****************************************************************************/
	/***        Type Definitions                                              ***/
	/****************************************************************************/
	/**
 * Enum to define the event type of hard button
 */
	typedef enum
	{
		E_EVENT_HARD_BUTTON_PRESS = 0,
		E_EVENT_HARD_BUTTON_RELEASE,
		E_EVENT_HARD_BUTTON_HOLD,
		E_EVENT_HARD_BUTTON_ON_HOLD,
		E_EVENT_HARD_BUTTON_DOUBLE_CLICK,
		E_EVENT_HARD_BUTTON_TRIPLE_CLICK,
		E_EVENT_HARD_ILDE,
		E_EVENT_HARD_ILDE_BREAK,
		E_EVENT_HARD_MAX
	} eHardButtonEventType;

	typedef uint8_t (*p_btnReadValue)(uint8_t);

	typedef struct
	{
		uint8_t u8BtnLastState;
		uint32_t u32IdleLevel;
		uint8_t button_type; // hardware button = 1, ADC = 0, One level button = 2
		uint8_t button_pin;	 // Debound enable;
	} tsButtonConfig;

	/**
 * Event callback function type
 * button idx, event type, custom data
 */
	typedef void (*pHardButtonEventHandler)(int, int, void *);

	void vHardButtonInit(tsButtonConfig *params, uint8_t u8BtnCount);

	void vHardButtonSetCallback(eHardButtonEventType event, pHardButtonEventHandler cb, void *data);

	void vHardButtonSetGetTickCallback(uint32_t (*gettickCb)(void));

	/**
 * Event callback function type
 * button idx, event type, custom data
 */
	typedef void (*pHardButtonEventHandler)(int, int, void *);

	/****************************************************************************/
	/***         Exported global functions                                     ***/
	/****************************************************************************/
	void buttons_set_callback(pHardButtonEventHandler cb);

	void buttons_process(void *params);

	void buttons_gpio_init(void);
#ifdef __cplusplus
}
#endif
