/** @file 
 * @brief Handling the Network connection
 * 
 * Initialisation of the Wlan connection
 * 
 */

#ifndef _AIO_NETW_H_
#define _AIO_NETW_H_


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_mac.h"
#include "esp_chip_info.h"
#include "esp_random.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_ppp.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "Conf.h"

extern bool rebootloop;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Change wifi mode to APSTA(Accesspoint and station)
 * 
 * This function is used for enabling the Access Point mode
 * 
 * @todo rename fuction
 */
esp_err_t check_wifi_mode();


/**
 * @brief Function for initialisation of the network connection
 * 
 * @param[in] aio_conf
 * @param[in] input
 * 
 * @return ESP_OK if success 
 */ 
esp_err_t init_NETW(aio_conf_t* aio_conf, datalink_t *input);
	

#ifdef __cplusplus
}
#endif //__cplusplus

#endif