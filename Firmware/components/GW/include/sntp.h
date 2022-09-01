/** @file 
 * @brief Time Synchronisation
 * 
 */

#ifndef _AIO_SNPT_H_
#define _AIO_SNPT_H_


#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

/**
 * @brief Check if the time is set and returns unix time
 * 
 * @return unix Time
 */
int64_t gettime_ns();

/**
 * @brief Check if the time is set and update time if it is not set
 */
void gettime_meta(char *timevar);
/**
 * @brief Check if the time is set and update time if it is not set
 * 
 * @todo compare with gettime_meta maybe it is obsolete? 
 */
void refresh_time();

/**
 * @brief Initialisation of the SNTP time
 */
void init_time(void);




#endif