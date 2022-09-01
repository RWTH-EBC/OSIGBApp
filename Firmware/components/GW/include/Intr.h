/** @file 
 * @brief Interrupts 
 * 
 * 
 * @date Jan 2021
 * 
*/

#ifndef _AIO_INTR_H_
#define _AIO_INTR_H_


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "time.h"

#include "AIO_Board.h"
#include "Conf.h"
#include "Netw.h"
#include "Intr.h"

#include "BLE.h"


#define ESP_INTR_FLAG_DEFAULT 0

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief Init the Interrupts on the ESP board
 * Interrupts are used for the config button
 * 
 */
esp_err_t init_INTR();


#ifdef __cplusplus
}
#endif //__cplusplus

#endif