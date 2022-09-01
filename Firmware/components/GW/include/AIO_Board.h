/** @file 
 * @brief The configuration and initialisation of the analog gateway and meanwhile also of the temp gateway
 * 
 * 
 * @date Jan 2021
 * 
*/

#ifndef _AIO_BOARD_H_
#define _AIO_BOARD_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/adc.h"
#include "driver/dac.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "TMP_MAX31865.h"
#include "Conf.h"
#include "AIO_MCP3021.h"
#include "led.h"

extern bool rebootloop;
// ---------- GPIO Definitions

/// @todo Move PIN reset to AIO_Conf.h
#define PIN_RESET		 0

// #define PIN_ADC			37	// ADC1_CH1
// #define PIN_DAC			DAC_CHANNEL_2	// 26
// #define PIN_CAL			 4

// XTR111 = Voltage to Current converter
/// XTR Output Disable 	
#define PIN_XTR_OD		26	
/// XTR Output Failure 
#define PIN_XTR_EF		27	
/// XTR Output PWM	
#define PIN_XTR_DAC		25		

// Digital to analog converter = MCP3021
/// DAC Address
#define MCP3021_ADDRESS 0X4d
/// DAC SDA PIN
#define PIN_MCP_SDA		17
/// DAC SCL PIN
#define PIN_MCP_SCL		16


/// @todo Put MAX configuration to seperate Config file for the RTD board. 

// MAX31865 RTD-To digital Converter

/// Chip select PIN for MAX 2
#define PIN_MAX_CS_2	16	// VSPICS0	SPI Chips Select 
/// Chip select PIN for MAX 2
#define PIN_MAX_CS_1	17	
// #define PIN_MAX_RDY		 9	//	SPI Data Ready 		

/// VSPICLK	SPI Clock 
#define PIN_MAX_SCK		5	
/// VSPIQ	SPI MISO 			
#define PIN_MAX_MISO	4		
/// VSPID	SPI Chip Select			
#define PIN_MAX_MOSI	18	



// ---------- AIO Modes
/**
 * @brief The diffrent input and output modes
 */ 
typedef enum {
	AIO_DISABLED = 0,
	AIO_VOLTAGE_0_10,
	AIO_VOLTAGE_2_10,
	AIO_CURRENT_0_20,
	AIO_CURRENT_4_20,
	AIO_PASSIVE
} aio_mode_t;


/**
 * @brief enumeration for the calculation of the output value
 * 
 * This is used for a better overview while using a switch to calculate the input/output value.
 * 
 */ 
typedef enum {
	AI_CODE_TO_VOLT = 0,
	AI_VOLT_TO_PHYS,
	AI_PHYS_TO_UNIT,
	AO_UNIT_TO_PHYS,
	AO_PHYS_TO_VOLT,
	AO_VOLT_TO_CODE
} aio_conv_t;


/**
 * @brief Structure for storing the values during the Calculation
 * 
 * The values from the ADC are converted to the real values. The interim results are stored in this struct.
 * 
 */ 
typedef struct {
	aio_mode_t AI;
	aio_mode_t AO;

	float	AI_unit_max;
	float	AI_unit_min;
	float	AO_unit_max;
	float	AO_unit_min;

	float	AI_phys_max;
	float	AI_phys_min;
	float	AO_phys_max;
	float	AO_phys_min;

	float	AI_code_max;
	float	AI_code_min;
	float	AO_code_max;
	float	AO_code_min;

	uint8_t	OFFS_CAL;
	float	OFFS_ADC;
	float	VREF_EXT;
	float	VREF_ADC;
	float	VSUP;
	float	VSUP1;
} aio_dev_t;

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Function for initialisation of the analog gateway
 * 
 * @param[in] aio_conf
 * @param[in] input
 * 
 * @return ESP_OK if success 
 */ 
esp_err_t init_AIO(aio_conf_t *aio_conf, datalink_t* input);


/// @todo Put to RTD specific config file (init_RTD function)
/**
 * @brief Function for initialisation of the temperature gateway
 * 
 * @param[in] aio_conf
 * @param[in] input
 * 
 * @return ESP_OK if success 
 */ 
esp_err_t init_RTD(aio_conf_t *aio_conf, datalink_t* input);


/**
 * @brief Function for restarting the Gateway
 * 
 * This function cheacks if the interrupt button is pressed. This will disalbe the restart process.
 * Hence, the gateway will not restart if the button is pressed during the configuration of the gateway.
 */ 
void restart_on_failure();

/**
 * @brief This function will set the given voltage as output voltage
 * 
 * The raw value will be calculated from the given value
 * 
 * @param[in] _unit: The voltage which should be set
 */ 
void  AIO_set(float _unit);


/**
 * @brief This Function gets the value from the ADC.
 * 
 * @param[in] n: Deprecated/ @todo delete n
 */ 
float AIO_get(uint16_t n);

/**
 * @brief This function will set the actuator initial value with the last one it had before restart
 */ 
esp_err_t AO_Init();

/**
 * @brief store the last setpoint in nvs 
 */ 

esp_err_t setpoint_save(uint32_t setpoint1,uint32_t setpoint2);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif