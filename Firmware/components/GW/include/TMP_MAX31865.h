/** @file
 * @brief MAX31865 Temperature sensor configuration
 *  For more information see github documentation: @link https://github.com/jamieparkinson/ESP32-MAX31865
 */

#ifndef MAX31865_H
#define MAX31865_H

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include "AIO_Board.h"

#define MAX31856_CONFIG_REG            0x00
#define MAX31856_CONFIG_BIAS           0x80
#define MAX31856_CONFIG_MODEAUTO       0x40
#define MAX31856_CONFIG_MODEOFF        0x00
#define MAX31856_CONFIG_1SHOT          0x20
#define MAX31856_CONFIG_3WIRE          0x10
#define MAX31856_CONFIG_24WIRE         0x00
#define MAX31856_CONFIG_FAULTSTAT      0x02
#define MAX31856_CONFIG_FILT50HZ       0x01
#define MAX31856_CONFIG_FILT60HZ       0x00

#define MAX31856_RTDMSB_REG           0x01
#define MAX31856_RTDLSB_REG           0x02
#define MAX31856_HFAULTMSB_REG        0x03
#define MAX31856_HFAULTLSB_REG        0x04
#define MAX31856_LFAULTMSB_REG        0x05
#define MAX31856_LFAULTLSB_REG        0x06
#define MAX31856_FAULTSTAT_REG        0x07


#define MAX31865_FAULT_HIGHTHRESH     0x80
#define MAX31865_FAULT_LOWTHRESH      0x40
#define MAX31865_FAULT_REFINLOW       0x20
#define MAX31865_FAULT_REFINHIGH      0x10
#define MAX31865_FAULT_RTDINLOW       0x08
#define MAX31865_FAULT_OVUV           0x04


#define RTD_A 3.9083e-3
#define RTD_B -5.775e-7

typedef enum {
	MAX31865_60Hz = 0,
	MAX31865_50Hz = 1
} max31865_notch_t;

typedef enum {
	MAX31865_2WIRE = 0,
	MAX31865_3WIRE = 1,
	MAX31865_4WIRE = 0
} max31865_numwires_t;

typedef struct {
    spi_device_interface_config_t spi_cfg;
    spi_device_handle_t spi_dev;
	float RTD_nominal;
	float REF_resistor;
	max31865_numwires_t wires;
} max31865_t;


#ifdef __cplusplus
extern "C" {
#endif

esp_err_t init_MAX31865_1(float _RTD_nominal, float _REF_resistor, max31865_numwires_t _wires);
esp_err_t init_MAX31865_2(float _RTD_nominal, float _REF_resistor, max31865_numwires_t _wires);
esp_err_t free_MAX31865();

float max31865_temperature_1();
float max31865_temperature_2();
void  max31865_enable(u_int8_t devnum, bool status);


#ifdef __cplusplus
}
#endif

#endif
