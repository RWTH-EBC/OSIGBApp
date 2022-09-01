#include "Mqtt.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "math.h"
#include <string.h>
#include "cJSON_parse.h" 
#include "led.h"
#include "sntp.h"
#include <esp_tls.h>

// RTC_DATA_ATTR bool meta = true;


#ifdef CONFIG_IOT_INTERFACE_AIO

#elif CONFIG_IOT_INTERFACE_DIO
	#include "DIO_Modbus.h"
#elif CONFIG_IOT_INTERFACE_TMP
	#include "TMP_sleep.h"
	// SemaphoreHandle_t MutexOTA;
	uint16_t count = 0; 
#endif

static float mqtt_old_val_1;
static float mqtt_old_val_2;
static float mqtt_new_val_1;
static float mqtt_new_val_2;

static mqtt_payload_t mqtt_payload;
static mqtt_payload_t mqtt_payload_2;
static mqtt_payload_t mqtt_payload_3;
// static char mqtt_payload_ms[128];
static char mqtt_sub_topic[138] = "";
static char mqtt_pub_topic[138] = "";
static char mqtt_ack_pub_topic[138] = "";
static char mqtt_upd_topic[138] = "";
static char mqtt_meta_topic[138] = "";

// Subscribe and publish topic for use with fiware service
static char mqtt_conf_sub_topic[138] = "";
static char mqtt_conf_pub_topic[138] = "";
// Variable for the status of the publish message to fiware
uint8_t conf_send = 0;
volatile uint64_t isr_ctr =0; 
// static uint64_t mqtt_timer_alarm;
// static uint64_t ntp_timer_alarm;
static TaskHandle_t mqtt_publisher_handle;
static TaskHandle_t mqtt_subscriber_handle;
static esp_mqtt_client_handle_t mqtt_client;
// static timer_config_t cfg = {TIMER_ALARM_EN, TIMER_PAUSE, TIMER_INTR_LEVEL, TIMER_COUNT_UP, TIMER_AUTORELOAD_EN, TIMER_SRC_CLK_DEFAULT, TIMER_DIVIDER};
gptimer_handle_t mqtt_gptimer = NULL;
// gptimer_handle_t ntp_gptimer = NULL;
gptimer_config_t timer_config = {
        .clk_src = SOC_MOD_CLK_APB,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
};

typedef struct {
    uint64_t event_count;
} example_queue_element_t;

time_t  now;
struct tm  ttm;
volatile uint8_t flag=0;
uint64_t time_us;
uint64_t timer_count=0;
uint64_t delay_ms =0;
uint64_t delay_us =0;
uint32_t CounterISR =0;
uint8_t indication=0;
// static uint64_t  dayAlarm= (uint64_t) (86400) * TIMER_SCALE ; // 24 hours for next alarm
// static uint64_t time_diff=0;
bool floatVal=false;
static const char *TAG = "Mqtt";
static aio_conf_t *conf;
static datalink_t *input;
char ntp_address[128]= "pool.ntp.org"; // default ntp server
extern struct timeval tv_now;
bool disconnect = false; 
char import_flag=0;
volatile double time_ns;
char timestamp[12]="timestamp";
char timerbuf0[37];
char timerbuf1[37];
float fval;
// #ifndef CONFIG_IOT_INTERFACE_TMP
static swop_payload_t swop;
// #endif


// convert to ISO6801 format
void getTimeISO( char* timerbuf)
{
  uint64_t milli = tv_now.tv_usec / 1000;

  char buf[sizeof("2011-10-08T07:07:09.000Z")];
  strftime(buf, sizeof (buf), "%FT%T", gmtime(&tv_now.tv_sec)); // gmtime convert to utc time in the given format
  sprintf(timerbuf, "%s.%lluZ", buf, milli);
}


/*	Scan MQTT payload
	for key-value pairs
 */
uint16_t mqtt_next_field(char* str, mqtt_payload_t* payload, uint16_t offset)
{
	uint16_t end = 0;
	uint16_t pos[4] = { 0,0,0,0 };
	uint16_t key_pos[2] = { 0,0};
	uint16_t val_pos[2] = { 0,0};
	int cnt=0;
	int key_cnt =0;
	int val_cnt =0;
 //	printf ("string is mqtt_next is %s \n",str);

	switch (conf->mqtt_encode)
	{

	/* new implementation for float values */
	case MQTT_ENCODE_JSON:

	 for (int i = offset; i <= strlen(str); i++)
	  {
		if (str[i] == '\"')
		{
		 key_pos[key_cnt++] = i;
		}
		if (str[i] == ':')
		{
		 val_pos[val_cnt++] = i;
		}
		if ((str[i] == '}')) 
		{
			end = i;
			val_pos[val_cnt++] = i;
			break;
		}
 	 }

		if (key_cnt < 2 && val_cnt < 2) {
			end = strlen(str);
			break;
		}
		memcpy(payload->key, (void*)&str[key_pos[0] + 1], key_pos[1] - key_pos[0] - 1);
		memcpy(payload->val, (void*)&str[val_pos[0] + 1], val_pos[1] - val_pos[0] - 1);
	//	memcpy(payload->val, (void*)&str[pos[2] + 1], pos[3] - pos[2] - 1);
		payload->key[key_pos[1] - key_pos[0] - 1] = '\0';
 		payload->val[val_pos[1] - val_pos[0] - 1] = '\0';
		ESP_LOGD(TAG, "JSON pair -> %s:%s", payload->key, payload->val);
		printf ("JSON pair-> %s:%s \n", payload->key, payload->val);
		break;

	/*
	case MQTT_ENCODE_JSON:
		for (int i = offset; i <= strlen(str); i++) {
			if (str[i] == '\"')
			{
			pos[cnt++] = i;
			printf("i for start is %d \n",i);
			}
			
			if ((str[i] == ';') || (str[i] == '}')) {
				end = i;
				printf("i for end is %d \n",i);
				break;
			}
		}

		if (cnt < 4) {
			end = strlen(str);
			break;
		}

		memcpy(payload->key, (void*)&str[pos[0] + 1], pos[1] - pos[0] - 1);
		memcpy(payload->val, (void*)&str[pos[2] + 1], pos[3] - pos[2] - 1);
		payload->key[pos[1] - pos[0] - 1] = '\0';
		payload->val[pos[3] - pos[2] - 1] = '\0';
		ESP_LOGD(TAG, "JSON pair:\t%s:%s", payload->key, payload->val);
		//printf ("JSON pair:\t%s:%s", payload->key, payload->val);
		break;
    */
	case MQTT_ENCODE_UL20:
		for (int i = offset; i <= strlen(str); i++) {
			end = i;
			if ((str[i] == '@') || (str[i] == '|') || (str[i] == '\0'))
				pos[cnt++] = i;
			if (cnt == 3)
				break;
		}

		if (cnt < 2) {
			end = strlen(str);
			break;
		}

		memcpy(payload->id, (void*)&str[offset], pos[0] - offset);
		memcpy(payload->key, (void*)&str[pos[0] + 1], pos[1] - pos[0] - 1);
		memcpy(payload->val, (void*)&str[pos[1] + 1], pos[2] - pos[1] - 1);
		payload->id[pos[0] - offset] = '\0';
		payload->key[pos[1] - pos[0] - 1] = '\0';
		payload->val[pos[2] - pos[1] - 1] = '\0';
		ESP_LOGD(TAG, "UL pair:\t%s:%s", payload->key, payload->val);
		break;

	case MQTT_ENCODE_AED:
		ESP_LOGE(TAG, "Switched to wrong case");
		break;

	default:
		break;
	}

	offset = end + 1;
	return offset;
}

/*	MQTT payload marshall to encode acknowledgment  
	using key-value pairs
	@value : time_ns
 */
void mqtt_setpoint_ack_marshall() 
{ 
	char* key="setpoint";
	sprintf(mqtt_payload.str, "{\"%s\":\"%s\"}", key, timerbuf0);
}

void mqtt_ack_marshall(float value) 
{  
	memset((void*)&mqtt_payload, 0, sizeof(mqtt_payload_t));
	char* key1="set_State";
//	#ifdef CONFIG_IOT_INTERFACE_AIO
	char* key2="t6";
  //  #endif
   time_ns=gettime_ns();
	getTimeISO(timerbuf0);
    switch (conf->mqtt_encode)
	     { 
           case MQTT_ENCODE_JSON:
		   sprintf(mqtt_payload.str, "{\"%s\":%.3f, \"%s\":\"%s\"}", key1, value, key2, timerbuf0);		 
		     break;

	      case MQTT_ENCODE_UL20:
		  sprintf(mqtt_payload.str, "%s|%.3f , %s|%s", key1, value, key2, timerbuf0);	
	     	break;

		   default:
		    break;
        } 
	//	ESP_LOGD(TAG, "Marshall_Ack:\t%s = %s	", key1, timerbuf0, key2, timerbuf0);
 } 

/*	MQTT payload marshall
	using key-value pairs
 */
void mqtt_pub_marshall(char* key, float value)
{   
	// int16_t random;
	memset((void*)&mqtt_payload_2, 0, sizeof(mqtt_payload_t));
	// time_ns=gettime_ns();
	getTimeISO(timerbuf0);

	switch (conf->mqtt_encode)
	{
	case MQTT_ENCODE_JSON:
		sprintf(mqtt_payload_2.str, "{\"%s\":%.3f, \"%s\":\"%s\"}", key, value, timestamp,timerbuf0);
		break;

	case MQTT_ENCODE_UL20:
		sprintf(mqtt_payload_2.str, "%s|%.3f , %s|%s", key, value, timestamp,timerbuf0);
		// sprintf(mqtt_payload_ms, mqtt_payload.str);
		break;
	
	case MQTT_ENCODE_AED:

	// time_us = (uint64_t)curTime.tv_sec * 1000000L + (uint64_t)curTime.tv_usec;
    // time_ns = time_us*1000;
		sprintf(mqtt_payload_2.str, "%s value=%.3f %.3f", key, value,time_ns);
		// sprintf(mqtt_payload_ms, "%s value=%.3f %lld", key, value, time_ns);
      ESP_LOGD(TAG, "Marshall:\t%s = %.3f with t=%.3f", key, value,time_ns);
		break;
	default:
		break;
  	} 	
}

/* MQTT payload marshall for 2 Values)
	Only for use with AED-Encoding
*/
void mqtt_marshall2(char* key1, float value1, char* key2, float value2)
{
	// int16_t random;
	memset((void*)&mqtt_payload, 0, sizeof(mqtt_payload_t));
	memset((void*)&mqtt_payload_2, 0, sizeof(mqtt_payload_t));
	// time_ns =gettime_ns();
	sprintf(mqtt_payload.str, "%s value=%.3f %.3f", key1, value1, time_ns);
	sprintf(mqtt_payload_2.str, "%s value=%.3f %.3f", key2, value2, time_ns);
}

/*	MQTT payload unmarshall
	using key-value pairs
 */
 void mqtt_unmarshall(char *str, char *key)
{
	uint16_t offset = 0;
	//printf( "key %s \n", key);
	do
	{
		offset = mqtt_next_field(str, &mqtt_payload, offset);
		if (strcmp(mqtt_payload.key, key) == 0)
			break;
		memset(mqtt_payload.key, 0, 32);
		memset(mqtt_payload.val, 0, 32);
	} while (offset < strlen(str));

}
void mqtt_JsonPrase(char *str)
{
	cJSON *payload = cJSON_Parse(str);
	char *string = cJSON_Print(payload);
	cJSON* value=NULL;
	ESP_LOGD(TAG, "Incoming Instruction: \n %s", string);
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
		  value= cJSON_GetObjectItem(payload, conf->out_key);
		  if (value != NULL)
		   {
		      strcpy(mqtt_payload.key,conf->out_key);
			 if (cJSON_IsString(value))
		 	    {
		    	 strcpy(mqtt_payload.val,value->valuestring);
			 	 ESP_LOGD(TAG, "output value is string %s \n", mqtt_payload.val);
				 floatVal=false;
		         }

			   else if (cJSON_IsNumber(value))
			    {
                  fval=value->valuedouble;
			      ESP_LOGD(TAG, "output value is dobue %.2f \n", fval);
			      sprintf(mqtt_payload.val,"%.2f",fval);
			      ESP_LOGD(TAG, "float double converted to string %s \n", mqtt_payload.val);
			      floatVal=true;
			    }
			   else
		       {
	             printf (" smth wrong");
			   }
		  }
		  
	}
      cJSON_Delete(payload);
	    //cJSON_Delete(value);
}

void publish_meta_tag()
{
	// 1
	char *meta1 = get_Meta(1, conf, input);
	ESP_LOGD(TAG, "META1: \n %s", meta1);
	esp_mqtt_client_publish(mqtt_client, mqtt_meta_topic, meta1, 0, conf->mqtt_pub_qos, 0);

	// 2
	char *meta2 = get_Meta(2, conf, input);
	ESP_LOGD(TAG, "META2: \n %s", meta2);
	esp_mqtt_client_publish(mqtt_client, mqtt_meta_topic, meta2, 0, conf->mqtt_pub_qos, 0);
}

#ifdef CONFIG_IOT_INTERFACE_TMP
void go_to_sleep()
{		
		// if (xSemaphoreTake(MutexOTA, 10000/portTICK_PERIOD_MS))
		// {
			ESP_LOGD(TAG, "Going to sleep for %d sec", conf->tmp_bat_mode_val/1000000);
			// vTaskDelay(1000/portTICK_PERIOD_MS);
			esp_mqtt_client_stop(mqtt_client);
			// esp_mqtt_client_destroy(mqtt_client);
			esp_wifi_disconnect();
			esp_wifi_stop();
			start_sleep(); 
		// }
}
#endif


/**
 * @brief MQTT subscriber task
 * handles incoming messages
 */
void mqtt_subscriber_task(void* arg)
{
	char out_key[64];
    char str[128];
	memcpy(str,conf->out_key,strlen(conf->out_key));
	//int i=0;
	//int c=0;
	float value;
	while (1)
	{
		// Wait timer
		xTaskNotifyWait(0x00, 0xffffffff, NULL, portMAX_DELAY);
	// #ifdef CONFIG_IOT_INTERFACE_TMP
	 	//if (xSemaphoreTake(MutexOTA, 1000/portTICK_PERIOD_MS))
	 	// {
	// #endif
	  //    if(xSemaphoreTake(mutex, 2000 / portTICK_PERIOD_MS)){
		strcpy(mqtt_payload.str,mqtt_payload_3.str);
		ESP_LOGD(TAG, "Incoming message (on topic): %s, %s\n", mqtt_sub_topic, mqtt_payload.str);
	//	printf ("Incoming message (on topic): %s, %s\n", mqtt_sub_topic, mqtt_payload.str);
		 
		switch (conf->mqtt_encode)
		{
			case MQTT_ENCODE_JSON:
			case MQTT_ENCODE_UL20:
				memset(out_key, 0, 64);
				if (conf->mqtt_encode == MQTT_ENCODE_UL20){
					strcat(out_key, conf->mqtt_deviceid);
					strcat(out_key, "@");
					strcat(out_key, conf->out_key);
				} else
				{
					strcpy(out_key, conf->out_key);
				}
		/*
    	   for(i=0; i<8; i++)  
           {
        	  if(mqtt_payload.str[i+2]==str[i])
        	   c++;
 	       }
			// printf("number %d\n ", c);
 	       if(c==8)
			 { 
               mqtt_JsonPrase(mqtt_payload.str);
			   ESP_LOGD(TAG, "setpoint instruction");
			 }
           else
		     { 
               mqtt_unmarshall(mqtt_payload.str, conf->out_key);
			   ESP_LOGD(TAG, " not a setpoint instruction");
		     }
			c=0;
       */
	        mqtt_unmarshall(mqtt_payload.str, conf->out_key);
		   // mqtt_JsonPrase(mqtt_payload.str);
			memcpy(swop.dpt, mqtt_payload.key, strlen(mqtt_payload.key));
			memcpy(swop.val, mqtt_payload.val, strlen(mqtt_payload.val));
			
			// for  setpoint Acknoledgment
			#ifdef CONFIG_IOT_INTERFACE_AIO
             value= atof(swop.val);
			//  printf ("Key: %f \n", value);
		  	 mqtt_ack_marshall(value);
			 esp_mqtt_client_publish(mqtt_client, mqtt_pub_topic, mqtt_payload.str, 0,conf->mqtt_pub_qos,0);
			 mqtt_setpoint_ack_marshall(); 
			 esp_mqtt_client_publish(mqtt_client, mqtt_ack_pub_topic, mqtt_payload.str, 0, conf->mqtt_pub_qos, 0);
            #endif

		//	printf ("Key: %s, Value: %s\n", swop.dpt,swop.val);
		//	printf ("out key: %s \n", conf->out_key);
		//	printf ("name1: %s, name2: %s \n", input->in1.name,input->in2.name);
					
			break;

			// Encoding on Aedifion Topics
			case MQTT_ENCODE_AED:

				if (strcmp(mqtt_payload.str, "update") == 0)
				{
					xSemaphoreGive(ota_semaphore);
					break;
				}
				if (strcmp(mqtt_payload.str, "restart") == 0)
				{
					printf("restarting in 5 seconds\n");
					led_blink(5);
					for (size_t i = 0; i < 5; i++)
					{
						uint8_t a = 5 - i; 
						printf("%d ...\n", a);
						vTaskDelay(pdMS_TO_TICKS(1000));
					}
					esp_restart();
				}
				if (strcmp(mqtt_payload.str, "time") == 0)
				{
				   if (conf->checkbox[0] == 't' && conf->checkbox[1] == 'r' && conf->checkbox[2] == 'u'&& conf->checkbox[3] == 'e')
					{
					 refresh_time();
					}
				}

				OnGotData(mqtt_payload.str, &swop);

				ESP_LOGD(TAG, "Type = \t\t%s",swop.typ);
				ESP_LOGD(TAG, "Datapoint = \t%s",swop.dpt);
				ESP_LOGD(TAG, "Value = \t\t%s",swop.val);
				ESP_LOGI(TAG, "SWOP: %s @ %s, value = %s", swop.typ, swop.dpt, swop.val);

			default:
				break;
		}
		
		if (strcmp(input->in1.name, swop.dpt) == 0 || strcmp(input->in2.name, swop.dpt) == 0)
		{
			ESP_LOGD(TAG, "For US");
			printf("for US \n");

			// Check if this is an user input
			switch (swop.cmd)
			{
			case update:
				xSemaphoreGive(ota_semaphore);
				break;
			
			case restart:
				printf("restarting in 5 seconds\n");
				led_blink(5);
				for (size_t i = 0; i < 5; i++)
				{
					uint8_t a = 5 - i; 
					printf("%d ...\n", a);
					vTaskDelay(pdMS_TO_TICKS(1000));
				}
				esp_restart();
				break;

			case reset_time:
			    if (conf->checkbox[0] == 't' && conf->checkbox[1] == 'r' && conf->checkbox[2] == 'u'&& conf->checkbox[3] == 'e')
			    {
                  refresh_time();
			    }
				break;
			
			default:
				break;
			}

			// Change the sampling rate

			if(swop.smpl_rate > 0)
			{
				ESP_LOGI(TAG, "New sampling rate = %d \n restart in 5 Seconds" , swop.smpl_rate);
				conf->tmp_1_log_mode_val = swop.smpl_rate;
				conf->mod_11_log_mode_val = swop.smpl_rate;
				conf->in_log_val = swop.smpl_rate;

				write_aio_conf(conf);
				led_blink(5);
				for (size_t i = 0; i < 5; i++)
				{
					uint8_t a = 5 - i; 
					printf("%d ...\n", a);
					vTaskDelay(pdMS_TO_TICKS(1000));
				}
				esp_restart();
							
			}

			if (strlen(swop.val) > 0)
			{
			#ifdef CONFIG_IOT_INTERFACE_AIO
			if (strcmp(input->in2.name, swop.dpt) == 0)
			{
				// itoa(value1->valueint,swop->val,10)							
			    float fsetValue= atof(swop.val);
			    uint32_t rest= (fsetValue-(uint32_t)fsetValue)*10;
				ESP_LOGD(TAG, "Setpoint adujsted succesfully");
				// printf("setval %f \n",fsetValue);
				AIO_set(fsetValue);  // set data on Analog Board
				//setpoint_save((uint32_t)fsetValue,rest );					 
			}
			else
			{
				ESP_LOGD(TAG, "Can not change set input. \n Maybe there is something wrong?");
			//	printf ("smth wrong");
			}
			
			#elif CONFIG_IOT_INTERFACE_DIO
			if (strcmp(input->in1.name, swop.dpt) == 0 && input->in1.enable == WRITE_REGISTER)
			set_Modbus(conf->mod_10_num,conf->mod_11_func,conf->mod_11_adr,swop.val);

			if (strcmp(input->in2.name, swop.dpt) == 0 && input->in2.enable == WRITE_REGISTER)
			set_Modbus(conf->mod_10_num,conf->mod_12_func,conf->mod_12_adr,swop.val);

			#endif
			}
		}

		else
		{
			ESP_LOGD(TAG, "Not four us");
			//printf(" NOT for US");
			
		}		
		// #endif		
	// xSemaphoreGive(mutex);
//	 	}
	}
	vTaskDelete(NULL);
}


