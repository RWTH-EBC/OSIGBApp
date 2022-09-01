#include "cJSON_parse.h"
#include <string.h>
#include <stdio.h>
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_chip_info.h"
#include "esp_random.h"

 

static const char *TAG = "cJSON_parse";



void OnGotData(char *incomingBuffer, swop_payload_t* swop)
{
    
	cJSON *payload = cJSON_Parse(incomingBuffer);
    if (payload == NULL)
    {
         const char *error_ptr = cJSON_GetErrorPtr();
         if (error_ptr != NULL)
         {
             ESP_LOGE(TAG, "cJSON parse error %s\n", error_ptr);
         }
         memcpy(swop->typ, "0", 2);
         memcpy(swop->dpt, "0", 2);
         itoa(0.0,swop->val,10);

    }
    else{
  
        cJSON *type1 = cJSON_GetObjectItem(payload, "type");
        cJSON *datapoint = cJSON_GetObjectItem(payload, "datapoint");
        cJSON *value1 = cJSON_GetObjectItem(payload, "value");
        cJSON *cmd = cJSON_GetObjectItem(payload, "command");
        //  cJSON_bool acked = cJSON_GetObjectItem(payload, "acked");  //Not working 
        cJSON *smpl_rate = cJSON_GetObjectItem(payload, "sampling_rate");

        ESP_LOGD(TAG, "Type = \t%s", type1->valuestring);
	    ESP_LOGD(TAG, "Datapoint = \t%s", datapoint->valuestring);
	    ESP_LOGD(TAG, "Value = \t%d", value1->valueint);
    
        memcpy(swop->typ, type1->valuestring, strlen(type1->valuestring)+1);
        memcpy(swop->dpt, datapoint->valuestring, strlen(datapoint->valuestring)+1);
        itoa(value1->valueint,swop->val,10);
        if (cmd != NULL)
            swop->cmd = cmd->valueint;
        if (smpl_rate != NULL)
            swop->smpl_rate = smpl_rate->valueint;
    }
	cJSON_Delete(payload);
 
}

char* getStatus(uint8_t AIO_mode)
{
		switch (AIO_mode)
	{
	case AIO_DISABLED:
		// ESP_LOGD(TAG,"Analog %s: disabled", IO);
        return "Disabled";
		break;
	case AIO_VOLTAGE_0_10:
		// ESP_LOGI(TAG, "Analog %s: AIO_VOLTAGE_0_10", IO);
        return "AIO_VOLTAGE_0_10";
		break;
	case AIO_VOLTAGE_2_10:
		// ESP_LOGI(TAG, "Analog %s: AIO_VOLTAGE_2_10", IO);
        return "AIO_VOLTAGE_2_10";
		break;
	case AIO_CURRENT_0_20:
		// ESP_LOGI(TAG, "Analog %s: AIO_CURRENT_0_20", IO);
        return "AIO_CURRENT_0_20";
		break;
	case AIO_CURRENT_4_20:
		// ESP_LOGI(TAG, "Analog %s: AIO_CURRENT_4_20", IO);
        return "AIO_CURRENT_4_20";
		break;
	default:
        return "No_status";
		break;
	}
}


char *conf_req()
{
    char *string = NULL;
    cJSON *request = cJSON_CreateObject();

    //Type can be subscription or configuration
    cJSON_AddStringToObject(request, "type", "configuration");
    #ifdef CONFIG_IOT_INTERFACE_AIO
    cJSON_AddStringToObject(request, "mqtt", "Analog_GW");
     #elif CONFIG_IOT_INTERFACE_DIO
    cJSON_AddStringToObject(request, "mqtt", "Digital_GW");
    #elif CONFIG_IOT_INTERFACE_TMP
    cJSON_AddStringToObject(request, "mqtt", "Temperature_GW");
    #endif
    string = cJSON_Print(request);
    if (string == NULL)
        ESP_LOGE(TAG, "Failed to print config request String for fiware autoconfig");

    cJSON_Delete(request);
    return string;
}

