/** @file
 *  @brief Handling the use of the LED
 * 
 */

#ifndef _led_H_
#define _led_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BLINK_GPIO CONFIG_LED_GPIO

    /**
     * @brief Intialisation of the onboard LED
     */
    void init_led();

    /**
     * @brief blink the led 2 times
     * @todo remove from code (only use blink and blink fast)
     */
    void blink();

    /**
     * @brief Blink the LED slow 
     * 1s on 1s off
     * @param[in] times: How many times the LED should flash on
     */
    void led_blink(uint8_t times);

    /**
     * @brief Blink the LED fast 
     * 0.3s on 0.3s off
     * @param[in] times: How many times the LED should flash on
     */
    void led_fast(uint8_t times);


#endif