/*	MQTT publisher task
	handles outgoing messages
 */
void mqtt_publisher_task(void* arg)
{
	static uint8_t err_1 = 0;
	static uint8_t err_2 = 0;

	while (1)
	{
		static char payload_1[128];
		static char payload_2[128];
		// Wait timer
		xTaskNotifyWait(0x00, 0xffffffff, NULL, portMAX_DELAY);
		  
		//if(xSemaphoreTake(mutex, 2000 / portTICK_PERIOD_MS)){

	  if (CounterISR == 3600)
	    { 
		   if (conf->checkbox[0] == 't' && conf->checkbox[1] == 'r' && conf->checkbox[2] == 'u'&& conf->checkbox[3] == 'e')
			{
              refresh_time(); // ntp time refresh
			}
		  CounterISR=0;
    	}

		/*
	   if (indication==1)
	    {
           indication=0;
			printf("synchronisation Done");
	    }
		*/

	   // printf (" input num %u \n",input->num);
		// Get data
		//	printf ("test after refresh\n");
		#ifdef CONFIG_IOT_INTERFACE_AIO
		if (input->num == 1){
			//start1=xTaskGetTickCount();
			//printf ("AIO test\n");
			mqtt_new_val_1 = AIO_get(1);
			//	printf ("input enable true\n");					// AIO Board input
			if (mqtt_new_val_1 == 65000.000){
				printf ("AIO test failed\n");
				err_1 = 1;
				ESP_LOGD(TAG,"Value wrong");
				continue;
			}else
			err_1 = 0;
		//	mqtt_new_val_1 =50;
		}else
		{
			//printf ("input enable false\n");	
					continue;
		}


		#elif CONFIG_IOT_INTERFACE_DIO
		ESP_LOGD(TAG, " status device %d adr %d = %d", conf->mod_10_num, conf->mod_11_adr, conf->mod_11_status);

		if(conf->mod_11_status == READ_REGISTER){
			mqtt_new_val_1 =  get_Modbus(conf->mod_10_num, conf->mod_11_func, conf->mod_11_adr);
				if (mqtt_new_val_1 == 65000.00){
					err_1 = 1;
					ESP_LOGD(TAG,"Value wrong");
				}else
					err_1 = 0;
		}

		ESP_LOGD(TAG, " status device %d adr %d = %d", conf->mod_10_num, conf->mod_12_adr, conf->mod_12_status);

		if(conf->mod_12_status == READ_REGISTER){		
		 mqtt_new_val_2 =  get_Modbus(conf->mod_10_num, conf->mod_12_func, conf->mod_12_adr);
		 	if (mqtt_new_val_2 == 65000.00){
				err_2 = 1;
				ESP_LOGD(TAG,"Value wrong 2");
			 }else
				err_2 = 0;
		}
		
		 
		#elif CONFIG_IOT_INTERFACE_TMP
           
      //  printf ("counterISR beginng %llu \n",counterISR );
		/*
		// Temperature Probe 1
		*/
		if (conf->tmp_1_enable)
		mqtt_new_val_1 = max31865_temperature_1();	
		
		if (mqtt_new_val_1 < input->in1.min || mqtt_new_val_1 > input->in1.max) 
		{
			ESP_LOGD(TAG, "Value 1 out of Range (Min = %.2f, Max = %.2f)", input->in1.min, input->in1.max);
			
		}
		else
		 {
			if (conf->calib_enable)
			{
				mqtt_new_val_1= (mqtt_new_val_1 * conf->calib_gain) + conf->calib_offset;
			}
		 }
			
		/*
		// Temperature Probe 2
		*/
		if (conf->tmp_2_enable)
		mqtt_new_val_2 = max31865_temperature_2();					
		if (mqtt_new_val_2 < input->in2.min || mqtt_new_val_2 > input->in2.max) 
		  {
			ESP_LOGD(TAG, "Value 2 out of Range (Min = %.2f, Max = %.2f)", input->in2.min, input->in2.max);
			
		  }
		else
		  {
			if (conf->calib_enable)
			{
		    	mqtt_new_val_2= (mqtt_new_val_2 * conf->calib_gain) + conf->calib_offset;
			}
		  }
			

		#endif

        time_ns=gettime_ns(); // get timestamp 

		switch (input->num)
		{
		case 0:
			ESP_LOGI(TAG, "All input's Disabled");{
				printf ("input enable =0\n");	
				#ifdef CONFIG_IOT_INTERFACE_TMP
				if(conf->tmp_bat_mode)
				go_to_sleep();
				#endif
				//xSemaphoreGive(mutex);
				continue;
			}
			// xSemaphoreGive(mutex);
			break;
		case 1:
			if (input->in1.enable == 1)
			{
				//	printf ("publush case 1\n");
				ESP_LOGD(TAG, "val_1=%.2f ", mqtt_new_val_1);
				//
				if (err_1 == 1){
				#ifdef CONFIG_IOT_INTERFACE_TMP
				if(conf->tmp_bat_mode)
				go_to_sleep();
				#endif
				printf ("error test =1\n");	
			//xSemaphoreGive(mutex);
				continue;
			}

				// Check report mode
				if (input->in1.log_mode == MQTT_LOG_THRESHOLD)
				{
					ESP_LOGD(TAG, "Log Mode Threshold \n old val 1 = %.2f", mqtt_old_val_1);

					if (fabsf(mqtt_new_val_1 - mqtt_old_val_1) < input->in1.log_mode_val)
					{
						#ifdef CONFIG_IOT_INTERFACE_TMP
						if(conf->tmp_bat_mode)
						go_to_sleep();
						#endif
					//	xSemaphoreGive(mutex);
						continue;
					}
				}	
				mqtt_old_val_1 = mqtt_new_val_1;
				mqtt_pub_marshall(input->in1.name, mqtt_new_val_1);
				//printf ("input enabled marshal=1\n");			       
			}
			else if (input->in2.enable == 1)
			{
				//printf ("publush case 1\n");
				ESP_LOGD(TAG, "val_2=%.2f ", mqtt_new_val_2);
				if (err_2 == 1){
				#ifdef CONFIG_IOT_INTERFACE_TMP
				if(conf->tmp_bat_mode)
				go_to_sleep();
				#endif
				//xSemaphoreGive(mutex);
				continue;
			}

				// Check report mode
				if (input->in2.log_mode == MQTT_LOG_THRESHOLD)
				{
					ESP_LOGD(TAG, "Log Mode Threshold \n old val 2 = %.2f", mqtt_old_val_2);
					if (fabsf(mqtt_new_val_2 - mqtt_old_val_2) < input->in2.log_mode_val){
				#ifdef CONFIG_IOT_INTERFACE_TMP
				if(conf->tmp_bat_mode)
				go_to_sleep();
				#endif
				//xSemaphoreGive(mutex);
				continue;
			}
				}
				
				mqtt_old_val_2 = mqtt_new_val_2;
				mqtt_pub_marshall(input->in2.name, mqtt_new_val_2);
			}

			// sprintf(mqtt_payload.str, mqtt_payload_ms);
			// xSemaphoreGive(mutex);
			break;
		case 2:
			ESP_LOGD(TAG, "Values \n\t\t\t val_1 \t= %.2f \n\t\t\t val_2 \t= %.2f", mqtt_new_val_1, mqtt_new_val_2);
			//Check if both values are out of range
				// If true continue (send no updates to server)
			if(err_1 == 1 && err_2 ==1){
				#ifdef CONFIG_IOT_INTERFACE_TMP
				if(conf->tmp_bat_mode)
				go_to_sleep();
				#endif
				//xSemaphoreGive(mutex);
				continue;
			}

			// Check report mode
			if (input->in1.log_mode == MQTT_LOG_THRESHOLD && input->in2.log_mode == MQTT_LOG_THRESHOLD)
			{
				ESP_LOGD(TAG, "Log Mode Threshold, old val1 = %.2f, old val2 = %.2f", mqtt_old_val_1, mqtt_old_val_2);
				if (fabsf(mqtt_new_val_1 - mqtt_old_val_1) < input->in1.log_mode_val && fabsf(mqtt_new_val_2 - mqtt_old_val_2) < input->in2.log_mode_val)
				{
#ifdef CONFIG_IOT_INTERFACE_TMP
					if (conf->tmp_bat_mode)
						go_to_sleep();
#endif
				//	xSemaphoreGive(mutex);
					continue;
				}
			}
		//	ESP_LOGD(TAG, "Setting New values to old ones");
			mqtt_old_val_1 = mqtt_new_val_1;
			mqtt_old_val_2 = mqtt_new_val_2;

			if(err_1 == 0 && err_2 == 0)
			{         
				switch (conf->mqtt_encode)
				{
					case MQTT_ENCODE_JSON:
				//	stop3=xTaskGetTickCount()-start3;
	           //     printf ("t2= %llu \t",stop3 );
				//	ESP_LOGD(TAG, "Parsing JSON");
					 getTimeISO(timerbuf1);
					 sprintf(mqtt_payload_2.str, "{\"%s\":%.3f, \"%s\":%.3f, \"%s\":\"%s\"}", input->in1.name,mqtt_new_val_1,input->in2.name, mqtt_new_val_2, timestamp,timerbuf1);
				//	char *payload = parse_JSON_out(input->in1.name, mqtt_new_val_1, input->in2.name, mqtt_new_val_2, timerbuf1);
				//	sprintf(mqtt_payload.str, "%s", payload);
				//	free(payload);
					ESP_LOGD(TAG, "New String: %s", mqtt_payload.str);
					break;
					case MQTT_ENCODE_UL20:
					case MQTT_ENCODE_AED:
						mqtt_pub_marshall(input->in1.name, mqtt_new_val_1);
						// sprintf(payload_1, mqtt_payload_ms);
						sprintf(payload_1, mqtt_payload_2.str);
						mqtt_pub_marshall(input->in2.name, mqtt_new_val_2);
						sprintf(payload_2, mqtt_payload_2.str);
						sprintf(mqtt_payload_2.str, "%s \n%s", payload_1 , payload_2);
						break;
				}
			}else if (err_1 == 1) {
				 mqtt_old_val_2 = mqtt_new_val_2;
				mqtt_pub_marshall(input->in2.name, mqtt_new_val_2);
				// sprintf(mqtt_payload.str, mqtt_payload_ms);
			}else if(err_2 == 1){
				 mqtt_old_val_1 = mqtt_new_val_1;
				mqtt_pub_marshall(input->in1.name, mqtt_new_val_1);
				// sprintf(mqtt_payload.str, mqtt_payload_ms);
			}else{
				ESP_LOGD(TAG, "Something went wrong, err_1 = %d, err_2 = %d", err_1, err_2);
			}

			// mqtt_marshall2(conf->in_key, mqtt_new_val_1, "RoomTemperaturel", mqtt_new_val_2);	
			//xSemaphoreGive(mutex);
			break;
		default:
		// xSemaphoreGive(mutex);
			break;
		}

	// Publish data
	if (disconnect)
		break;

   // start5=xTaskGetTickCount();
   // printf (" before publish \n");	
	esp_mqtt_client_publish(mqtt_client, mqtt_pub_topic, mqtt_payload_2.str, 0, conf->mqtt_pub_qos, 0);
	gptimer_get_raw_count(mqtt_gptimer, &timer_count);
//	printf ("timer count = %llu \n", timer_count);
//	timer_count=0;
//	printf ("counter ISR last %d \n", CounterISR);
	//printf ("published\n");	
//	if (counterISR > 0)
//	{
//	printf ("counterISR  %llu \n",counterISR );
//	}
	//printf ("published\n");
	ESP_LOGI(TAG, "Outgoing message on topic: %s, %s", mqtt_pub_topic, mqtt_payload_2.str);
	//stop5=xTaskGetTickCount()-start5;
	// printf ("t3= %llu \n",stop5 );
	mqtt_payload.str[0] = '\0';
	#ifdef CONFIG_IOT_INTERFACE_TMP
	if (conf->tmp_bat_mode == 1)
		go_to_sleep();
	#endif
		//xSemaphoreGive(mutex);
	 //	}
	}

	vTaskDelete(NULL);

}