char *get_Meta(uint8_t i, aio_conf_t *conf, datalink_t *input)
{
    char esp_name[32];
    static char time[64];
	if ( strlen(conf->wifi_ap_ssid) > 0)
		memcpy(esp_name, conf->wifi_ap_ssid, strlen(conf->wifi_ap_ssid));
	else {
		uint8_t chipid[6];
		
		esp_efuse_mac_get_default(chipid);
		sprintf(esp_name, "ESP_%02x%02x%02x%02x%02x%02x", chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);
    }

    char *string = NULL;
    // cJSON *name = NULL;
    // cJSON *device = NULL;
    // cJSON *ip = NULL;
    // cJSON *source = NULL;

    cJSON *meta = cJSON_CreateObject();

    switch (i)
    {
    case 1:
        cJSON_AddStringToObject(meta, "name", input->in1.name );
        cJSON_AddStringToObject(meta, "L5_unit", input->in1.unit );
        break;
    case 2:
        cJSON_AddStringToObject(meta, "name", input->in2.name );  
        cJSON_AddStringToObject(meta, "L5_unit", input->in2.unit );

        break;
    default:
        break;
    }




    cJSON_AddStringToObject(meta, "IP", input->ip );
   
    cJSON_AddStringToObject(meta, "Gateway-Name", esp_name );


    #ifdef CONFIG_IOT_INTERFACE_AIO
    switch (i)
    {
    case 1:
        cJSON_AddStringToObject(meta, "Status", getStatus(conf->in_mode));
        break;
    case 2:
        cJSON_AddStringToObject(meta, "Status", getStatus(conf->out_mode));
    default:
        break;
    }
    cJSON_AddStringToObject(meta, "source", "Analog_GW");
    #elif CONFIG_IOT_INTERFACE_DIO
    cJSON_AddNumberToObject(meta, "Device ID",conf->mod_10_num);
    
    switch (i)
    {
    case 1:
        cJSON_AddNumberToObject(meta, "Value-Correction", input->in1.cor);
        if (input->in1.enable == READ_REGISTER)
            cJSON_AddStringToObject(meta, "Status", "READ");
        else if(input->in1.enable == WRITE_REGISTER)
            cJSON_AddStringToObject(meta, "Status", "WRITE");
        else
            cJSON_AddStringToObject(meta, "Status", "Disabled");

        cJSON_AddNumberToObject(meta, "Function Code",conf->mod_11_func);
        cJSON_AddNumberToObject(meta, "Address",conf->mod_11_adr);
        break;
    case 2:
        cJSON_AddNumberToObject(meta, "Value-Correction", input->in2.cor);
        if (input->in2.enable == READ_REGISTER)
            cJSON_AddStringToObject(meta, "Status", "READ");
        else if(input->in2.enable == WRITE_REGISTER)
            cJSON_AddStringToObject(meta, "Status", "WRITE");
        else
            cJSON_AddStringToObject(meta, "Status", "Disabled");

        cJSON_AddNumberToObject(meta, "Function Code",conf->mod_12_func);
        cJSON_AddNumberToObject(meta, "Address",conf->mod_12_adr);
        break;
    default:
        break;
    }

    cJSON_AddStringToObject(meta, "source", "Digital_GW");
    #elif CONFIG_IOT_INTERFACE_TMP
        switch (i)
    {
    case 1:
        cJSON_AddStringToObject(meta, "Status", (input->in1.enable == 1) ? "Enabled" : "Disabled");     
        break;
    case 2:
        cJSON_AddStringToObject(meta, "Status", (input->in2.enable == 1 ) ? "Enabled" : "Disabled");
    default:
        break;
    }
    cJSON_AddStringToObject(meta, "Sleep-mode", (input->sleep == 1) ? "Enabled" : "Disabled");     
    cJSON_AddNumberToObject(meta, "Sleep time", input->sleeptime);     
    cJSON_AddStringToObject(meta, "source", "Temperature_GW");

    #endif
    gettime_meta(time);
    cJSON_AddStringToObject(meta, "Settime", time);
    cJSON_AddStringToObject(meta, "Version", input->ver);


    string = cJSON_Print(meta);
    if (string == NULL)
        ESP_LOGE(TAG, "Failed to print meta String");

    cJSON_Delete(meta);
    return string;
}

/**
 * @brief Update a string value in the config
 * 
 * reads the input JSON message and update 
 * the dependencies in the config variable/array. 
 * 
 * @param[in] mqtt_name: The name of the variable in the JSON message
 * @param[out] var_name:The name of the variable which should be update
 * @param[in] varlen: the stringlength of the input string
 * @param[in] payload: The payload in cJSON format
 */
