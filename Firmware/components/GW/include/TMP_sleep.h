/** @file
 * @brief Sleep function of the Temperature Gateway
 * 
 * This is not longer used since the Temperature Gateway now also contains a 24 V to 3,3 V buck converter.
 * So there is no need for a deep sleep at the moment. Maybe in the future when the power consumption shuld be reduced
 * 
 */
#ifndef _TMP_SLEEP_H_
#define _TMP_SLEEP_H_

#include "esp_sleep.h"

#include "Conf.h"
#include "AIO_Board.h"
#include "Netw.h"
#include "Mqtt.h"
#include "math.h"
#include "driver/rtc_io.h"
#include "esp32/rom/uart.h"

#define max_en 26

void deep_sleep_tmp(aio_conf_t* aio_conf, datalink_t* input_in);

/**
 * @brief Starting Sleep Mode and enable CTRL button interrupt
*/
void start_sleep();

/**
 * @brief Sleep vor an hour. 
*/
void long_sleep();

#endif