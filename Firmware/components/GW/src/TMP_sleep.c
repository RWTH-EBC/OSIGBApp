#include "TMP_sleep.h"
#include "esp_log.h"

RTC_DATA_ATTR float mqtt_old_val_1;
RTC_DATA_ATTR float mqtt_old_val_2;
static float mqtt_new_val_1;
static float mqtt_new_val_2;
RTC_DATA_ATTR uint32_t timesWokenUp = 0;

static bool gotosleep;

static const char *TAG = "TMP_sleep";
static aio_conf_t *conf;
static datalink_t *input;


void deep_sleep_tmp(aio_conf_t* aio_conf, datalink_t* input_in)
{
	
    ESP_LOGD(TAG, "Checking temperatures. Woken up %d times", timesWokenUp);
    conf = aio_conf;
	input = input_in;

    if (timesWokenUp == 0)
    {
        ESP_LOGD(TAG, "Set values for the first Time");
        mqtt_old_val_1 = -1000.0;
	    mqtt_old_val_2 = -1000.0;
    }
	timesWokenUp++;

	if ((aio_conf->mod_11_status == 1 && aio_conf->tmp_1_log_mode == MQTT_LOG_PERIODIC)|| 
		(aio_conf->mod_12_status == 1 && aio_conf->tmp_2_log_mode == MQTT_LOG_PERIODIC))
		return;

	static uint8_t err_1 = 0;
	static uint8_t err_2 = 0;

    gotosleep = true;

    // while (1)
	// {
		// static char payload_1[32];
        

        ESP_LOGD(TAG, "old Val1 = %.2f, old Val2 = %.2f", mqtt_old_val_1, mqtt_old_val_2);

		
		// Temperature Probe 1
		
		if (conf->tmp_1_enable)
		mqtt_new_val_1 = max31865_temperature_1();				
		
		if (mqtt_new_val_1 < input->in1.min || mqtt_new_val_1 > input->in1.max) 
		{
			ESP_LOGD(TAG, "Value 1 out of Range (Min = %.2f, Max = %.2f)", input->in1.min, input->in1.max);
			err_1 = 1;
		}

		
		// Temperature Probe 2
		
		if (conf->tmp_2_enable)
		mqtt_new_val_2 = max31865_temperature_2();					

		if (mqtt_new_val_2 < input->in2.min || mqtt_new_val_2 > input->in2.max) 
		{
			ESP_LOGD(TAG, "Value 2 out of Range (Min = %.2f, Max = %.2f)", input->in2.min, input->in2.max);
			err_2 = 1;
		}


	
		switch (input->num)
		{
		case 0:
			ESP_LOGD(TAG, "All input's Disabled");
			break;
		case 1:
			if (input->in1.enable == 1)
			{
				ESP_LOGD(TAG, "val_1=%.2f ", mqtt_new_val_1);
				if (err_1 == 1)
					break;

				// Check report mode
				if (input->in1.log_mode == MQTT_LOG_THRESHOLD)
				{
					ESP_LOGD(TAG, "Log Mode Threshold \n old val 1 = %.2f", mqtt_old_val_1);

					if (fabsf(mqtt_new_val_1 - mqtt_old_val_1) < input->in1.log_mode_val)
						break;
				}	
				mqtt_old_val_1 = mqtt_new_val_1;
				gotosleep = false;

			}
			else if (input->in2.enable == 1)
			{
				ESP_LOGD(TAG, "val_2=%.2f ", mqtt_new_val_2);
				if (err_2 == 1)
					break;

				// Check report mode
				if (input->in2.log_mode == MQTT_LOG_THRESHOLD)
				{
					ESP_LOGD(TAG, "Log Mode Threshold \n old val 2 = %.2f", mqtt_old_val_2);
					if (fabsf(mqtt_new_val_2 - mqtt_old_val_2) < input->in2.log_mode_val)
						break;
				}
				
				mqtt_old_val_2 = mqtt_new_val_2;
				gotosleep = false;
			}
			
			break;
		case 2:
			ESP_LOGD(TAG, "Values \n\t\t val_1 \t= %.2f \n\t\t val_2 \t= %.2f", mqtt_new_val_1, mqtt_new_val_2);
			if(err_1 == 1 && err_2 ==1)
				break;

			// Check report mode
			if (input->in1.log_mode == MQTT_LOG_THRESHOLD && input->in2.log_mode == MQTT_LOG_THRESHOLD)
				{
				ESP_LOGD(TAG, "Log Mode Threshold");
				if (fabsf(mqtt_new_val_1 - mqtt_old_val_1) < input->in1.log_mode_val && fabsf(mqtt_new_val_2 - mqtt_old_val_2) < input->in2.log_mode_val)
					break;
				}
			if(err_1 == 0 && err_2 == 0)
			{
                ESP_LOGD(TAG, "Setting New values to old ones");
			    mqtt_old_val_1 = mqtt_new_val_1;
			
			    mqtt_old_val_2 = mqtt_new_val_2;
                gotosleep = false; 

			}else if (err_1 == 1) {
				mqtt_old_val_2 = mqtt_new_val_2;
                gotosleep = false; 

			}else if(err_2 == 1){
				mqtt_old_val_1 = mqtt_new_val_1;
                gotosleep = false; 

			}
	
			break;
		default:
			break;

    } 
 	
    if (gotosleep  == true && timesWokenUp != 0)
    {
        start_sleep();
    }else{
        ESP_LOGD(TAG, "Starting the whole board and publishing the values");
    } 
    
}

void start_sleep()
{	
	gpio_set_level(max_en, 0);
    rtc_gpio_pullup_en(PIN_RESET);
    rtc_gpio_pulldown_dis(PIN_RESET);
    esp_sleep_enable_ext0_wakeup(PIN_RESET,0);

	// esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    
    esp_sleep_enable_timer_wakeup(conf->tmp_bat_mode_val);
    ESP_LOGD(TAG, "Going to sleep for %d sec", conf->tmp_bat_mode_val/1000000);
	uart_tx_wait_idle(CONFIG_ESP_CONSOLE_UART_NUM);
    esp_deep_sleep_start();
}

void long_sleep()
{
	gpio_set_level(max_en, 0);
    rtc_gpio_pullup_en(PIN_RESET);
    rtc_gpio_pulldown_dis(PIN_RESET);
    esp_sleep_enable_ext0_wakeup(PIN_RESET,0);
	// esp_sleep_disable_wakeup_source()
	
	// esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    
    esp_sleep_enable_timer_wakeup(3.6E+9);
    ESP_LOGI(TAG, "Not able to Connect to Ap. \nGoing to sleep for 1h.");
    esp_deep_sleep_start();
}
