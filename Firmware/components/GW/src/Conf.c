#include "Conf.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_chip_info.h"
#include "esp_random.h"
#include <string.h>
#include <esp_log.h>

static const char *TAG = "AIO_Conf";
extern char import_flag;

void print_aio_conf(aio_conf_t* conf)
{
	ESP_LOGI(TAG, "AIO configuration:");

	printf("\t%s\n", conf->wifi_ap_ssid);
	printf("\t%s\n", conf->wifi_ap_pass);
	printf("\t%s\n", conf->wifi_sta_ssid);
	printf("\t%s\n", conf->wifi_sta_pass);
	printf("\t%d\n", conf->mqtt_encrypt);
	printf("\t%s\n", conf->mqtt_host);
	printf("\t%d\n", conf->mqtt_port);
	printf("\t%s\n", conf->mqtt_user);
	printf("\t%s\n", conf->mqtt_pass);
	printf("\t%s\n", conf->mqtt_apikey);
	printf("\t%s\n", conf->mqtt_deviceid);
	printf("\t%s\n", conf->mqtt_pub);
	printf("\t%d\n", conf->mqtt_pub_qos);
	printf("\t%s\n", conf->mqtt_sub);
	printf("\t%d\n", conf->mqtt_sub_qos);
	printf("\t%d\n", conf->mqtt_encode);
	printf("\t%d\n", conf->mqtt_autoconfig);
	printf("\t%d\n", conf->in_mode);
	printf("\t%s\n", conf->in_key);
	printf("\t%f\n", conf->in_min);
	printf("\t%f\n", conf->in_max);
	printf("\t%d\n", conf->in_log_mode);
	printf("\t%f\n", conf->in_log_val);
	printf("\t%d\n", conf->out_mode);
	printf("\t%s\n", conf->out_key);
	printf("\t%f\n", conf->out_min);
	printf("\t%f\n", conf->out_max);
	printf("\t%d\n", conf->tmp_bat_mode);
	printf("\t%d\n", conf->tmp_bat_mode_val);
	printf("\t%d\n", conf->tmp_1_enable);
	printf("\t%s\n", conf->tmp_1_name);
	printf("\t%f\n", conf->tmp_1_min);
	printf("\t%f\n", conf->tmp_1_max);
	printf("\t%d\n", conf->tmp_1_log_mode);
	printf("\t%f\n", conf->tmp_1_log_mode_val);
	printf("\t%d\n", conf->tmp_1_rtd);
	printf("\t%d\n", conf->tmp_1_wires);
	printf("\t%d\n", conf->mod_10_num);
	printf("\t%d\n", conf->mod_11_status);
	printf("\t%s\n", conf->mod_11_name);
	printf("\t%d\n", conf->mod_11_adr);
	printf("\t%d\n", conf->mod_11_cor);
	printf("\t%d\n", conf->mod_11_func);
	printf("\t%d\n", conf->mod_11_log_mode);
	printf("\t%f\n", conf->mod_11_log_mode_val);
	printf("\t%d\n", conf->mod_12_status);
	printf("\t%s\n", conf->mod_12_name);
	printf("\t%d\n", conf->mod_12_adr);
	printf("\t%d\n", conf->mod_12_cor);
	printf("\t%d\n", conf->mod_12_func);
	printf("\t%d\n", conf->mod_12_log_mode);
	printf("\t%f\n", conf->mod_12_log_mode_val);
	printf("\t%s\n", conf->checkbox);
	printf("\t%s\n", conf->ntp_address);
	printf("\t%s\n", conf->sync_time);

}

aio_conf_field_t next_field(char* str, uint16_t length, uint16_t offset)
{
	aio_conf_field_t aio_conf_field;
	memset(&aio_conf_field, 0, sizeof(aio_conf_field_t));

	for (int i = offset; i <= length; i++)
		if ((str[i] == '\r') || (i == length))
			for (int j = offset; j < i; j++)
				if (str[j] == '=') {
					memcpy(aio_conf_field.name, &str[offset], j - offset);
					aio_conf_field.name[j - offset] = '\0';
					memcpy(aio_conf_field.value, &str[j + 1], i - j - 1);
					aio_conf_field.value[i - j - 1] = '\0';
					aio_conf_field.offset = i + 2;
					return aio_conf_field;
				}

	return aio_conf_field;
}

