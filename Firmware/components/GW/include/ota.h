/** @file
 * @brief Over the Air update
 */
 

#ifndef _ota_H_
#define _ota_H_

#include <stdio.h>
#include <string.h>
// #include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "Mqtt.h"
#include "led.h"
#include "Conf.h"

/**
 * @brief Semaphore vor ota handling
 * 
 * If this semaphore is given. The ota script will start the update.
 * This has the advantage, that the semaphore can be given from different parts of the code,
 * like button press or via MQTT. But the ota function can only take the semaphore one time.
 * Therefore, a second ota process can not be started if one is running.
 */ 
extern SemaphoreHandle_t ota_semaphore;

/**
 * @brief Run the Ota script and wait for the semaphore
 * This function runs as a Task and is started during the intialisation.
 * The update process will start if the semaphore is aviable.
 */
void run_ota(void *params);

/**
 * @brief Initialisation of the ota process.
 * 
 * @param[in] input: The datalink structure
 */
void ota(datalink_t* input);

/**
 * @brief Starts the ota proces
 * Deprecated the semaphore can be given in every function.
 * 
 * @todo remove this function
 */
void on_button_pushed(void *params);

#endif  
 