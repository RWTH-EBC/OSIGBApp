/** @file 
 * @brief The main configuration for the Gateways. Mainly for the handling of the webserver 
 * 
 * Includes all important variables, 
 * the default configuration and
 * the .html for the Webserver 
 * 
 * @date Jan 2021
 * 
*/

#ifndef _AIO_CONF_H_
#define _AIO_CONF_H_

#define STORAGE_NAMESPACE "aio_storage"

#include "nvs_flash.h"				


/**
 * @brief Structure for all variables of the IoT Configurator
 * 
 * Alle values are stored in this struct when the gateway is operating.
 * The variables are filled during the initialisation of the gateway.
 */
typedef struct {

	// WiFi AP
	/// Wifi SSID for AP-Mode
	char wifi_ap_ssid[32];   
	char wifi_ap_pass[64];
	
	// WiFi STA
	/// Wifi SSID for STA-Mode
	char wifi_sta_ssid[32];
	char wifi_sta_pass[64];

	// MQTT
		// Broker
		/// TLS or non encrypted
	uint8_t mqtt_encrypt;
		/// MQTT Hostname
	char	mqtt_host[64];
	uint16_t mqtt_port;
		///MQTT Broker credentials
	char	mqtt_user[64];
	char	mqtt_pass[64];

		// TOPICS
		/// API-key for creating topic. Not used with Aedifion. 
	char	mqtt_apikey[32];
		/// DeviceID for MQTT Autorisation. Not used with Aedifion. @todo Use DevicID/MAC 
	char	mqtt_deviceid[32];

		/// MQTT publish topic and Quality of Service
	char	mqtt_pub[72];
	uint8_t mqtt_pub_qos;
	char	mqtt_sub[72];
	uint8_t mqtt_sub_qos;
	uint8_t mqtt_encode;
	uint8_t mqtt_autoconfig;

	// AIO Interface
		//INPUT
		/// AIO Input Mode (Disable/Voltage/Current)
	uint8_t in_mode;
		/// AIO Datapoint name input
	char	in_key[128];
		/// AIO input min Value
	float	in_min;
	float	in_max;
		/// Input sampling (Periodic/Threshold)
	uint8_t in_log_mode;
		/// Sampling rate (Periodic) or Threshold value
	float	in_log_val;

		//OUTPUT
		/// AIO Output Mode (Disable/Voltage/Current)
	uint8_t out_mode;
	char	out_key[128];
	float	out_min;
	float	out_max;

	// TMP Interface
	uint8_t tmp_bat_mode;
	uint32_t tmp_bat_mode_val;
		// 1
	uint8_t tmp_1_enable;
	char 	tmp_1_name[128];
	float 	tmp_1_min;
	float 	tmp_1_max;
	uint8_t tmp_1_log_mode;
	float 	tmp_1_log_mode_val;
	uint8_t tmp_1_rtd;
	uint8_t tmp_1_wires;

		// 2
	uint8_t tmp_2_enable;
	char 	tmp_2_name[128];
	float 	tmp_2_min;
	float 	tmp_2_max;
	uint8_t tmp_2_log_mode;
	float 	tmp_2_log_mode_val;
	uint8_t tmp_2_rtd;
	uint8_t tmp_2_wires;

	uint8_t calib_enable;
	float calib_offset;
	float calib_gain;

	// Digital Interface
		//1
	uint8_t 		mod_10_num;
		//1.1
	uint8_t		mod_11_status;
	char 		mod_11_name[128];
	uint8_t 		mod_11_adr;
	uint8_t 		mod_11_cor;
	uint8_t		mod_11_func;
	uint8_t		mod_11_log_mode;
	float		mod_11_log_mode_val;
		//1.2
	uint8_t		mod_12_status;
	char 		mod_12_name[128];
	uint8_t 	mod_12_adr;
	uint8_t 	mod_12_cor;
	uint8_t		mod_12_func;
	uint8_t		mod_12_log_mode;
	float		mod_12_log_mode_val;
	
	// Synchronisation
	char	checkbox[128];
    char	ntp_address[128];
	char	sync_time[128];

} aio_conf_t;

/**
 * @brief Structure for input values of one specific input
 * 
 *
 */
typedef struct{
	uint8_t enable;
		/// Datapointname
	char 	name[128];
	float 	min;
	float 	max;
		/// Unit of the Datapoint. When using Budo this is automatic generated.
	char	unit[8];
		/// logging mode
	uint8_t log_mode;
	float 	log_mode_val;
		/// value correction
	uint16_t cor;
} input_t;

/**
 * @brief Structure for storing variables after initialsation
 * 
 * To have a more uniform code. After the initialisation,
 * the gateway specific variables are stored in this structure.
 * The advantage of this is, that we can use some function for all
 * kind of gateways.
 */
typedef struct{
	input_t in1;
	input_t in2;
	uint8_t num;
	uint8_t sleep;
	uint32_t sleeptime;
	char ip[32];
	char ver[128];
} datalink_t;


/**
 * @brief Structure for reading stored variables from nvs
 * 
 * 
 */
typedef struct {
	char name[142];
	char value[142];
	uint16_t offset;
} aio_conf_field_t;

/**
 * @brief Set which config to load during initialisation
 * 
 * Memory will load from Memory.
 * Default and Custom are configurations stored in Conf.h
 */
typedef enum {
	AIO_CONF_INIT_MEMORY = 0,
	AIO_CONF_INIT_DEFAULT,
	AIO_CONF_INIT_CUSTOM
} aio_conf_init_t;


/**
 * @brief Configure the Modubs communication on the digital gateway
 */
typedef enum {
	DISABLE = 0,
	READ_REGISTER,
	WRITE_REGISTER
} modbus_status_t;


/**
 * @brief Default configuration 
 * 
 * Is loaded during first initialsation or after reset of the gateway.
 */
static const aio_conf_t aio_conf_default = { "", "", "", "", 1, "", 8883, "", "", "", "", "", 0, "", 0, 2, 0, 0, "", 0, 0, 0, 1000, 0, "", 0, 0, 0, 10000000, 1, "", 0,0,0,0,0,0,0, "", 0,0,0,0,0,2,0,0,1,1,0,"",0,0,3,0,10000,0,"",0,0,3,0,10000, "true","",""};
// static const aio_conf_t aio_conf_default = { "", "", "", "", 1, "", 8883, "", "", "", "", "", 0, "", 0, 2, 0, "", 0, 0, 0, 1000, 0, "", 0, 0, 0, 10000000, 1, "", 0,0,0,0,0,0,0, "", 0,0,0,0,0,2,1,0,"",0,0,3,0,10000,0,"",0,0,3,0,10000 };

/**
 * @brief custom configuration 
 * 
 * Can be set in main.c.
 * When init_CONF() is set to AIO_CONF_INIT_CUSTOM, the configuration can not be overwritten
 * by the esp since after every bootup the custom config is loaded. 
 */
static const aio_conf_t aio_conf_custom  = 		{ "","","","",1,"",8883,"","", "", "", "",0,"",0,2, 0, 0,"",0,0,0,1000,0,"",0,0,0,10000000,1,"RoomTemperature",0,30,0,1000,0,2,1,"RoomTemperaturel", 0, 30, 0, 1000, 0, 2,0,1,1,1,1,"Belimo",4,100,3,0,10000, 2, "Belimo",0,100,6,0,10000,"true","",""};

/**
 * @brief Html code 
 * 
 * @ref html
 */
