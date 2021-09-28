/*
 * Common.h
 *
 *  Created on: Apr 24, 2021
 *      Author: ductu
 */

#ifndef MAIN_COMMON_H_
#define MAIN_COMMON_H_
/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "main.h"

enum
{
	E_LOG_LVL_NONE,
	E_LOG_LVL_INFO,
	E_LOG_LVL_ERROR,
	E_LOG_LVL_WARNING,
	E_LOG_LVL_DEBUG,
	E_LOG_LVL_NEVER
};

/* Console color */
#define RESET "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"
/*Background colors*/
#define BG_KOLORS_BLK "\x1b[40m" //Black
#define BG_KOLORS_RED "\x1b[41m" //Red
#define BG_KOLORS_GRN "\x1b[42m" //Green
#define BG_KOLORS_YEL "\x1b[43m" //Yellow
#define BG_KOLORS_BLU "\x1b[44m" //Blue

#define LOG_SHOULD_I(level) (level <= LOG_BUILD_LEVEL && level <= E_LOG_LVL_DEBUG)
#define LOG(level, tag, ...)                                       \
	do                                                             \
	{                                                              \
		if (LOG_SHOULD_I(level))                                   \
		{                                                          \
			printf("[%s] %s:%d: " RESET, tag, __func__, __LINE__); \
			printf(__VA_ARGS__);                                   \
			printf("\r\n");                                        \
		}                                                          \
	} while (0)

#define APP_LOGE(...) LOG(E_LOG_LVL_ERROR, KRED "ERROR" RESET, __VA_ARGS__)
#define APP_LOGI(...) LOG(E_LOG_LVL_INFO, KGRN "INFOR" RESET, __VA_ARGS__)
#define APP_LOGD(...) LOG(E_LOG_LVL_DEBUG, KYEL "DEBUG" RESET, __VA_ARGS__)
#define APP_LOGW(...) LOG(E_LOG_LVL_WARNING, BG_KOLORS_YEL "ALARM" RESET, __VA_ARGS__)

#define LOG_BUILD_LEVEL E_LOG_LVL_DEBUG

#define GPIO_USER_BUTTON 0
#define GPIO_INPUT_PIN_SEL (1ULL << GPIO_USER_BUTTON)


#define GPIO_USER_LED_GREEN 16
#define GPIO_USER_LED_RED 17
#define GPIO_USER_LED_BLUE 4

#define GPIO_OUTPUT_PIN_SEL ((1ULL<<GPIO_USER_LED_GREEN) | (1ULL<<GPIO_USER_LED_RED) | (1ULL<<GPIO_USER_LED_BLUE)) 
typedef union
{
	uint8_t sum_error;
	struct
	{
		uint8_t rever_bit : 1;
		uint8_t laser_error : 1;
		uint8_t leser_alarm : 1;
		uint8_t high_temp : 1;
		uint8_t low_temp : 1;
		uint8_t fan_error : 1;
		uint8_t fan_speed_compensation_start : 1;
		uint8_t fan_speed_alarm : 1;
	};
} dust_error_t;

typedef struct
{
	float humidiy;
	float temperature;
	uint16_t pm25;
	uint16_t pm10;
	bool rain_status;
	float rain_metter;

	bool dust_error;
	bool sht3x_error;
	bool rain_error;
} sensor_data_t;

typedef struct
{
	bool wifi_status;
	bool mqtt_status;
	sensor_data_t sensor;
} deive_data_t;

typedef struct
{
	int time_interval_send_data;
	int time_interval_check_ota;
	char jobId[32];
	char mqtt_topic_pub[100];
	char mqtt_topic_pub_err[100];
	char mqtt_topic_jobsub[100];
	char mqtt_topic_jobpub[100];

} mqtt_config_t;

extern mqtt_config_t mqtt_config;
extern deive_data_t deive_data;

uint32_t usertimer_gettick(void);
#endif /* MAIN_COMMON_H_ */
