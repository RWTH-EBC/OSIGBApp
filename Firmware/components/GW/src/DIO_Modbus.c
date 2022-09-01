// Copyright 2016-2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "DIO_Modbus.h"
#define TAG "DIO_Modbus"

#ifdef CONFIG_IOT_INTERFACE_DIO

static aio_conf_t *config; 



#define MB_PORT_NUM     (CONFIG_MB_UART_PORT_NUM)   // Number of UART port used for Modbus connection
#define MB_DEV_SPEED    (CONFIG_MB_UART_BAUD_RATE)  // The communication speed of the UART



#define MASTER_CHECK(a, ret_val, str, ...) \
    if (!(a)) { \
        ESP_LOGE(TAG, "%s(%u): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        return (ret_val); \
    }
 




void set_Modbus(int dev, uint8_t func, int adr, char *val)
{
    
    ESP_LOGI(TAG, "Set adress %d to value = %s", adr, val);
    esp_err_t err = ESP_OK;
    

    
     mb_param_request_t setparam = {dev, func, adr, 1};  //Device Adress, Func code (3 Read Holding Reg, 6 Write Input Reg), start Adr, reg length
    
    // int wert = 3000;  //Value to set or Variable for Output
  
    int Value = atoi(val);  //Value to set or Variable for Output
    if (config->mod_11_cor > 0)
        Value *= config->mod_11_cor;

    err = mbc_master_send_request(&setparam, &Value);
    switch (err)
    {
    case ESP_OK:
        ESP_LOGI(TAG, "Set to Value = %d", Value);
        break;

    case ESP_ERR_INVALID_ARG:
        ESP_LOGE(TAG, "invalid Argument");  
        break;  
    
    default:
       break;
    }

}

float get_Modbus(uint16_t dev, uint8_t func, uint16_t adr)
{
    uint16_t Value = 0;
    float   Value_cor = 100.00;
    esp_err_t err = ESP_OK;
    ESP_LOGD(TAG, "Get Values from Device: %d, Adress:  %d", dev, adr);

    
    mb_param_request_t setparam = {dev, func, adr, 1};  //Device Adress, Func code (3 Read Holding Reg, 6 Write Input Reg), start Adr, reg length
    ESP_LOGD(TAG, "parameters are set");

    // get the value from the modubs device
    ESP_LOGD(TAG, "Value before reading Device \n\t\t val = %d", Value);
    err = mbc_master_send_request(&setparam, &Value);
    ESP_LOGD(TAG, "val = %d", Value);
    switch (err)
    {
    case ESP_OK:
        ESP_LOGI(TAG, "Aktual Value = %u", Value);
        if (config->mod_11_cor > 0){
            Value_cor = (float)Value / config->mod_11_cor;
            return Value_cor;
        }else{
            return Value;
        }

    case ESP_ERR_INVALID_ARG:
        ESP_LOGE(TAG, "invalid Argument");  
        Value = 65000.00;
        return Value; 

    case ESP_ERR_TIMEOUT:
        ESP_LOGE(TAG, "Timeout");
        Value = 65000.00;
        return Value; 
    default:

       return Value;
    }
    
// return Value;
}

/* void add_unit(char *in_name, char *out_unit)
{
    char name[128];
    strcpy(name, in_name);
    char* unit = strrchr(name, '+');
    if (unit != NULL){
        unit++;
       
        if (strcmp(unit,".C") == 0)
            strcpy(out_unit, "°C");
        else if (strcmp(unit,"perc") == 0)
            strcpy(out_unit, "'%'");
        else if (strcmp(unit,"perm") == 0)
            strcpy(out_unit, "‰");
        else
            strcpy(out_unit, "\0");
    }else{
         strcpy(out_unit, "\0");
    }

     ESP_LOGI(TAG, "unit = %s", out_unit);
}  */

void init_modbus_devices(aio_conf_t* conf, datalink_t* input)
{
    input->num = 0;
    config = conf;

    if (conf->mod_11_status == READ_REGISTER)
	{
		input->num++;	


		input->in1.enable = 1;
		memcpy(input->in1.name, conf->mod_11_name, strlen(conf->mod_11_name));
		ESP_LOGD(TAG, "Name 1 = %s", input->in1.name );
		// input->in1.min = conf->tmp_1_min;
		// input->in1.max = conf->tmp_1_max;
		input->in1.log_mode = conf->mod_11_log_mode;
		input->in1.log_mode_val = conf->mod_11_log_mode_val;
        input->in1.cor = conf->mod_11_cor;
        add_unit(input->in1.name, input->in1.unit);
    }
    else if (conf->mod_11_status == WRITE_REGISTER )
    {
        input->in1.enable = 2;
        memcpy(input->in1.name, conf->mod_11_name, strlen(conf->mod_11_name));
        input->in1.cor = conf->mod_11_cor;
        add_unit(input->in1.name, input->in1.unit);
    }
    

    if (conf->mod_12_status == READ_REGISTER)
	{
		input->num++;	


		input->in2.enable = 1;
		memcpy(input->in2.name, conf->mod_12_name, strlen(conf->mod_12_name));
		ESP_LOGD(TAG, "Name 1 = %s", input->in2.name );
		// input->in2.min = conf->tmp_1_min;
		// input->in2.max = conf->tmp_1_max;
		input->in2.log_mode = conf->mod_12_log_mode;
		input->in2.log_mode_val = conf->mod_12_log_mode_val;
        input->in2.cor = conf->mod_12_cor;
        add_unit(input->in2.name, input->in2.unit);

    }

    else if (conf->mod_12_status == WRITE_REGISTER)
    {
        input->in2.enable = 2;
		memcpy(input->in2.name, conf->mod_12_name, strlen(conf->mod_12_name));        
        input->in2.cor = conf->mod_12_cor;        
        add_unit(input->in2.name, input->in2.unit);

    }
    ESP_LOGD(TAG, "Number of inputs = %d", input->num);
}

// Modbus master initialization
esp_err_t init_MODBUS(void)
{
    // Initialize and start Modbus controller
    mb_communication_info_t comm = {
        .port = MB_PORT_NUM,
#if CONFIG_MB_COMM_MODE_ASCII
        .mode = MB_MODE_ASCII,
#elif CONFIG_MB_COMM_MODE_RTU
        .mode = MB_MODE_RTU,
#endif
        .baudrate = MB_DEV_SPEED,
        .parity = MB_PARITY_NONE
    };
    void *master_handler = NULL;

    esp_err_t err = mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler);
    MASTER_CHECK((master_handler != NULL), ESP_ERR_INVALID_STATE,
                 "mb controller initialization fail.");
    MASTER_CHECK((err == ESP_OK), ESP_ERR_INVALID_STATE,
                 "mb controller initialization fail, returns(0x%x).",
                 (uint32_t)err);
    err = mbc_master_setup((void *)&comm);
    MASTER_CHECK((err == ESP_OK), ESP_ERR_INVALID_STATE,
                 "mb controller setup fail, returns(0x%x).",
                 (uint32_t)err);

    // Set UART pin numbers
    err = uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD, CONFIG_MB_UART_RXD,
                       CONFIG_MB_UART_RTS, UART_PIN_NO_CHANGE);

    err = mbc_master_start();
    MASTER_CHECK((err == ESP_OK), ESP_ERR_INVALID_STATE,
                 "mb controller start fail, returns(0x%x).",
                 (uint32_t)err);

    MASTER_CHECK((err == ESP_OK), ESP_ERR_INVALID_STATE,
                 "mb serial set pin failure, uart_set_pin() returned (0x%x).", (uint32_t)err);
    // Set driver mode to Half Duplex
    err = uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX);
    MASTER_CHECK((err == ESP_OK), ESP_ERR_INVALID_STATE,
                 "mb serial set mode failure, uart_set_mode() returned (0x%x).", (uint32_t)err);
    vTaskDelay(5);

    ESP_LOGI(TAG, "Modbus master stack initialized...");
    return err;
}

#endif 