static const char html_request[]	   = "<!DOCTYPE html><html><head> <title>IoT Gate</title> <meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"> <link rel=\"icon\" href=\"data:,\"> <style>html{font-family: Helvetica; font-size: 90%%;}input[type=text], select{width: 100%%; padding: 7px; border: 1px solid #ccc; border-radius: 4px; resize: vertical; margin: 0px 0px 5px 0px; box-sizing: border-box;}input[type=submit]{background-color: #4CAF50; border: none; color: white; padding: 16px 32px; text-decoration: none; margin: 4px 2px; cursor: pointer; width: 100%%;}fieldset{border: 2px solid; border-radius: 3px; padding: 7px 7px 7px 7px; margin: 7px 0px;}.ext{border-color: #F00;}.container{background-color: #f2f2f2; padding: 10px;}.col{float: left; margin-top: 6px;}.row:after{content: \"\"; display: table; clear: both;}</style></head><body> <div class=\"container\"> <h2 align=\"center\">IoT AIO Configurator <br></h2> <h3 align=\"center\"> %s </h3> <h3 align=\"center\"> %s </h3> <form action=\"/\" method=\"post\" enctype=\"text/plain\"> <fieldset> <legend>WiFi:</legend> <fieldset class=\"ext\"> <legend>Access Point:</legend> SSID:<br><input type=\"text\" name=\"ssid_ap\" value=\"%s\"><br>PASS:<br><input type=\"text\" name=\"pass_ap\" value=\"%s\"> </fieldset> <fieldset class=\"ext\"> <legend>Station:</legend> SSID:<br><input type=\"text\" name=\"ssid_sta\" value=\"%s\"><br>PASS:<br><input type=\"text\" name=\"pass_sta\" value=\"%s\"> </fieldset></fieldset> <fieldset> <legend>MQTT:</legend> <fieldset class=\"ext\"> <legend>IoT Agent:</legend> <div class=\"row\"> <div class=\"col\" style=\"width:23%%\"> Crypt:<br><select name=\"scheme\"> <option value=\"0\"%s>None</option> <option value=\"1\"%s>TLS</option> </select> </div><div class=\"col\" style=\"width:60%%\"> Host:<br><input type=\"text\" name=\"host\" value=\"%s\"> </div><div class=\"col\" style=\"width:17%%\"> Port:<br><input type=\"text\" name=\"port\" value=\"%d\"> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:50%%\"> User Name:<br><input type=\"text\" name=\"user\" value=\"%s\"> </div><div class=\"col\" style=\"width:50%%\"> Password:<br><input type=\"text\" name=\"key\" value=\"%s\"> </div></div></fieldset> <fieldset class=\"ext\"> <legend>Topics:</legend> API-Key:<br><input type=\"text\" name=\"apikey\" value=\"%s\" placeholder=\"Empty for Aedifion\"><br>Device-ID:<br><input type=\"text\" name=\"id\" value=\"%s\" placeholder=\"Empty for Aedifion\"><br><div class=\"row\"> <div class=\"col\" style=\"width:75%%\"> Publish: <i>Aedifion: [load_balance_grop/project_handle]</i><br><input type=\"text\" name=\"pub_topic\" value=\"%s\" placeholder=\"attrs\"> </div><div class=\"col\" style=\"width:25%%\"> <br><select name=\"pub_qos\"> <option value=\"0\"%s>QoS 0 [&lt 1]</option> <option value=\"1\"%s>QoS 1 [&gt 1]</option> <option value=\"2\"%s>QoS 2 [=1]</option> </select> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:75%%\"> Subscribe: <i>Empty for Aedifion</i><br><input type=\"text\" name=\"sub_topic\" value=\"%s\" placeholder=\"Empty for Aedifion\"> </div><div class=\"col\" style=\"width:25%%\"> <br><select name=\"sub_qos\"> <option value=\"0\"%s>QoS 0 [&lt 1]</option> <option value=\"1\"%s>QoS 1 [&gt 1]</option> <option value=\"2\"%s>QoS 2 [=1]</option> </select> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:70%%\"> Encoding:<br><select name=\"encode\"> <option value=\"0\"%s>JSON -{\"key\":\"value\"}</option> <option value=\"1\"%s>UL2.0 - key|value</option> <option value=\"2\"%s>Aedifion - Influx,Swop (Put only publish topic)</option> </select> </div><div class=\"col\" style=\"width:30%%\"> Auto configuration with Fiware:<br><select name=\"autoconfig_fw\"> <option value=\"0\"%s>Disabled</option> <option value=\"1\"%s>Enabled</option> </select> </div></div><font size=\"2\"><i>&midast;TOPIC: /&ltapi-key&gt/&ltdevice-id&gt/&ltmode&gt</i></font> </fieldset> </fieldset> <fieldset> <legend>Analog Gateway Configuration:</legend> <fieldset class=\"ext\"> <legend>INPUT:</legend> <div class=\"row\"> <div class=\"col\" style=\"width:100%%\"> Phys:<br><select name=\"in\"> <option value=\"0\"%s>Disabled</option> <option value=\"1\"%s>Voltage 0-10 V</option> <option value=\"2\"%s>Voltage 2-10 V</option> <option value=\"3\"%s>Current 0-20 mA</option> <option value=\"4\"%s>Current 4-20 mA</option> </select> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:25%%;margin-top:30px\"> Sensor: </div><div class=\"col\" style=\"width:30%%\"> Name:<br><input type=\"text\" name=\"in_key\" value=\"%s\"> </div><div class=\"col\" style=\"width:22%%\"> Min:<br><input type=\"text\" name=\"in_min\" value=\"%.2f\"> </div><div class=\"col\" style=\"width:23%%\"> Max:<br><input type=\"text\" name=\"in_max\" value=\"%.2f\"> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:25%%;margin-top:15px\"> Mode: </div><div class=\"col\" style=\"width:30%%\"> <select name=\"mode\"> <option value=\"0\"%s>Periodic</option> <option value=\"1\"%s>Threshold</option> </select> </div><div class=\"col\" style=\"width:45%%\"> <input type=\"text\" name=\"mode_val\" value=\"%.2f\" placeholder=\"Rate [ms] / Value [-]\"> </div></div></fieldset> <fieldset class=\"ext\"> <legend>OUTPUT:</legend> <div class=\"row\"> <div class=\"col\" style=\"width:100%%\"> Phys:<br><select name=\"out\"> <option value=\"0\"%s>Disabled</option> <option value=\"1\"%s>Voltage 0-10 V</option> <option value=\"2\"%s>Voltage 2-10 V</option> <option value=\"3\"%s>Current 0-20 mA</option> <option value=\"4\"%s>Current 4-20 mA</option> </select> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:25%%;margin-top:30px\"> Actuator: </div><div class=\"col\" style=\"width:30%%\"> Name:<br><input type=\"text\" name=\"out_key\" value=\"%s\"> </div><div class=\"col\" style=\"width:22%%\"> Min:<br><input type=\"text\" name=\"out_min\" value=\"%.2f\"> </div><div class=\"col\" style=\"width:23%%\"> Max:<br><input type=\"text\" name=\"out_max\" value=\"%.2f\"> </div></div></fieldset> </fieldset> <fieldset> <legend>Temperature Gateway Configuration</legend> <div class=\"row\"> <div class=\"col\" style=\"width:25%%;margin-top:15px\"> <br>Sleep Mode: </div><div class=\"col\" style=\"width:30%%\"> <br><select name=\"tmp_bat_mode\"> <option value=\"0\"%s>Disable</option> <option value=\"1\"%s>Enable</option> </select> </div><div class=\"col\" style=\"width:45%%\"> Sleep TIme [us]<br><input type=\"text\" name=\"tmp_bat_mode_val\" value=\"%d\" placeholder=\"Sleep Time[us]\"> </div></div><fieldset class=\"ext\"> <legend>Probe 1, Input:</legend> <div class=\"row\"> <div class=\"col\" style=\"width:25%%;\"> <div class=\"col\" style=\"width:80%%\"> <select name=\"tmp_1_enable\"> <option value=\"0\"%s>Disable</option> <option value=\"1\"%s>Enable</option> </select> </div></div><div class=\"col\" style=\"width:30%%\"> Name:<br><input type=\"text\" name=\"tmp_1_name\" value=\"%s\"> </div><div class=\"col\" style=\"width:22%%\"> Min:<br><input type=\"text\" name=\"tmp_1_min\" value=\"%.2f\"> </div><div class=\"col\" style=\"width:23%%\"> Max:<br><input type=\"text\" name=\"tmp_1_max\" value=\"%.2f\"> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:25%%;margin-top:15px\"> Mode: </div><div class=\"col\" style=\"width:30%%\"> <select name=\"tmp_1_log_mode\"> <option value=\"0\"%s>Periodic</option> <option value=\"1\"%s>Threshold</option> </select> </div><div class=\"col\" style=\"width:45%%\"> <input type=\"text\" name=\"tmp_1_log_mode_val\" value=\"%.2f\" placeholder=\"Rate [ms] / Value [-]\"> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:25%%;margin-top:15px\"> RTD: </div><div class=\"col\" style=\"width:30%%\"> <select name=\"tmp_1_rtd\"> <option value=\"0\"%s>PT100</option> <option value=\"1\"%s>PT1000</option> </select> </div><div class=\"col\" style=\"width:45%%\"> <select name=\"tmp_1wires\"> <option value=\"0\"%s>2 wires</option> <option value=\"1\"%s>3 wires</option> <option value=\"2\"%s>4 wires</option> </select> </div></div></fieldset> <fieldset class=\"ext\"> <legend>Probe 2, Input:</legend> <div class=\"row\"> <div class=\"col\" style=\"width:25%%;\"> <div class=\"col\" style=\"width:80%%\"> <select name=\"tmp_2_enable\"> <option value=\"0\"%s>Disable</option> <option value=\"1\"%s>Enable</option> </select> </div></div><div class=\"col\" style=\"width:30%%\"> Name:<br><input type=\"text\" name=\"tmp_2_name\" value=\"%s\"> </div><div class=\"col\" style=\"width:22%%\"> Min:<br><input type=\"text\" name=\"tmp_2_min\" value=\"%.2f\"> </div><div class=\"col\" style=\"width:23%%\"> Max:<br><input type=\"text\" name=\"tmp_2_max\" value=\"%.2f\"> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:25%%;margin-top:15px\"> Mode: </div><div class=\"col\" style=\"width:30%%\"> <select name=\"tmp_2_log_mode\"> <option value=\"0\"%s>Periodic (same rate as probe 1)</option> <option value=\"1\"%s>Threshold</option> </select> </div><div class=\"col\" style=\"width:45%%\"> <input type=\"text\" name=\"tmp_2_log_mode_val\" value=\"%.2f\" placeholder=\"Rate [ms] / Value [-]\"> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:25%%;margin-top:15px\"> RTD: </div><div class=\"col\" style=\"width:30%%\"> <select name=\"tmp_2_rtd\"> <option value=\"0\"%s>PT100</option> <option value=\"1\"%s>PT1000</option> </select> </div><div class=\"col\" style=\"width:45%%\"> <select name=\"tmp_2wires\"> <option value=\"0\"%s>2 wires</option> <option value=\"1\"%s>3 wires</option> <option value=\"2\"%s>4 wires</option> </select> </div></div></fieldset> <fieldset class=\"ext\"> <legend> Calibration Settings:</legend> <div class=\"row\"> <div class=\"col\" style=\"width:25%%;\"> <div class=\"col\" style=\"width:80%%\"> <label for=\"calib_enable\"> User Calibration:</label><select name=\"calib_enable\"> <option value=\"0\"%s>Disable</option> <option value=\"1\"%s>Enable</option> </select> </div></div><div class=\"col\" style=\"width:30%%\"> <br> Calibration Offset:<br><input type=\"text\" name=\"calib_offset\" value=\"%.2f\"> </div><div class=\"col\" style=\"width:23%%\"> <br> Calibration Gain:<br><input type=\"text\" name=\"calib_gain\" value=\"%.2f\"> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:25%%;margin-top:15px\"> <div class=\"col\" style=\"width:80%%\"> <label for=\"scale_enable\"> </div></div></fieldset></fieldset> <fieldset> <legend>Digital Gateway Configuration</legend> <fieldset class=\"ext\"> <legend>Device 1</legend> <div class=\"row\"> <div class=\"col\" style=\"width:20%%;\"> Device Number (Modbus)<br><input type=\"text\" name=\"mod_10_num\" value=\"%d\"> </div></div><div class=\"row\"> <div class=\"col\" style=\"width:25%%;\"> <div class=\"col\" style=\"width:80%%\"> Status 1 <br><select name=\"mod_11_status\"> <option value=\"0\"%s>Disable</option> <option value=\"1\"%s>Read</option> <option value=\"2\"%s>Write</option> </select> </div></div><div class=\"col\" style=\"width:30%%\"> Name:<br><input type=\"text\" name=\"mod_11_name\" value=\"%s\"> </div><div class=\"col\" style=\"width:15%%\"> Address:<br><input type=\"text\" name=\"mod_11_adr\" value=\"%d\"> </div><div class=\"col\" style=\"width:30%%\"> Value Correction:<br><input type=\"text\" name=\"mod_11_cor\" value=\"%d\"> </div><div class=\"row\"> <div class=\"col\" style=\"width:15%%\"> Command <input type=\"text\" name=\"mod_11_func\" value=\"%d\" placeholder=\"Modbus command to send\"> </div><div class=\"col\" style=\"width:25%%\"> Sampling <select name=\"mod_11_log_mode\"> <option value=\"0\"%s>Periodic</option> <option value=\"1\"%s>Threshold</option> </select> </div><div class=\"col\" style=\"width:35%%\"> Rate [ms] / Value [-] <input type=\"text\" name=\"mod_11_log_mode_val\" value=\"%.2f\" placeholder=\"Rate [ms] / Value [-]\"> </div></div></div><div class=\"row\"> <div class=\"col\" style=\"width:25%%;\"> <div class=\"col\" style=\"width:80%%\"> Status 2 <br><select name=\"mod_12_status\"> <option value=\"0\"%s>Disable</option> <option value=\"1\"%s>Read</option> <option value=\"2\"%s>Write</option> </select> </div></div><div class=\"col\" style=\"width:30%%\"> Name:<br><input type=\"text\" name=\"mod_12_name\" value=\"%s\"> </div><div class=\"col\" style=\"width:15%%\"> Address:<br><input type=\"text\" name=\"mod_12_adr\" value=\"%d\"> </div><div class=\"col\" style=\"width:30%%\"> Value Correction:<br><input type=\"text\" name=\"mod_12_cor\" value=\"%d\"> </div><div class=\"row\"> <div class=\"col\" style=\"width:15%%\"> Command <input type=\"text\" name=\"mod_12_func\" value=\"%d\" placeholder=\"Modbus command to send\"> </div><div class=\"col\" style=\"width:25%%\"> Sampling <select name=\"mod_12_log_mode\"> <option value=\"0\"%s>Periodic (same rate as Register 1)</option> <option value=\"1\"%s>Threshold</option> </select> </div><div class=\"col\" style=\"width:35%%\"> Rate [ms] / Value [-] <input type=\"text\" name=\"mod_12_log_mode_val\" value=\"%.2f\" placeholder=\"Rate [ms] / Value [-]\"> </div></div></div></fieldset></fieldset> <fieldset class=\"ext\"> <legend> Synchronisation:</legend><br> <input type=\"checkbox\" id=\"NTP\" name=\"NTP\" value=\"true\" checked=\"true\"> <label for=\"NTP\"> Synchronise with NTP Server</label> <br><br> NTP Server Address: (Checkbox must be enabled!)<br> <input type=\"text\" name=\"Address\" value=\"%s\" placeholder=\"Default:ntp1.rwth-aachen.de\"> Synchronisation Time:<br><input type=\"datetime-local\" name=\"Snyc_time\" value=\"2022-02-23T21:11:10\"> </fieldset> <input type=\"submit\" value=\"Submit\"> </form> </div></body></html>";