void disconnect_mqtt()
{
	disconnect = true;
	
	if (strlen(mqtt_sub_topic) > 1)	
		esp_mqtt_client_unsubscribe(mqtt_client,mqtt_sub_topic);
	

	// vTaskDelete(&mqtt_publisher_handle);
	// vTaskDelete(&mqtt_subscriber_handle);

	esp_mqtt_client_disconnect(mqtt_client);
	esp_mqtt_client_destroy(mqtt_client);
//	ESP_ERROR_CHECK(timer_pause(TIMER_GROUP_0, TIMER_0));
//	ESP_ERROR_CHECK(timer_deinit(TIMER_GROUP_0, TIMER_0));
	ESP_ERROR_CHECK(gptimer_stop(mqtt_gptimer));

}

/*	MQTT event handler
	Finite State Machine
 */
esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
	switch (event->event_id) {
	case MQTT_EVENT_ANY:
		break;
		
	case MQTT_EVENT_CONNECTED:
		ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");

		if (strlen(mqtt_sub_topic) > 1)									// SUBSCRIBE Topic
		{
			esp_mqtt_client_subscribe(mqtt_client, mqtt_sub_topic, conf->mqtt_sub_qos);
		}
		if (strlen(mqtt_upd_topic) > 1)	
		{
			esp_mqtt_client_subscribe(mqtt_client, mqtt_upd_topic, conf->mqtt_sub_qos);
		}
		if(conf->mqtt_autoconfig == 1)
		{
			esp_mqtt_client_subscribe(mqtt_client, mqtt_conf_sub_topic, conf->mqtt_sub_qos);
		}
	
		if (strlen(mqtt_pub_topic) > 1)
		{
		    time_ns=gettime_ns(); // get time at the start
			ESP_ERROR_CHECK(gptimer_start(mqtt_gptimer));
		//	ESP_ERROR_CHECK(timer_start(TIMER_GROUP_0, TIMER_0)); // Start PUBLISH Timer
			ESP_LOGD(TAG, "init Time: %.3f ", time_ns);	
		}	
		break;

	case MQTT_EVENT_DISCONNECTED:
		ESP_ERROR_CHECK(gptimer_stop(mqtt_gptimer));
	//	ESP_ERROR_CHECK(timer_pause(TIMER_GROUP_0, TIMER_0));			// Pause PUBLISH Timer
		ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
		ESP_ERROR_CHECK(esp_mqtt_client_reconnect(mqtt_client));
		break;

	case MQTT_EVENT_SUBSCRIBED:
		// ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, Topic =%s", event->msg_id, event->topic);
		ESP_LOGD(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
		if (conf->mqtt_autoconfig == 1 && conf_send == 0)
		{
			char *conf_payload = conf_req();
			esp_mqtt_client_publish(mqtt_client, mqtt_conf_pub_topic, conf_payload, 0,conf->mqtt_pub_qos,0);
			conf_send = 1;
			ESP_LOGD(TAG, "Configuration sent. \n %s", conf_payload);
		}
		break;

	case MQTT_EVENT_UNSUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
		break;

	case MQTT_EVENT_PUBLISHED:
		ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		break;

	case MQTT_EVENT_DATA:
	    ESP_LOGI(TAG, "MQTT_EVENT_DATA");	
		// printf(" MQTT_EVENT_DATA");
	//	if(xSemaphoreTake(mutex, 2000 / portTICK_PERIOD_MS))
	//	{
		// ESP_LOGD(TAG, "MQTT_EVENT_DATA mutex acquired \n");
		 memset(mqtt_payload_3.str, 0, 32);
		 memcpy(mqtt_payload_3.str, event->data, event->data_len);  
		 char topic[138];
		 memset(topic, 0, 138);
		 memcpy(topic, event->topic, event->topic_len);

		 if (conf->mqtt_autoconfig == 1 && strcmp(topic, mqtt_conf_sub_topic) == 0)
		 {	
			ESP_LOGI(TAG, "Receive configuration: %s", mqtt_payload.str);
			Read_Config(mqtt_payload_3.str, conf);
			write_aio_conf(conf);
		//	xSemaphoreGive(mutex);
			disconnect_mqtt();
				for (size_t i = 0; i < 5; i++)
				{
					uint8_t a = 5 - i; 
					printf("%d ...\n", a);
					led_blink(1);
					// vTaskDelay(pdMS_TO_TICKS(1000));
				}
				esp_restart();
		 }
		 else
	//	   {
	//	   xSemaphoreGive(mutex);
		   xTaskNotify(mqtt_subscriber_handle, 0x00, eNoAction); 
	//	   }
		 break;
	//	}	
	case MQTT_EVENT_BEFORE_CONNECT:
		break;

	case MQTT_EVENT_ERROR:
		if (event->error_handle->connect_return_code == MQTT_CONNECTION_REFUSE_NOT_AUTHORIZED || event->error_handle->connect_return_code == MQTT_CONNECTION_REFUSE_BAD_USERNAME)
		{
			ESP_LOGE(TAG, "Problem with MQTT credentials, connect_return_code = %d", event->error_handle->connect_return_code);
			led_fast(3);
			break;	
		}
		else
		{		
		ESP_LOGE(TAG, "MQTT_EVENT_ERROR \n Try to restart gateway");
		printf("event error code: %d \n",event->error_handle->connect_return_code); 
		 // Return mqtt error return code
		restart_on_failure();
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		break;
		}
	case MQTT_EVENT_DELETED:
		break;
	}
	return ESP_OK;
}

