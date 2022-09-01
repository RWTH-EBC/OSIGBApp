/** @file
 *  @brief Handling JSON Messages
 * 
 * The cJSON library is used.
 * This is necessary to crypt and encrypt the messages for sending via MQTT.
 */

#ifndef _cJSON_parse_H_
#define _cJSON_parse_H_

#include "cJSON.h"
#include "esp_log.h"
#include "Conf.h"
#include "AIO_Board.h"
#include "sntp.h"

/**
 * @brief Commands to change settings on the gateway via MQTT
 * 
 */
typedef enum {
	empty = 0,
	update,
	restart,
	reset_time
} JSON_command_t;

/**
 * @brief Struct for storing the variables of a swop message
 * 
 * SWOP is a JSON type, defined by aedifion
 */
typedef struct
{
	char typ[16];
	char dpt[128];
	char val[32];
	JSON_command_t cmd;
	int smpl_rate;
} swop_payload_t;
 
/** @brief cJSON Parsing incoming message
 * 
 *  @param incomingBuffer Message received via MQTT
 *  @param swop variable which returns the values of the JSON message
 */
void OnGotData(char *incomingBuffer, swop_payload_t* swop);

/**
 * @brief Generate the config request for fiware
 * 
 * This function generates the fiware configuration request.
 * With type subscription new change will be pushed automatically
 * to the gateway
 * @link https://github.com/telefonicaid/iotagent-json/blob/master/docs/usermanual.md#configuration-retrieval-1
 */
char *conf_req();

/** 
 * @brief Build the JSON Payload for Metadata to send to Aedifion
 */
char *get_Meta(uint8_t i, aio_conf_t *conf, datalink_t *input);

/**
 * @brief Parsing the incoming json message from the Fiware server
 * 
 * Parsing incoming message and update the configuration array. 
 * When the config is updated the esp will store it in NVS and restarts.
 * 
 * @param[in] incomingBuffer: The MQTT payload
 * @param[in] conf: The config array 
 */
void Read_Config(char *incomingBuffer, aio_conf_t *conf);

char *parse_JSON_out(char *in1_name, float val1, char *in2_name, float val2, char* timerbuf1);

#endif