static const char html_response_pass[] = "<!DOCTYPE html><html><head><title>IoT Gate</title><meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"><link rel=\"icon\" href=\"data:,\"><style>html{font-family: Helvetica;}input[type=submit]{background-color: #4CAF50; border: none; color: white; padding: 16px 32px; text-decoration: none; margin: 4px 2px; cursor: pointer; width: 100%%;}.container{background-color: #f2f2f2; padding: 10px;}</style></head><body><div class=\"container\"><h2>IoT Configurator</h2><br><input type=\"submit\" value=\"SUCCESFUL\"></div></body></html>";

static const char html_response_fail[] = "<!DOCTYPE html><html><head><title>IoT Gate</title><meta charset=\"utf-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"><link rel=\"icon\" href=\"data:,\"><style>html{font-family: Helvetica;}input[type=submit]{background-color: #fe7070; border: none; color: white; padding: 16px 32px; text-decoration: none; margin: 4px 2px; cursor: pointer; width: 100%%;}.container{background-color: #f2f2f2; padding: 10px;}</style></head><body><div class=\"container\"><h2>IoT Configurator</h2><br><input type=\"submit\" value=\"FAIL\"></div></body></html>";

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Export the actual configuration to the html output 
 * 
 * Exports the configuration inside an HTML form provided as a regular expression.
 * 
 * @param[in] conf: the config structure
 * @param[out] str: the filled HTM file
 * @param[in] reg_str: the empty html file with variables
 * 
 * @return if success ESP_OK
 */
