/** @file 
 * @brief MCP3021 Analog to digital converter
 * 
 * 
 * Functions used with the ADC.
 * 
 * @todo move to AIO_Board.h
 * @date Jan 2021
 * 
 */

#ifndef AIO_MCP3021_H
#define AIO_MCP3021_H

#include <stdio.h>
#include "driver/i2c.h"
#include "esp_types.h"
#include "AIO_Board.h"


void init_MCP3021();

float get_voltage(void);

#endif