esp_err_t write_aio_conf(aio_conf_t* conf)
{
	ESP_LOGI(TAG, "Write AIO configuration to NVS");
	nvs_handle my_handle;
	esp_err_t err;

	// Open
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
	if (err != ESP_OK)
		return err;

	// Write value including previously saved blob if available
	size_t required_size = sizeof(aio_conf_t) + sizeof(uint32_t);
	err = nvs_set_blob(my_handle, "aio_conf", conf, required_size);

	if (err != ESP_OK)
		return err;

	// Commit
	err = nvs_commit(my_handle);
	if (err != ESP_OK)
		return err;

	// Close
	nvs_close(my_handle);
	return ESP_OK;
}

/*	Exports the configuration
	inside an HTML form provided
	as a regular expression.
 */
esp_err_t export_aio_conf(aio_conf_t* conf, char* str, const char* reg_str)
{
	char esp_name[32]; 
	char device[32];
	ESP_LOGD(TAG, "starting to Export config: html_size = %d", sizeof(str));
	// printf("string:\n %s\n", reg_str);
	// ESP_LOGD(TAG, "End of string");
	if ( strlen(conf->wifi_ap_ssid) > 0)
		memcpy(esp_name, conf->wifi_ap_ssid, strlen(conf->wifi_ap_ssid));
	else {
		uint8_t chipid[6];
		// char ssid_ap[32];
		esp_efuse_mac_get_default(chipid);
		sprintf(esp_name, "ESP_%02x%02x%02x%02x%02x%02x", chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);
		// sprintf(ssid_ap, "ESP_%02x%02x%02x%02x%02x%02x", chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);
		// memcpy(esp_name, ssid_ap, strlen(ssid_ap));
	}
	#ifdef CONFIG_IOT_INTERFACE_TMP
	sprintf(device,"Temperature Gateway");
	#elif CONFIG_IOT_INTERFACE_AIO
	sprintf(device,"Analog Gateway");
	#elif CONFIG_IOT_INTERFACE_DIO
	sprintf(device,"Digital Gateway");
	#endif

	int ret = snprintf(str, 85000, reg_str,
		(esp_name[0] != '\0') ? (esp_name) : "Name not found",
		(device[0] != '\0') ? (device) : "Device not defined",
		(conf->wifi_ap_ssid[0] != '\0') ? (conf->wifi_ap_ssid) : "\0",
		(conf->wifi_ap_pass[0] != '\0') ? (conf->wifi_ap_pass) : "\0",
		(conf->wifi_sta_ssid[0] != '\0') ? (conf->wifi_sta_ssid) : "\0",
		(conf->wifi_sta_pass[0] != '\0') ? (conf->wifi_sta_pass) : "\0",
		(conf->mqtt_encrypt == 0) ? "selected" : "\0",
		(conf->mqtt_encrypt == 1) ? "selected" : "\0",
		(conf->mqtt_host[0] != '\0') ? (conf->mqtt_host) : "\0",
		conf->mqtt_port,
		(conf->mqtt_user[0] != '\0') ? (conf->mqtt_user) : "\0",
		(conf->mqtt_pass[0] != '\0') ? (conf->mqtt_pass) : "\0",
		(conf->mqtt_apikey[0] != '\0') ? (conf->mqtt_apikey) : "\0",
		(conf->mqtt_deviceid[0] != '\0') ? (conf->mqtt_deviceid) : "\0",
		(conf->mqtt_pub[0] != '\0') ? (conf->mqtt_pub) : "\0",
		(conf->mqtt_pub_qos == 0) ? "selected" : "\0",
		(conf->mqtt_pub_qos == 1) ? "selected" : "\0",
		(conf->mqtt_pub_qos == 2) ? "selected" : "\0",
		(conf->mqtt_sub[0] != '\0') ? (conf->mqtt_sub) : "\0",
		(conf->mqtt_sub_qos == 0) ? "selected" : "\0",
		(conf->mqtt_sub_qos == 1) ? "selected" : "\0",
		(conf->mqtt_sub_qos == 2) ? "selected" : "\0",
		(conf->mqtt_encode == 0) ? "selected" : "\0",
		(conf->mqtt_encode == 1) ? "selected" : "\0",
		(conf->mqtt_encode == 2) ? "selected" : "\0",
		(conf->mqtt_autoconfig == 0) ? "selected" : "\0",
		(conf->mqtt_autoconfig == 1) ? "selected" : "\0",
		(conf->in_mode == 0) ? "selected" : "\0",
		(conf->in_mode == 1) ? "selected" : "\0",
		(conf->in_mode == 2) ? "selected" : "\0",
		(conf->in_mode == 3) ? "selected" : "\0",
		(conf->in_mode == 4) ? "selected" : "\0",
		(conf->in_key[0] != '\0') ? (conf->in_key) : "\0",
		conf->in_min,
		conf->in_max,
		(conf->in_log_mode == 0) ? "selected" : "\0",
		(conf->in_log_mode == 1) ? "selected" : "\0",
		conf->in_log_val,
		(conf->out_mode == 0) ? "selected" : "\0",
		(conf->out_mode == 1) ? "selected" : "\0",
		(conf->out_mode == 2) ? "selected" : "\0",
		(conf->out_mode == 3) ? "selected" : "\0",
		(conf->out_mode == 4) ? "selected" : "\0",
		(conf->out_key[0] != '\0') ? (conf->out_key) : "\0",
		conf->out_min,
		conf->out_max,
		(conf->tmp_bat_mode == 0) ? "selected" : "\0",
		(conf->tmp_bat_mode == 1) ? "selected" : "\0",
		conf->tmp_bat_mode_val,
		(conf->tmp_1_enable == 0) ? "selected" : "\0",
		(conf->tmp_1_enable == 1) ? "selected" : "\0",
		(conf->tmp_1_name[0] != '\0') ? (conf->tmp_1_name) : "\0",
		conf->tmp_1_min,
		conf->tmp_1_max,
		(conf->tmp_1_log_mode == 0) ? "selected" : "\0",
		(conf->tmp_1_log_mode == 1) ? "selected" : "\0",
		conf->tmp_1_log_mode_val,
		(conf->tmp_1_rtd == 0) ? "selected" : "\0",
		(conf->tmp_1_rtd == 1) ? "selected" : "\0",
		(conf->tmp_1_wires == 0) ? "selected" : "\0",
		(conf->tmp_1_wires == 1) ? "selected" : "\0",
		(conf->tmp_1_wires == 2) ? "selected" : "\0",
		(conf->tmp_2_enable == 0) ? "selected" : "\0",
		(conf->tmp_2_enable == 1) ? "selected" : "\0",
		(conf->tmp_2_name[0] != '\0') ? (conf->tmp_2_name) : "\0",
		conf->tmp_2_min,
		conf->tmp_2_max,
		(conf->tmp_2_log_mode == 0) ? "selected" : "\0",
		(conf->tmp_2_log_mode == 1) ? "selected" : "\0",
		conf->tmp_2_log_mode_val,
		(conf->tmp_2_rtd == 0) ? "selected" : "\0",
		(conf->tmp_2_rtd == 1) ? "selected" : "\0",
		(conf->tmp_2_wires == 0) ? "selected" : "\0",
		(conf->tmp_2_wires == 1) ? "selected" : "\0",
		(conf->tmp_2_wires == 2) ? "selected" : "\0",
		(conf->calib_enable == 0) ? "selected" : "\0",
		(conf->calib_enable == 1) ? "selected" : "\0",
		conf->calib_offset,
		conf->calib_gain,
		conf->mod_10_num,
		(conf->mod_11_status == 0) ? "selected" : "\0",
		(conf->mod_11_status == 1) ? "selected" : "\0",
		(conf->mod_11_status == 2) ? "selected" : "\0",
		(conf->mod_11_name[0] != '\0') ? (conf->mod_11_name) : "\0",
		conf->mod_11_adr,
		conf->mod_11_cor,
		conf->mod_11_func,
		(conf->mod_11_log_mode == 0) ? "selected" : "\0",
		(conf->mod_11_log_mode == 1) ? "selected" : "\0",
		conf->mod_11_log_mode_val,
		(conf->mod_12_status == 0) ? "selected" : "\0",
		(conf->mod_12_status == 1) ? "selected" : "\0",
		(conf->mod_12_status == 2) ? "selected" : "\0",
		(conf->mod_12_name[0] != '\0') ? (conf->mod_12_name) : "\0",
		conf->mod_12_adr,
		conf->mod_12_cor,
		conf->mod_12_func,
		(conf->mod_12_log_mode == 0) ? "selected" : "\0",
		(conf->mod_12_log_mode == 1) ? "selected" : "\0",
		conf->mod_12_log_mode_val,
		"\0",
		0);

	ESP_LOGD(TAG, "ret = %d", ret);
	if (ret < 0)
		return ESP_ERR_NOT_FOUND;
	return ESP_OK;
}

/*	Imports the configuration from
	a string in standard HTML form format.
 */
esp_err_t import_aio_conf(aio_conf_t* conf, char* str)
{
	ESP_LOGI(TAG, "Importing AIO configuration");

	int len = strlen(str);
	int field_num = 0;
	char flag =0;
	
	// Check number of fields
	for (int i = 0; i < len; i++)
		if (str[i] == '=')
			//printf("%02x", str[i]);
			field_num++;

	if (field_num < 51)
		{
			ESP_LOGI(TAG, "Not enough fields!!");
			return ESP_FAIL;
		}

	aio_conf_field_t field;

	// SSID_AP
	field = next_field(str, len, 0);
	memcpy(conf->wifi_ap_ssid, field.value, 32);

	// PASS_AP
	field = next_field(str, len, field.offset);
	memcpy(conf->wifi_ap_pass, field.value, 64);

	// SSID_STA
	field = next_field(str, len, field.offset);
	memcpy(conf->wifi_sta_ssid, field.value, 32);

	// PASS_STA
	field = next_field(str, len, field.offset);
	memcpy(conf->wifi_sta_pass, field.value, 64);

	// ENCRYPTION
	field = next_field(str, len, field.offset);
	conf->mqtt_encrypt = atoi(field.value);

	// HOST
	field = next_field(str, len, field.offset);
	memcpy(conf->mqtt_host, field.value, 64);

	// PORT
	field = next_field(str, len, field.offset);
	conf->mqtt_port = atoi(field.value);

	// USERNAME
	field = next_field(str, len, field.offset);
	memcpy(conf->mqtt_user, field.value, 64);

	// PASSWORD
	field = next_field(str, len, field.offset);
	memcpy(conf->mqtt_pass, field.value, 64);

	// APIKEY
	field = next_field(str, len, field.offset);
	memcpy(conf->mqtt_apikey, field.value, 32);

	// DEVICEID
	field = next_field(str, len, field.offset);
	memcpy(conf->mqtt_deviceid, field.value, 32);

	// PUB_TOPIC
	field = next_field(str, len, field.offset);
	memcpy(conf->mqtt_pub, field.value, 72);

	// PUB_QOS
	field = next_field(str, len, field.offset);
	conf->mqtt_pub_qos = atoi(field.value);

	// SUB_TOPIC
	field = next_field(str, len, field.offset);
	memcpy(conf->mqtt_sub, field.value, 72);

	// SUB_QOS
	field = next_field(str, len, field.offset);
	conf->mqtt_sub_qos = atoi(field.value);

	// ENCODE
	field = next_field(str, len, field.offset);
	conf->mqtt_encode = atoi(field.value);

	// Autoconfig for Fiware
	field = next_field(str, len, field.offset);
	conf->mqtt_autoconfig = atoi(field.value);

	// IN_MODE
	field = next_field(str, len, field.offset);
	conf->in_mode = atoi(field.value);

	// IN_KEY
	field = next_field(str, len, field.offset);
	memcpy(conf->in_key, field.value, 128);

	// IN_MIN
	field = next_field(str, len, field.offset);
	conf->in_min = atof(field.value);

	// IN_MAX
	field = next_field(str, len, field.offset);
	conf->in_max = atof(field.value);

	// IN_LOG_MODE
	field = next_field(str, len, field.offset);
	conf->in_log_mode = atoi(field.value);

	// IN_LOG_VALUE
	field = next_field(str, len, field.offset);
	conf->in_log_val = atof(field.value);

	// OUT_MODE
	field = next_field(str, len, field.offset);
	conf->out_mode = atoi(field.value);

	// OUT_KEY
	field = next_field(str, len, field.offset);
	memcpy(conf->out_key, field.value, 128);

	// OUT_MIN
	field = next_field(str, len, field.offset);
	conf->out_min = atof(field.value);

	// OUT_MAX
	field = next_field(str, len, field.offset);
	conf->out_max = atof(field.value);

	// tmp_bat_mode
	field = next_field(str, len, field.offset);
	conf->tmp_bat_mode = atoi(field.value);

	// tmp_bat_mode_val
	field = next_field(str, len, field.offset);
	conf->tmp_bat_mode_val = atoi(field.value);

	// tmp_1_enable
	field = next_field(str, len, field.offset);
	conf->tmp_1_enable = atoi(field.value);
	
	// tmp_1_name
	field = next_field(str, len, field.offset);
	memcpy(conf->tmp_1_name, field.value, 128);

	// tmp_1_min
	field = next_field(str, len, field.offset);
	conf->tmp_1_min = atof(field.value);
	
	// tmp_1_max
	field = next_field(str, len, field.offset);
	conf->tmp_1_max = atof(field.value);

	// tmp_1_log_mode
	field = next_field(str, len, field.offset);
	conf->tmp_1_log_mode = atoi(field.value);
	
	// tmp_1_log_mode_val
	field = next_field(str, len, field.offset);
	conf->tmp_1_log_mode_val = atof(field.value);
	
	// tmp_1_rtd
	field = next_field(str, len, field.offset);
	conf->tmp_1_rtd = atoi(field.value);
		
	// tmp_1_wires
	field = next_field(str, len, field.offset);
	conf->tmp_1_wires = atoi(field.value);
	
	// tmp_2_enable
	field = next_field(str, len, field.offset);
	conf->tmp_2_enable = atoi(field.value);
	
	// tmp_2_name
	field = next_field(str, len, field.offset);
	memcpy(conf->tmp_2_name, field.value, 128);

	// tmp_2_min
	field = next_field(str, len, field.offset);
	conf->tmp_2_min = atof(field.value);
	
	// tmp_2_max
	field = next_field(str, len, field.offset);
	conf->tmp_2_max = atof(field.value);

	// tmp_2_log_mode
	field = next_field(str, len, field.offset);
	conf->tmp_2_log_mode = atoi(field.value);
	
	// tmp_2_log_mode_val
	field = next_field(str, len, field.offset);
	conf->tmp_2_log_mode_val = atof(field.value);
	
	// tmp_2_rtd
	field = next_field(str, len, field.offset);
	conf->tmp_2_rtd = atoi(field.value);
		
	// tmp_2_wires
	field = next_field(str, len, field.offset);
	conf->tmp_2_wires = atoi(field.value);

    // calib enable
	field = next_field(str, len, field.offset);
	conf->calib_enable = atoi(field.value);

    // calib offset
	field = next_field(str, len, field.offset);
	conf->calib_offset = atof(field.value);

	// calib gain
	field = next_field(str, len, field.offset);
	conf->calib_gain = atof(field.value);

	// mod_10_num
	field = next_field(str, len, field.offset);
	conf->mod_10_num = atoi(field.value);

	// mod_11_status
	field = next_field(str, len, field.offset);
	conf->mod_11_status = atoi(field.value);

	// mod_11_name
	field = next_field(str, len, field.offset);
	memcpy(conf->mod_11_name, field.value, 128);

	// mod_11_adr
	field = next_field(str, len, field.offset);
	conf->mod_11_adr = atoi(field.value);

	// mod_11_cor
	field = next_field(str, len, field.offset);
	conf->mod_11_cor = atoi(field.value);

	// mod_11_func
	field = next_field(str, len, field.offset);
	conf->mod_11_func = atoi(field.value);

	// mod_11_log_mode
	field = next_field(str, len, field.offset);
	conf->mod_11_log_mode = atoi(field.value);
	
	// mod_11_log_mode_val
	field = next_field(str, len, field.offset);
	conf->mod_11_log_mode_val = atof(field.value);

	// mod_12_status
	field = next_field(str, len, field.offset);
	conf->mod_12_status = atoi(field.value);

	// mod_12_name
	field = next_field(str, len, field.offset);
	memcpy(conf->mod_12_name, field.value, 128);

	// mod_12_adr
	field = next_field(str, len, field.offset);
	conf->mod_12_adr = atoi(field.value);

	// mod_12_cor
	field = next_field(str, len, field.offset);
	conf->mod_12_cor = atoi(field.value);

	// mod_12_func
	field = next_field(str, len, field.offset);
	conf->mod_12_func = atoi(field.value);

	// mod_12_log_mode
	field = next_field(str, len, field.offset);
	conf->mod_12_log_mode = atoi(field.value);
	
	// mod_12_log_mode_val
	field = next_field(str, len, field.offset);
	conf->mod_12_log_mode_val = atof(field.value);

    // ntp synchronisation checkbox value
	field = next_field(str, len, field.offset);
	memcpy(conf->checkbox, field.value, 128);
	if (conf->checkbox[0] != 't' || conf->checkbox[1] != 'r' || conf->checkbox[2] != 'u'|| conf->checkbox[3] != 'e')
	{
       memcpy(conf->ntp_address, conf->checkbox, 128);
	   flag=1;
	}

	// ntp_address
	if ( flag==0)
	{
       field = next_field(str, len, field.offset);
	   memcpy(conf->ntp_address, field.value, 128);

	   field = next_field(str, len, field.offset);
	   memcpy(conf->sync_time, field.value, 128);
	}
	else
	{
	field = next_field(str, len, field.offset);
	memcpy(conf->sync_time, field.value, 128);
	flag=0;
	}


	return ESP_OK;
}

/* Read from NVS and print restart counter
   and the table with run times.
   Return an error if anything goes wrong
   during this process.
 */
esp_err_t init_CONF(aio_conf_t* aio_conf, aio_conf_init_t _init)
{
	ESP_LOGI(TAG, "Init NVS Memory module");

	ESP_ERROR_CHECK(nvs_flash_init());			// Init flash memory



	nvs_handle my_handle;
	
	// Open
	ESP_ERROR_CHECK(nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle));

	switch(_init) {
		case AIO_CONF_INIT_DEFAULT:
			ESP_LOGI(TAG, "NVS configuration override: using default");
			*aio_conf = aio_conf_default;
			write_aio_conf(aio_conf);
			nvs_close(my_handle);
		break;

		case AIO_CONF_INIT_CUSTOM:
			ESP_LOGI(TAG, "NVS configuration override: using custom");
			*aio_conf = aio_conf_custom;
			write_aio_conf(aio_conf);
			nvs_close(my_handle);
		break;

		case AIO_CONF_INIT_MEMORY:
		{
			// Read run time blob
			size_t required_size = 0;  // value will default to 0, if not set yet in NVS

			// obtain required memory space to store blob being read from NVS
			esp_err_t err;
			err = nvs_get_blob(my_handle, "aio_conf", NULL, &required_size);
			if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
				return err;

			if (required_size == 0) {
				ESP_LOGW(TAG, "NVS configuration not found: using default");
				*aio_conf = aio_conf_default;
				write_aio_conf(aio_conf);
				nvs_close(my_handle);
			}
			else {
				ESP_LOGI(TAG, "NVS configuration found: using memory");
				err = nvs_get_blob(my_handle, "aio_conf", aio_conf, &required_size);
				if (err != ESP_OK) {
					ESP_LOGE(TAG, "Memory reading error");
					nvs_close(my_handle);
					init_CONF(aio_conf, AIO_CONF_INIT_DEFAULT);
				}
			}
		}
		break;
	}
	
	return ESP_OK;
}

void add_unit(char *in_name, char *out_unit)
{
    char name[128];
    strcpy(name, in_name);
    char* unit = strrchr(name, '+');
    if (unit != NULL){
        unit++;
       
        if (strcmp(unit,".C") == 0)
            strcpy(out_unit, "°C");
        else if (strcmp(unit,"perc") == 0)
            strcpy(out_unit, "%");
        else if (strcmp(unit,"perm") == 0)
            strcpy(out_unit, "‰");
        else
            strcpy(out_unit, "\0");
    }else{
         strcpy(out_unit, "\0");
    }

     ESP_LOGI(TAG, "unit = %s", out_unit);
} 