esp_err_t upd_string(char *mqtt_name, char *var_name, int varlen, cJSON *payload)
{
    //cJSON *string= cJSON_GetObjectItem(payload, mqtt_name);
    cJSON *string=NULL;
    string= cJSON_GetObjectItem(payload, mqtt_name);
    if (cJSON_IsString(string))
    {
        if (strcmp(string->valuestring, var_name) != 0)
        {
            ESP_LOGD(TAG, "%s = \t%s", mqtt_name,string->valuestring);
            memset(var_name, 0, varlen);
            memcpy(var_name, string->valuestring, strlen(string->valuestring));
        }
        else 
        {
            ESP_LOGD(TAG, "No new value for %s", mqtt_name);
        }
        return ESP_OK;  
    }
    else
    {
        ESP_LOGE(TAG, "Failed to parse %s", mqtt_name);
        return ESP_FAIL;
    }
    
}
/**
 * @brief Update a integer value in the config
 * 
 * reads the input JSON message and update 
 * the dependencies in the config variable/array. 
 * 
 * @param[in] mqtt_name: The name of the variable in the JSON message
 * @param[out] var_name:The name of the variable which should be update as uint8_t
 * @param[in] payload: The payload in cJSON format
 */
esp_err_t upd_int(char *mqtt_name, uint8_t *var_name, cJSON *payload)
{
    int value;
    cJSON *msg= cJSON_GetObjectItem(payload, mqtt_name);
    value = atol(msg->valuestring);
    // value = cJSON_GetNumberValue(msg);
    if (value != *var_name)
    {
       ESP_LOGD(TAG, "%s = \t%s", mqtt_name, msg->valuestring);
       *var_name = value;
    //    ESP_LOGD(TAG, "New value = %d", *var_name);
    }
    else 
    {
        ESP_LOGD(TAG, "No new value for %s", mqtt_name);
    }
    return ESP_OK;  
}

/**
 * @brief Update a float value in the config
 * 
 * reads the input JSON message and update 
 * the dependencies in the config variable/array. 
 * 
 * @param[in] mqtt_name: The name of the variable in the JSON message
 * @param[out] var_name:The name of the variable which should be update as float
 * @param[in] payload: The payload in cJSON format
 */
esp_err_t upd_float(char *mqtt_name, float *var_name, cJSON *payload)
{
    float value;
    cJSON *msg= cJSON_GetObjectItem(payload, mqtt_name);
    value = atof(msg->valuestring);
    // value = cJSON_GetNumberValue(msg);
    if (value != *var_name)
    {
       ESP_LOGD(TAG, "%s = \t%s", mqtt_name, msg->valuestring);
       *var_name = value;
    //    ESP_LOGD(TAG, "New value = %d", *var_name);
    }
    else 
    {
        ESP_LOGD(TAG, "No new value for %s", mqtt_name);
    }
    return ESP_OK;  
}

void Read_Config(char *incomingBuffer, aio_conf_t *conf)
{
   	cJSON *payload = cJSON_Parse(incomingBuffer);
	char *string = cJSON_Print(payload);
    ESP_LOGD(TAG, "Incoming Configuration from fiware: \n %s", string);

	if (payload == NULL)
    {
         const char *error_ptr = cJSON_GetErrorPtr();
         if (error_ptr != NULL)
         {
             ESP_LOGE(TAG, "cJSON parse error %s\n", error_ptr);
         }
    }
    else
    {
        cJSON *mqtt=NULL;
        mqtt = cJSON_GetObjectItem(payload, "mqtt");
        // cJSON *mqtt = cJSON_GetObjectItem(payload, "mqtt");
        cJSON *mqtt2;
        cJSON_ArrayForEach(mqtt2, mqtt)
        {
            upd_string("pub_topic", conf->mqtt_pub, 72, mqtt2);
            upd_string("subscribe_topic", conf->mqtt_sub, 72, mqtt2);
        }

        cJSON *array=NULL;;
        cJSON *config=NULL;;
        #ifdef CONFIG_IOT_INTERFACE_AIO
        config = cJSON_GetObjectItem(payload, "Analog_GW");
        cJSON_ArrayForEach(array, config)
        {
			//Input
            upd_int("In_Phys", &conf->in_mode, array);
            upd_string("In_Name", conf->in_key, 128, array);
            upd_float("In_Min", &conf->in_min, array);
            upd_float("In_Max", &conf->in_max, array);
            upd_int("In_Mode", &conf->in_log_mode, array);
            upd_float("In_Mode_value", &conf->in_log_val, array);

			//Output
            upd_int("Out_Phys", &conf->out_mode, array);
            upd_string("Out_Name", conf->out_key, 128, array);
            upd_float("In_Min", &conf->in_min, array);
            upd_float("Out_Max", &conf->out_max, array);
        }
        #elif CONFIG_IOT_INTERFACE_DIO
        config = cJSON_GetObjectItem(payload, "Digital_GW"); 
        cJSON_ArrayForEach(array, config)
        {
            upd_int("Device_Number", &conf->mod_10_num, array);

			//Digital Device 1
            upd_int("1_State", &conf->mod_11_status, array);
            upd_string("1_Name", conf->mod_11_name, 128, array);
            upd_int("1_Address", &conf->mod_11_adr, array);
            upd_int("1_ValCorr", &conf->mod_11_cor, array);
            upd_int("1_cmd", &conf->mod_11_func, array);
            upd_int("1_Sampling", &conf->mod_11_log_mode, array);
            upd_float("1_Sampling_Rate", &conf->mod_11_log_mode_val, array);

			//Digital Device 1
            upd_int("2_State", &conf->mod_12_status, array);
            upd_string("2_Name", conf->mod_12_name, 128, array);
            upd_int("2_Address", &conf->mod_12_adr, array);
            upd_int("2_ValCorr", &conf->mod_12_cor, array);
            upd_int("2_cmd", &conf->mod_12_func, array);
            upd_int("2_Sampling", &conf->mod_12_log_mode, array);
            upd_float("2_Sampling_Rate", &conf->mod_12_log_mode_val, array);

		}
        #elif CONFIG_IOT_INTERFACE_TMP
        config = cJSON_GetObjectItem(payload, "Temperature_GW");
        cJSON_ArrayForEach(array, config)
        {
			//Input 1
            upd_int("In1", &conf->tmp_1_enable, array);
            upd_string("In1_Name", conf->tmp_1_name, 128, array);
            upd_float("In1_Min", &conf->tmp_1_min, array);
            upd_float("In1_Max", &conf->tmp_1_max, array);
            upd_int("In1_Mode", &conf->tmp_1_log_mode, array);
            upd_float("In1_Mode_value", &conf->tmp_1_log_mode_val, array);
            upd_int("In1_type", &conf->tmp_1_rtd, array);
            upd_int("In1_wires", &conf->tmp_1_wires, array);

			//Input 2
            upd_int("In2", &conf->tmp_2_enable, array);
            upd_string("In2_Name", conf->tmp_2_name, 128, array);
            upd_float("In2_Min", &conf->tmp_2_min, array);
            upd_float("In2_Max", &conf->tmp_2_max, array);
            upd_int("In2_Mode", &conf->tmp_2_log_mode, array);
            upd_float("In2_Mode_value", &conf->tmp_2_log_mode_val, array);
            upd_int("In2_type", &conf->tmp_2_rtd, array);
            upd_int("In2_wires", &conf->tmp_2_wires, array);
        }

        #endif
 
        conf->mqtt_autoconfig = 0;
    }
	cJSON_Delete(payload);
}

char *parse_JSON_out(char *in1_name, float val1, char *in2_name, float val2, char * time)
{
    char *payload = NULL;
    ESP_LOGD(TAG, "%f",val1 );
    // char val1s=(char)val1;
    // char val2s=(char)val2;
    char val1s[20];
    char val2s[20];
    char timest[45];
    sprintf(val1s, "%.2f", val1);
    sprintf(val2s, "%.2f", val2);
    sprintf(timest, "%s", time);


    ESP_LOGD(TAG, "%s",val1s );
    cJSON *msg = cJSON_CreateObject();
    //convert floats to string
    // snprintf(&val1s ,sizeof(val1s),"%f", val1);
    // snprintf(&val2s ,sizeof(val2s),"%f", val2);

    //Type can be subscription or configuration
    cJSON_AddStringToObject(msg, in1_name, val1s);
    cJSON_AddStringToObject(msg, in2_name, val2s);
     cJSON_AddStringToObject(msg, "timestamp", timest);

    
    payload = cJSON_Print(msg);
    if (payload == NULL)
        ESP_LOGE(TAG, "Failed to print payload for two inputs");
    ESP_LOGD(TAG, "Payload = %s", payload);
    cJSON_Delete(msg);
    return payload;
}