#ifdef CONFIG_testmode
//Only used for testing wifi strength
void wifi_signal()
{
	led_blink(3);
	uint16_t number = 10;
    wifi_ap_record_t ap_info[10];
    uint16_t ap_count = 0;
	memset(ap_info, 0, sizeof(ap_info));
	char wifi[512]; 


	ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
	 for (int i = 0; (i < 10) && (i < ap_count); i++) {
        ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
        ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
		ESP_LOGI(TAG, "Channel \t\t%d\n", ap_info[i].primary);
		sprintf(wifi,"Name \t\t %s \nSSID \t\t%s \nRSSI \t\t%d \nChannel \t\%d\n",conf->wifi_ap_ssid ,ap_info[i].ssid,ap_info[i].rssi,ap_info[i].primary );
		esp_mqtt_client_publish(mqtt_client, "mtw/nextgenbat_testhall/wifi", wifi, 0, conf->mqtt_pub_qos, 0);
	 }
	led_blink(1);
}
#endif


/*	Timer task ISR
 */
static bool IRAM_ATTR mqtt_timer_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
   	CounterISR++;

	// Notify task
	#ifdef CONFIG_IOT_INTERFACE_TMP		
	if (conf->tmp_bat_mode == 1)
	{
		if (count == 0)
		xTaskNotifyFromISR(mqtt_publisher_handle, 0x00, eNoAction, NULL);
		portYIELD_FROM_ISR();
		count++;
	}else{
		xTaskNotifyFromISR(mqtt_publisher_handle, 0x00, eNoAction, NULL);
		portYIELD_FROM_ISR();
	}
	#endif
	#ifndef CONFIG_IOT_INTERFACE_TMP

		xTaskNotifyFromISR(mqtt_publisher_handle, 0x00, eNoAction, NULL);
		portYIELD_FROM_ISR();
	#endif

	//gptimer_get_raw_count(timer, &timer_count);

	return ESP_OK;

}

/*
static bool IRAM_ATTR ntp_timer_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data)
{
	 timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0x00000000ULL);

    TIMERG1.int_clr_timers.t0_int_clr = 1; // Clear interrupt

     if (flag ==0)
	 {
      timer_set_alarm_value(TIMER_GROUP_1, TIMER_0, dayAlarm); // set the alarm value
	  flag=1;
    }
	indication=1;

	timer_set_alarm(TIMER_GROUP_1, TIMER_0, TIMER_ALARM_EN); // enable it	

     	 
   // xTaskNotifyFromISR(mqtt_publisher_handle, 0x00, eNoAction, NULL);

   	return ESP_OK;
 }
 */

esp_err_t init_MQTT_timer()
{
	 delay_ms = 1000; // default value is 1sec
	if (input->in1.log_mode == MQTT_LOG_PERIODIC)
		if ( fabs(input->in1.log_mode_val - 0.0) > 0.001)
		{
			ESP_LOGD(TAG, "Periodic Sampling Rate[ms] = %.2f", input->in1.log_mode_val);
			delay_ms = input->in1.log_mode_val;
		}
#ifdef CONFIG_IOT_INTERFACE_TMP		
	if (conf->tmp_bat_mode == 1)
		delay_ms = 500;
#endif

    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &mqtt_gptimer));

	gptimer_alarm_config_t mqtt_alarm_config = {
		.reload_count = 0,
        .alarm_count = delay_ms*1000,  // ( 1 tick = 1 us)
		.flags.auto_reload_on_alarm = true,
		
    };
	ESP_ERROR_CHECK(gptimer_set_alarm_action(mqtt_gptimer, &mqtt_alarm_config));

    gptimer_event_callbacks_t mqtt_cbs = {
        .on_alarm = mqtt_timer_isr,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(mqtt_gptimer, &mqtt_cbs, NULL));

	// ESP_LOGI(TAG, "Refresh interval set to: %d ms", delay_ms);
	ESP_LOGI(TAG, "MQTT Timer enable is done ! \n" );
	return ESP_OK;
}

esp_err_t init_NTP_timer()
{
	uint8_t hours=0;
	uint8_t minutes=0;
	uint8_t seconds=0;
	uint16_t mseconds=0;
	uint16_t temp_ms=0;
	   
	if (conf->checkbox[0] == 't' && conf->checkbox[1] == 'r' && conf->checkbox[2] == 'u' && conf->checkbox[3] == 'e') // checkbox enabled
	{
		/* synchronise with ntp server */
      if (conf->ntp_address[0] != '\0') // store the input ntp server address
	  {
        memcpy(ntp_address,conf->ntp_address,strlen(conf->ntp_address)+1);
		ESP_LOGI(TAG, "NTP Server is set to: %s ",ntp_address);
	  }	
      time_ns=gettime_ns();
	  ttm = *localtime(&tv_now.tv_sec);
	
	  hours=23-ttm.tm_hour;
	  minutes=59-ttm.tm_min;
	  seconds=59-ttm.tm_sec;
	  temp_ms=(tv_now.tv_usec/1000);
	  mseconds=999-(tv_now.tv_usec/1000); // taking into account time for refreshment
	  ESP_LOGI(TAG, "Time now: %dh %dm %ds %dms ", ttm.tm_hour , ttm.tm_min , ttm.tm_sec,temp_ms);
	}

   // take the time from the user
    else 
    {
		struct tm timeinfo; 
		char buf[5];
	    snprintf(buf, 5, &conf->sync_time[0], &conf->sync_time[1], &conf->sync_time[2], &conf->sync_time[3]);
		timeinfo.tm_year=atoi(buf)-1900; // year since 1900
		//printf ("year= %d \n", timeinfo.tm_year);
        snprintf(buf, 3, &conf->sync_time[5], &conf->sync_time[6]);
		timeinfo.tm_mon=atoi(buf)-1;      // month since jan ( 0 till 11)
		//printf ("month= %d \n", timeinfo.tm_mon);
		snprintf(buf, 3,&conf->sync_time[8], &conf->sync_time[9]);
		timeinfo.tm_mday=atoi(buf); 
		//printf ("day= %d \n", timeinfo.tm_mday);
    
		snprintf(buf, 3,&conf->sync_time[11], &conf->sync_time[12]);
		timeinfo.tm_hour=atoi(buf); 
		hours=23-timeinfo.tm_hour; 
		//printf ("hour= %d \n", timeinfo.tm_hour);
	    snprintf(buf, 3, &conf->sync_time[14], &conf->sync_time[15]);
		timeinfo.tm_min=atoi(buf);
		minutes=59-timeinfo.tm_min;
		//printf ("min= %d \n", timeinfo.tm_min);
		snprintf(buf, 3, &conf->sync_time[17], &conf->sync_time[18]);
		timeinfo.tm_sec=atoi(buf);  
		seconds=59-timeinfo.tm_sec;
		//printf ("sec= %d \n", timeinfo.tm_sec);
		mseconds=59;
		timeinfo.tm_wday=0;
		timeinfo.tm_yday=0;
	
		tv_now.tv_sec = mktime(&timeinfo); // convert to time_t (seconds)
		tv_now.tv_usec=0;
		settimeofday(&tv_now,NULL); // set system time
		ESP_LOGI(TAG, "Time now: %dh %dm %ds %dms ", timeinfo.tm_hour , timeinfo.tm_min , timeinfo.tm_sec,mseconds);
	}
    
   /*
	uint64_t time_diff=(hours*3600)+(minutes*60)+seconds; // in seconds
    ntp_timer_alarm  = (uint64_t) time_diff * TIMER_SCALE;
	ntp_timer_alarm += (uint64_t) mseconds * TIMER_SCALE / 1000ULL;
    ESP_LOGI(TAG, "Next Sync refesh in: %d h %d m %d s %d ms", hours , minutes , seconds,mseconds);
 // curTime.tv_sec=curTime.tv_sec+1; // offset error
 // curTime.tv_usec=curTime.tv_usec+500000; // offset error

	ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &ntp_gptimer));

	gptimer_alarm_config_t ntp_alarm_config = {
		.reload_count = 0,
        .alarm_count = 60000000, // ( 1 tick = 1us)
		.flags.auto_reload_on_alarm = true,	
    };
	ESP_ERROR_CHECK(gptimer_set_alarm_action(mqtt_gptimer, &ntp_alarm_config));
    gptimer_event_callbacks_t ntp_cbs = {
       .on_alarm = ntp_timer_isr,
    };

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(ntp_gptimer, &ntp_cbs, NULL));
	ESP_ERROR_CHECK(gptimer_start(ntp_gptimer));

	 ESP_LOGI(TAG, "Sync Timer start is done !" );
  */
	return ESP_OK;
}

esp_err_t init_MQTT_topics()
{
	char root[140] = "";
	ESP_LOGD(TAG, "Init MQTT topics");
	switch (conf->mqtt_encode)
	{
	case MQTT_ENCODE_JSON:
	case MQTT_ENCODE_UL20:
		
		 

		if (strlen(conf->mqtt_apikey) > 0) {
			strcat(root, "/");
			strcat(root, conf->mqtt_apikey);
		}
		if (strlen(conf->mqtt_deviceid) > 0) {
			strcat(root, "/");
			strcat(root, conf->mqtt_deviceid);
		}
		if (strlen(conf->mqtt_pub) > 0) {
			strcpy(mqtt_pub_topic, "/json");
			strcat(mqtt_pub_topic, root);
			strcat(mqtt_pub_topic, "/");
			strcat(mqtt_pub_topic, conf->mqtt_pub);		
		}
         strcpy(mqtt_ack_pub_topic, "/json"); 
         strcat(mqtt_ack_pub_topic, root);
         strcat(mqtt_ack_pub_topic, "/cmdexe"); // ack topic


		if (strlen(conf->mqtt_sub) > 0) {
			strcat(mqtt_sub_topic, root);
			strcat(mqtt_sub_topic, "/");
			strcat(mqtt_sub_topic, conf->mqtt_sub);
		}
		if (conf->mqtt_autoconfig == 1 && strlen(root) > 0)
		{
			strcpy(mqtt_conf_pub_topic, "/json");
			strcat(mqtt_conf_pub_topic, root);
			strcat(mqtt_conf_pub_topic, "/configuration/commands");

			strcpy(mqtt_conf_sub_topic, root); 	
			strcat(mqtt_conf_sub_topic, "/configuration/values");
		}

		break;
	
	case MQTT_ENCODE_AED:

		if (strlen(conf->mqtt_pub) > 0) {			
			strcpy(mqtt_sub_topic, "CONTROLS/");
			strcat(mqtt_sub_topic, conf->mqtt_pub);
			strcpy(mqtt_pub_topic, conf->mqtt_pub);
			strcat(mqtt_pub_topic, "/");
			strcpy(mqtt_meta_topic, "META/");
			strcat(mqtt_meta_topic, conf->mqtt_pub);
			strcpy(mqtt_upd_topic, conf->mqtt_pub);
			strcat(mqtt_upd_topic, "/update");

		}

/* 		if (strlen(conf->mqtt_deviceid) == 0) {
			strcpy(conf->mqtt_deviceid, "ESP32_%CHIPID%");
			ESP_LOGD(TAG, "Device ID = %s", conf->mqtt_deviceid);

		}
 */

		break;
	default:
		break;
	}
	// Configure Topics
	ESP_LOGI(TAG, "PUB topic: \t%s", mqtt_pub_topic);
	ESP_LOGI(TAG, "SUB topic: \t%s", mqtt_sub_topic);
	ESP_LOGI(TAG, "ACK PUB topic: \t%s", mqtt_ack_pub_topic);
	// ESP_LOGI(TAG, "Conf Pub topic: \t%s", mqtt_conf_pub_topic);
	// ESP_LOGI(TAG, "Conf Sub topic: \t%s", mqtt_conf_sub_topic);
	ESP_LOGI(TAG, "META topic:\t %s", mqtt_meta_topic);

	return ESP_OK;
}

esp_err_t init_MQTT(aio_conf_t* aio_conf, datalink_t* input_in)
{
	ESP_LOGI(TAG, "Init MQTT module");

	conf = aio_conf;
	input = input_in;
	char uri[72];
	sprintf(uri, "%s://%s", ((conf->mqtt_encrypt == 0) ? "mqtt" : "mqtts"), conf->mqtt_host);


	esp_mqtt_client_config_t mqtt_cfg = {
		.event_handle = mqtt_event_handler,
		.uri = uri,
		.port = conf->mqtt_port,
	//	.client_id = conf->mqtt_deviceid,
		.username = conf->mqtt_user,
		.password = conf->mqtt_pass,
		};

	if (strlen(conf->mqtt_deviceid) > 0)
	mqtt_cfg.client_id = conf->mqtt_deviceid;

	xTaskCreate( mqtt_publisher_task,  "mqtt_publisher_task", 3072, NULL, 4, &mqtt_publisher_handle);
	xTaskCreate(mqtt_subscriber_task, "mqtt_subscriber_task", 4096, NULL, 4, &mqtt_subscriber_handle);
	// xTaskCreatePinnedToCore( mqtt_publisher_task,  "mqtt_publisher_task", 2048, NULL, 4, &mqtt_publisher_handle,1);
	// xTaskCreatePinnedToCore(mqtt_subscriber_task, "mqtt_subscriber_task", 2048, NULL, 4, &mqtt_subscriber_handle,1);
	mutex = xSemaphoreCreateMutex();

	ESP_ERROR_CHECK(init_MQTT_topics());
	ESP_ERROR_CHECK(init_MQTT_timer());
	ESP_ERROR_CHECK(init_NTP_timer());
	
	mqtt_old_val_1 = -1000.0;
	mqtt_old_val_2 = -1000.0;
	ESP_LOGI(TAG,"Checking Signals");
	

	mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

	return esp_mqtt_client_start(mqtt_client);
}