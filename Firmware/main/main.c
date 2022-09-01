/** @file 
 * @brief The main code
 * @date Jan 2021
 * 
*/

#include <stdio.h>
#include "esp_log.h" 				
#include "esp_system.h"

#include "Conf.h"
#include "AIO_Board.h"
#include "Netw.h"
#include "Mqtt.h"
#include "Intr.h"
#include "TMP_MAX31865.h"
#include "ota.h"
#include "led.h"
#include "sntp.h"
#include "DIO_Modbus.h"

#ifdef CONFIG_IOT_INTERFACE_TMP
	#include "TMP_sleep.h"
	#include "BLE.h"
	esp_sleep_wakeup_cause_t reason;
	uint8_t bat_mode;
#endif
/// TAG of this module for Debug logs
static const char *TAG = "APP";														
/// If the gateway has problems it will restart after a defined amount of failures
bool rebootloop = true;		

static aio_conf_t aio_conf;														 	
static datalink_t input;

/**
 * @brief Send meta tags on startup
 * 
 * This variable is only used for the temperature gateways.
 * Without this variable the esp will send it's meta tag after every
 * deep sleep caused reboot.
 */
RTC_DATA_ATTR bool meta = true;



/**
 * @brief The main function of the code
 * 
 * Starts all necessary modules for the gateway
 */
void app_main(void)
{
	ESP_LOGD(TAG, "Startup..");
	ESP_LOGD(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGD(TAG, "IDF version: %s", esp_get_idf_version());


	ESP_LOGD(TAG, "Configuring Log levels");  											// Set the loging levels for debugging for each module
	esp_log_level_set("*", ESP_LOG_INFO);
	esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
	esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
	esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
	
	esp_log_level_set("APP", ESP_LOG_DEBUG);
	
	esp_log_level_set("MAX31865", ESP_LOG_WARN);
	esp_log_level_set("AIO_Board", ESP_LOG_DEBUG);
	esp_log_level_set("AIO_Conf", ESP_LOG_DEBUG);
	esp_log_level_set("AIO_Netw", ESP_LOG_DEBUG);
	esp_log_level_set("Mqtt", ESP_LOG_DEBUG);
	esp_log_level_set("AIO_Intr", ESP_LOG_INFO);
	esp_log_level_set("AIO_SNTP", ESP_LOG_INFO);
	esp_log_level_set("led",ESP_LOG_INFO);
	esp_log_level_set("cJSON_parse",ESP_LOG_DEBUG);
	esp_log_level_set("DIO_Modbus",ESP_LOG_DEBUG);
	esp_log_level_set("TMP_sleep",ESP_LOG_DEBUG);



	ESP_LOGD(TAG, "Init modules..");
	init_led();														// Init LED
	

	init_CONF(&aio_conf, AIO_CONF_INIT_MEMORY);						// Init Configuration from Memory
	// init_CONF(&aio_conf, AIO_CONF_INIT_CUSTOM);					// Init Configuration from Custom folder. No changes can be made in the configurator
		
	#ifdef CONFIG_IOT_INTERFACE_AIO
	ESP_LOGI(TAG, "AIO SETUP");
	init_AIO (&aio_conf, &input);                                    // Init AIO Board

	
	#elif CONFIG_IOT_INTERFACE_DIO
 	ESP_LOGI(TAG, "DIO SETUP");
	ESP_ERROR_CHECK(init_MODBUS());									// Init Modbus for Digital Board
	init_modbus_devices(&aio_conf, &input);

	#elif CONFIG_IOT_INTERFACE_TMP
	ESP_LOGI(TAG, "TMP SETUP");
	esp_rom_gpio_pad_select_gpio(max_en);
	gpio_set_direction(max_en, GPIO_MODE_OUTPUT);
	gpio_set_level(max_en, 1);
	bat_mode = aio_conf.tmp_bat_mode;
	ESP_LOGD(TAG, "BATmode = %d", bat_mode);

	reason = esp_sleep_get_wakeup_cause();
	ESP_LOGD(TAG, "Wakeupreason: %d",reason );
	if (reason == ESP_SLEEP_WAKEUP_EXT0)
	{
		aio_conf.tmp_bat_mode = 0;
		bat_mode = 0;
		rebootloop = false;
		//input.sleep = 0; 
		// eddy_stone(false);
	}
	init_RTD(&aio_conf, &input);
	ESP_LOGI(TAG, "Batmode" );
	if(bat_mode == 1)
	{
		deep_sleep_tmp(&aio_conf, &input);
	}
	#endif 

	init_INTR();													// Init Interrupts
	init_NETW(&aio_conf, &input);									// Init Network
	#ifdef CONFIG_IOT_INTERFACE_TMP
		if (reason == ESP_SLEEP_WAKEUP_EXT0)
			eddy_stone(true);
	#endif
	init_time();													// Init mutex for sntp time function
	ota(&input);													// Init OTA
	init_MQTT(&aio_conf, &input);									// Init MQTT
	// Set meta Tag on Aedifion server
	if (meta && aio_conf.mqtt_encode == MQTT_ENCODE_AED)
	{
		ESP_LOGI(TAG,"Set Meta Tags");
		meta = false; 
		publish_meta_tag();												// publish Meta tags for Datapoints
	}
	// if (aio_conf.mqtt_autoconfig == 1)
	// {
		
	// }
	led_blink(1);

	#ifdef CONFIG_testmode
	wifi_signal();													// send the signal strength to AP over MQTT
	#endif

    while(1){
		// vTaskDelay(100 / portTICK_PERIOD_MS);
         vTaskDelay(portMAX_DELAY);
		
	}
}