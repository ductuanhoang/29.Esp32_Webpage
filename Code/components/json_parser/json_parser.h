/*
 * json_parser.h
 *
 *  Created on: Nov 16, 2020
 *      Author: Yolo
 */

#ifndef MAIN_JSON_PARSER_JSON_PARSER_H_
#define MAIN_JSON_PARSER_JSON_PARSER_H_

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include "../../Common.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
typedef enum
{
	ENUM_NONE,
	ENUM_MESSAGE_SMART_CONFIG,
	ENUM_MESSAGE_CONTROL_IR,
	ENUM_MESSAGE_CONTROL_RAW,
	ENUM_MESSAGE_MODE_LEARN,
	ENUM_MESSAGE_MODE_TEST_DEVICE,
	ENUM_MESSAGE_NEW_IR_LEARN,
	ENUM_MESSAGE_RESPONSE
} enum_type_message;

/****************************************************************************/
/***         Exported global functions                                     ***/
/****************************************************************************/
bool json_parser_job(const char *message, uint16_t length);

// char *json_packet_message_sensor(void);
void json_packet_message_sensor(char *message_packet);

void json_packet_message_error(char *message_packet, uint8_t sensor_error);

#endif /* MAIN_JSON_PARSER_JSON_PARSER_H_ */
