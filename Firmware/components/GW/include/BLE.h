/** @file 
 * @brief Blutooth Low Energy
 * 
 * BLE is used for Eddy stone Functionality
 * The Gateway is able to send, so called, eddy stone beacons. 
 * These contain the actual IP address of the gateway and are used for a more easier configuration.
 * 
 */


#ifndef _BLE_H_
#define _BLE_H_

#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"



/**
 * @brief Send an Eddy Stone beacon
 * 
 * @param[in] led: True/False If true the LED will blink 2 times if the beacon is enabled.
 */
void eddy_stone(bool led);

/**
 * @brief write the actual IP adress to the Eddy stone layer.
 * 
 * This is neccessary to advice the correct ip when enabling the beacon.
 * 
 * @param[in] uri: The actual IP of the webserver
 */
void getURI_Eddy(char *uri);

#endif