esp_err_t  export_aio_conf(aio_conf_t* conf, char* str, const char* reg_str);

/**
 * @brief Import the actual configuration from the html webserver 
 * 
 * Imports the configuration from a string in standard HTML form format.
 * 
 * @param[out] conf: the config structure
 * @param[in] str: The response string from the webserver
 * 
 * @return if success ESP_OK
 */
esp_err_t  import_aio_conf(aio_conf_t* conf, char* str);

/**
 * @brief Writes the actual config to the nvs 
 * 
 * @param[out] conf: the config structure
 * 
 * @return if success ESP_OK
 */
esp_err_t	write_aio_conf(aio_conf_t* conf);

/**
 * @brief prints the actual config to the console
 * 
 */
void		print_aio_conf(aio_conf_t* conf);

/**
 * @brief Load the the config from nvs to config struct.
 * 
 * 
 * @param[out] conf: the config structure
 * @param[in] _init: Init from nvs or default or custom
 * 
 * @return if success ESP_OK
 */
esp_err_t	 init_CONF(aio_conf_t* conf, aio_conf_init_t _init);

/**
 * @brief Read the unit from BUDO datapoint name
 * 
 * 
 * @param[in] in_name: The BUDO Datapoint name
 * @param[out] out_unit: The actual unit used in the Datapoint name
 * 
 */
void add_unit(char *in_name, char *out_unit);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif