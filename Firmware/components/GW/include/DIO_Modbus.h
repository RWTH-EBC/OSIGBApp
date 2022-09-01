/** @file
 *  @brief Digital Gateway Modbus communication

 */

#ifndef _DIO_Modbus_H_
#define _DIO_Modbus_H_



#include "string.h"
#include "esp_log.h"
#include "Conf.h"

#include "sdkconfig.h"

// #ifdef CONFIG_IOT_INTERFACE_DI
// #include "../freemodbus/common/include/mbcontroller.h"
#include "../../managed_components/espressif__esp-modbus/freemodbus/common/include/mbcontroller.h"

/**
 * @brief Initialisation of the modbus communication
 * 
 * @return ESP_OK if success
 */
esp_err_t init_MODBUS(void);

/** @brief Set Value to Modbus device
 * 
 * @param[in] adr: Modbus register adress
 * @param[in] val: value to set device to.
 * 
 */
void set_Modbus(int dev, uint8_t func, int adr, char *val);

/**
 * @brief Initialisation of the connected modbus devices
 * 
 */
void init_modbus_devices(aio_conf_t* conf, datalink_t* input);


/* Get actual Modubs Value

@param dev: Device ID
@param adr: Register Adress
*/
float get_Modbus(uint16_t dev,uint8_t func, uint16_t adr);

//#endif
#endif
