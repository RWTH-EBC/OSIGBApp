/** @file 
 * @brief Mqtt comunication
 * 
 * @todo Change Name to MQTT because this is not only related to AIO
 * @date Jan 2021
 * 
*/

#ifndef _AIO_MQTT_H_
#define _AIO_MQTT_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"


#include "esp_private/periph_ctrl.h"
//#include "driver/timer.h"
#include "driver/gptimer.h"
// #include "soc/timer_group_struct.h"
#include "Conf.h"
#include "AIO_Board.h"
#include "Netw.h"
#include "sntp.h"
#include "ota.h"

#define TIMER_DIVIDER 50000
#define TIMER_SCALE (APB_CLK_FREQ / TIMER_DIVIDER)
//#define ACK 0  // to distinguis between if it is  an ack or normal data puplish
//#define NACK 1

/// Deprecated @todo remove
extern SemaphoreHandle_t Metasem;

/**
 * @brief Semaphore for handling sending and
 * receiving messages
 * 
 * Stop sending actual state when the receiving
 * of an input message is proccessed  
 * 
 * @todo use different variables for sending and receiving
 * This semaphore was created for a poc of fiware. 
 * If different variables are used sending and receiving can be
 * done simultaneously 
 */
SemaphoreHandle_t mutex;


/**
 * @brief Disconnect from MQTT
 * 
 * Unsubscribe from topic
 * Disconnect and destroy Client
 * Stop Timer
 * 
 * This stops also the communication from the gateways with the connected
 * devices 
 */
void disconnect_mqtt();

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The encoding types for MQTT messages
 * 
 * ATM only AED is supported. This is for communication with the aedifion cloud server
 */
typedef enum
{
	MQTT_ENCODE_JSON = 0,
	MQTT_ENCODE_UL20,
	MQTT_ENCODE_AED
} mqtt_encode_t;

/**
 * @brief Sampling types
 * 
 */
typedef enum
{
	MQTT_LOG_PERIODIC = 0,
	MQTT_LOG_THRESHOLD
} mqtt_log_mode_t;


/**
 * @brief Variables for storing the MQTT payload 
 * 
 */
typedef struct
{
	char id[32];
	char key[32];
	char val[32];
	char str[512];
} mqtt_payload_t;

/**
 * @brief Function for initialisation of the MQTT functionality of the gateway
 * 
 * @param[in] aio_conf
 * @param[in] input
 * 
 * @return ESP_OK if success 
 */ 
esp_err_t init_MQTT(aio_conf_t *aio_conf, datalink_t* input_in);

/** 
 * @brief reports periodically the wifi signal strength via MQTT
 * 
 * @todo this was designed for testing the wifi range of the gateways. Should this function be removed?
 */
void wifi_signal();


/** 
 * @brief Sending the actual meta Tag via MQTT
 * This function is only suported with AED encoding.
 * 
 */
void publish_meta_tag();


#ifdef __cplusplus
}
#endif //__cplusplus

#endif