#include "AIO_Board.h"
#include <esp_log.h>
#include <string.h>



uint16_t RTD_REF;
// ---------- Constants

#define PWM_OFFS		0.00720000
#define PWM_GAIN		0.98930000
#define GPIO_ATTN		0.52621507	// GPIO * (R1//R3) / (R2 + R1//R3) = GPIO * (2k2//1k) / (619 + 2k2//1k) = GPIO * GPIO_ATTN
#define VREF_ATTN		0.31250000	// VREF_EXT * 1.0 / (1.0 + 2.2) = VREF_EXT * VREF_ATTN
#define AI_V_ATTN		0.09140969	// Vi = ADC / 2^12 * VREF / (3.32 / (3.32 + 33)) = ADC / 2^12 * VREF / AI_V_ATTN
#define AI_I_ATTN		44.6865752	// Ii = ADC / 2^12 * VREF / (45.3//3300)         = ADC / 2^12 * VREF / AI_I_ATTN
#define AO_V_GAIN		3.15517241	// VOUT = VDAC * (1 + 10 / 4.64) = VDAC * AO_V_GAIN

// ---------- PWM

#define PWM_HS_TIMER	LEDC_TIMER_0
#define PWM_HS_MODE		LEDC_HIGH_SPEED_MODE
#define PWM_HS_CH0_GPIO	PIN_XTR_DAC
#define PWM_HS_CH0_CHAN	LEDC_CHANNEL_0


static const char *TAG = "AIO_Board";

static aio_dev_t AIO;

// ---------- PWM descriptors
static ledc_timer_config_t	 pwm_timer;
static ledc_channel_config_t pwm_channel;


float map_raw(float x, float _in_min, float _in_max, float _out_min, float _out_max)
{
	float m = (_out_max - _out_min) / (_in_max - _in_min);
	float y = (m * (x - _in_min) + _out_min);
	return y;
}

float map(float x, aio_conv_t _type)
{
	float raw = 0.000;
	switch (_type)
	{
	case AI_CODE_TO_VOLT:
		return (AIO.VREF_ADC * x / AIO.AI_code_max);		// V = VREF * CODE / 2^12, V = [0,VREF]
		break;

	case AI_VOLT_TO_PHYS:
		if ((AIO.AI == AIO_VOLTAGE_0_10) || (AIO.AI == AIO_VOLTAGE_2_10))
			return (x * (33 + 15.8) / 15.8);
			// return map_raw(x, 0, AIO.VREF_ADC, 0.0, AIO.VREF_ADC / AI_V_ATTN);
		if ((AIO.AI == AIO_CURRENT_0_20) || (AIO.AI == AIO_CURRENT_4_20))
			return (x / 156.4356*1000); //  divide the reading by the parallel 158 // 15800 that is 156.4356
			// return map_raw(x, 0, AIO.VREF_ADC, 0.0, AIO.VREF_ADC / AI_I_ATTN * 1000.0);
		break;

	case AI_PHYS_TO_UNIT:
		raw = map_raw(x, AIO.AI_phys_min, AIO.AI_phys_max, AIO.AI_unit_min, AIO.AI_unit_max);
		if (raw < AIO.AI_unit_min)
			return AIO.AI_unit_min;
		if (raw > AIO.AI_unit_max)
			return AIO.AI_unit_max;
		return raw;
		break;

	case AO_UNIT_TO_PHYS:
		return map_raw(x, AIO.AO_unit_min, AIO.AO_unit_max, AIO.AO_phys_min, AIO.AO_phys_max);
		break;

	case AO_PHYS_TO_VOLT:
		if ((AIO.AO == AIO_VOLTAGE_0_10) || (AIO.AO == AIO_CURRENT_0_20))
			return map_raw(x, AIO.AO_phys_min, AIO.AO_phys_max, 0.0 / AO_V_GAIN, 10.0 / AO_V_GAIN);
			// return map_raw(x, AIO.AO_phys_min, AIO.AO_phys_max, 0.0 / AO_V_GAIN, 10.0 / AO_V_GAIN);
		if ((AIO.AO == AIO_VOLTAGE_2_10) || (AIO.AO == AIO_CURRENT_4_20))
			return map_raw(x, AIO.AO_phys_min, AIO.AO_phys_max, 2.0 / AO_V_GAIN, 10.0 / AO_V_GAIN);
			// return map_raw(x, AIO.AO_phys_min, AIO.AO_phys_max, 2.0 / AO_V_GAIN, 10.0 / AO_V_GAIN);
		break;

	case AO_VOLT_TO_CODE:
		if ((AIO.AO == AIO_VOLTAGE_0_10) || (AIO.AO == AIO_VOLTAGE_2_10))
			raw = x/(1.0/512.0 *3.3*(1.0+10.0/4.64));		// V = DC/512 x 3.3 x ( 1 + 10 ÷ 4.64  = DC/0.0203360722
			// raw = x/0.020336072198276;
		if ((AIO.AO == AIO_CURRENT_0_20) || (AIO.AO == AIO_CURRENT_4_20))
			raw = x/0.040753651699952;
			// raw = x/(1/512*3.3*(1+10/4.64)/4.99*10)								//for current output is DC/512 x 3.3 x ( 1 + 10 ÷ 4,64 ) /499
		ESP_LOGD(TAG, "DC = %f", raw); 
		return raw;	
		// return (x - PWM_OFFS) * AIO.AO_code_max / AIO.VSUP / PWM_GAIN;		// V = CODE / 2^9 * VSUP * PWMG + PWMO, V = [0,VSUP]
		break;

	default:
		break;
	}

	return 0;
}

void set_raw_channel(uint8_t ch)
{
	pwm_channel.gpio_num = ch;
	ledc_channel_config(&pwm_channel);
}

void set_raw(uint16_t code)
{
	ESP_LOGD(TAG,"Set the value and OD to low, code=%d",code);
	gpio_set_level((gpio_num_t)PIN_XTR_OD, 0);
	ESP_ERROR_CHECK(ledc_set_duty(pwm_channel.speed_mode, pwm_channel.channel, code & 0x01FF));
	ESP_ERROR_CHECK(ledc_update_duty(pwm_channel.speed_mode, pwm_channel.channel));
}

void AIO_set(float _unit)
{	
	ESP_LOGD(TAG, "_unit = %f", _unit);
  
	if (AIO.AO == AIO_DISABLED){
		ESP_LOGD(TAG, "AIO_DISABLED");
		return;
	}
 
	// ESP_LOGD(TAG, "_unit = %f", _unit);
	if (_unit > AIO.AO_unit_max)
		_unit = AIO.AO_unit_max;
	if (_unit < AIO.AO_unit_min)
		_unit = AIO.AO_unit_min;
	ESP_LOGD(TAG, "_unit(fixed) = %f", _unit);
 
	float _phys = map(_unit, AO_UNIT_TO_PHYS);
	// float _volt = map(_phys, AO_PHYS_TO_VOLT);
	float _volt = _phys;
	float _code = map(_volt, AO_VOLT_TO_CODE);

	ESP_LOGD(TAG, "_phys = \t%f", _phys);
	ESP_LOGD(TAG, "_volt = \t%f", _volt);
	ESP_LOGD(TAG, "_code = \t%f", _code);


	 set_raw(_code);
	// set_raw(256);
}

float AIO_get(uint16_t n)
{
	ESP_LOGD(TAG, "AIO_get");
	if (AIO.AI == AIO_DISABLED){
		ESP_LOGD(TAG, "AIO_DISABLED");
		return 65000.000;
	}

	// float _code = get_raw(n);
	float _volt = get_voltage();
	// float _volt = map(_code, AI_CODE_TO_VOLT);
	float _phys = map(_volt, AI_VOLT_TO_PHYS);
	float _unit = map(_phys, AI_PHYS_TO_UNIT);

	return _unit;
}


// ------------------------------------------------------
//			Initialization
// ------------------------------------------------------

esp_err_t init_GPIO()
{
	ESP_LOGI(TAG, "GPIO init");
	//pinMode(PIN_XTR_EF,  INPUT);
	gpio_config_t io_conf;

	// Disable XTR
	ESP_LOGD(TAG,"Disable XTR");
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = (1ULL << PIN_XTR_OD);
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&io_conf));
	

	ESP_LOGD(TAG,"Pull OD high");
	gpio_set_level((gpio_num_t)PIN_XTR_OD, 1);
	
	return ESP_OK;
}


esp_err_t init_PWM()
{
	ESP_LOGI(TAG, "PWM init");
	pwm_timer.duty_resolution = LEDC_TIMER_9_BIT;	// resolution of PWM duty
	pwm_timer.freq_hz = 100000;						// frequency of PWM signal
	pwm_timer.speed_mode = PWM_HS_MODE;				// timer mode
	pwm_timer.timer_num = PWM_HS_TIMER;				// timer index
	ESP_ERROR_CHECK(ledc_timer_config(&pwm_timer));

	pwm_channel.channel = PWM_HS_CH0_CHAN;
	pwm_channel.duty = 0;
	pwm_channel.gpio_num = PWM_HS_CH0_GPIO;
	pwm_channel.speed_mode = PWM_HS_MODE;
	pwm_channel.hpoint = 0;
	pwm_channel.timer_sel = PWM_HS_TIMER;
	ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel));

	return ESP_OK;
}

esp_err_t init_RTD(aio_conf_t *aio_conf, datalink_t* input)
{
	input->num = 0;

	if (aio_conf->tmp_1_enable == 1)
	{
		input->num++;	
		ESP_LOGI(TAG, "RTD 1 enabled -> init");
		float _RTD_nominal;
		max31865_numwires_t _RTD_wires;

		input->in1.enable = aio_conf->tmp_1_enable;
		memcpy(input->in1.name, aio_conf->tmp_1_name, strlen(aio_conf->tmp_1_name));
		ESP_LOGD(TAG, "Name 1 = %s", input->in1.name );
		input->in1.min = aio_conf->tmp_1_min;
		input->in1.max = aio_conf->tmp_1_max;
		input->in1.log_mode = aio_conf->tmp_1_log_mode;
		input->in1.log_mode_val = aio_conf->tmp_1_log_mode_val;
		input->sleep = aio_conf->tmp_bat_mode;
		input->sleeptime = aio_conf->tmp_bat_mode_val;
		add_unit(input->in1.name, input->in1.unit);
		

		
		if (aio_conf->tmp_1_rtd == 0)
		{
			_RTD_nominal = 100.00;
			RTD_REF = 402;
		}
		else
		{
			_RTD_nominal = 1000.00;
			RTD_REF = 4000;
		}

		switch (aio_conf->tmp_1_wires)
		{
		case 0:
			_RTD_wires = MAX31865_2WIRE;
			break;
		case 1:
			_RTD_wires = MAX31865_3WIRE;
			break;
		case 2:
			_RTD_wires = MAX31865_4WIRE;
			break;
		default:
			_RTD_wires = MAX31865_2WIRE;
			break;
		}
		init_MAX31865_1(_RTD_nominal, RTD_REF, _RTD_wires);
		max31865_enable(1, true);
	}
	else
	{
		ESP_LOGD(TAG, "RTD 1 disabled");
	}
	
	if (aio_conf->tmp_2_enable == 1)
	{
		input->num++;	
		ESP_LOGI(TAG, "RTD 2 enabled -> init");
		float _RTD_nominal;
		max31865_numwires_t _RTD_wires;

		input->in2.enable = aio_conf->tmp_2_enable;
		memcpy(input->in2.name, aio_conf->tmp_2_name, strlen(aio_conf->tmp_2_name));
		ESP_LOGD(TAG, "Name 2 = %s", input->in2.name );
		input->in2.min = aio_conf->tmp_2_min;
		input->in2.max = aio_conf->tmp_2_max;
		input->in2.log_mode = aio_conf->tmp_2_log_mode;
		input->in2.log_mode_val = aio_conf->tmp_2_log_mode_val;
		add_unit(input->in2.name, input->in2.unit);

		if (aio_conf->tmp_2_rtd == 0)
			_RTD_nominal = 100.00;
		else
			_RTD_nominal = 1000.00;

		switch (aio_conf->tmp_2_wires)
		{
		case 0:
			_RTD_wires = MAX31865_2WIRE;
			break;
		case 1:
			_RTD_wires = MAX31865_3WIRE;
			break;
		case 2:
			_RTD_wires = MAX31865_4WIRE;
			break;
		default:
			_RTD_wires = MAX31865_2WIRE;
			break;
		}
		init_MAX31865_2(_RTD_nominal, RTD_REF, _RTD_wires);
		max31865_enable(2, true);
	}
	else
	{
		ESP_LOGD(TAG, "RTD 2 disabled");
	}
	ESP_LOGD(TAG, "Input.num = %d", input->num);
	return ESP_OK;
}

esp_err_t init_DEV(aio_conf_t* conf)
{
	ESP_LOGI(TAG, "DEV init");
	AIO.AI = (aio_mode_t)conf->in_mode;
	AIO.AO = (aio_mode_t)conf->out_mode;

	switch (AIO.AO)
	{
	case AIO_DISABLED:
		AIO.AO_phys_min = 0;
		AIO.AO_phys_max = 0;
		break;

	case AIO_VOLTAGE_0_10:
		AIO.AO_phys_min = 0;
		AIO.AO_phys_max = 10;
		break;

	case AIO_VOLTAGE_2_10:
		AIO.AO_phys_min = 2;
		AIO.AO_phys_max = 10;
		break;

	case AIO_CURRENT_0_20:
		AIO.AO_phys_min = 0;
		AIO.AO_phys_max = 20;
		break;

	case AIO_CURRENT_4_20:
		AIO.AO_phys_min = 4;
		AIO.AO_phys_max = 20;
		break;

	default:
		AIO.AO = AIO_DISABLED;
		AIO.AO_phys_min = 0;
		AIO.AO_phys_max = 0;
		break;
	}

	switch (AIO.AI)
	{
	case AIO_DISABLED:
		AIO.AI_phys_min = 0;
		AIO.AI_phys_max = 0;
		break;

	case AIO_VOLTAGE_0_10:
		AIO.AI_phys_min = 0;
		AIO.AI_phys_max = 10;
		break;

	case AIO_VOLTAGE_2_10:
		AIO.AI_phys_min = 2;
		AIO.AI_phys_max = 10;
		break;

	case AIO_CURRENT_0_20:
		AIO.AI_phys_min = 0;
		AIO.AI_phys_max = 20;
		break;

	case AIO_CURRENT_4_20:
		AIO.AI_phys_min = 4;
		AIO.AI_phys_max = 20;
		break;

	default:
		AIO.AI = AIO_DISABLED;
		AIO.AI_phys_min = 0;
		AIO.AI_phys_max = 0;
		break;
	}

	AIO.AI_unit_min = conf->in_min;
	AIO.AI_unit_max = conf->in_max;
	AIO.AO_unit_min = conf->out_min;
	AIO.AO_unit_max = conf->out_max;

	AIO.AI_code_max = 4096.0;
	AIO.AI_code_min = 0.0;
	AIO.AO_code_max = 512.0;
	AIO.AO_code_min = 0.0;

	AIO.VREF_EXT = 2.048;
	AIO.VREF_ADC = 1.1;
	AIO.OFFS_ADC = 0.0;
	AIO.OFFS_CAL = 0;
	AIO.VSUP = 3.3;
	AIO.VSUP1 = 3.3;
	// AIO.VSUP2 = 3.3;
	
	return ESP_OK;
}
void printmode(uint8_t AIO_mode, char* IO)
{
		switch (AIO_mode)
	{
	case AIO_DISABLED:
		ESP_LOGD(TAG,"Analog %s: disabled", IO);
		break;
	case AIO_VOLTAGE_0_10:
		ESP_LOGI(TAG, "Analog %s: AIO_VOLTAGE_0_10", IO);
		break;
	case AIO_VOLTAGE_2_10:
		ESP_LOGI(TAG, "Analog %s: AIO_VOLTAGE_2_10", IO);
		break;
	case AIO_CURRENT_0_20:
		ESP_LOGI(TAG, "Analog %s: AIO_CURRENT_0_20", IO);
		break;
	case AIO_CURRENT_4_20:
		ESP_LOGI(TAG, "Analog %s: AIO_CURRENT_4_20", IO);
		break;
	default:
		break;
	}
}


void restart_on_failure()
{
	if (rebootloop == true)
	{
		led_fast(3);
		uint8_t count = 5;
		for (int i = count; i > 0; i--) {
			ESP_LOGI(TAG, "Restarting ESP in %d s", i);
			vTaskDelay(500 / portTICK_PERIOD_MS);
	}

		esp_restart();
	}
	else {
		ESP_LOGE(TAG, "Not able to reboot. Button was pressed!");
	}

}

esp_err_t AO_Init(void)
{
	ESP_LOGI(TAG, "Set initial setpoint");
	 nvs_handle_t my_handle;
	 esp_err_t err;
	 uint32_t setpoint_buffer1 = 0;
	  uint32_t setpoint_buffer2 = 0;
	  float setpoint=0;
	  bool notfound=false;
	  bool error=false;

    err = nvs_open("sp_storage", NVS_READWRITE, &my_handle);
	 if (err != ESP_OK)
	 {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	    return err;
     } 
	else {
          printf("Done opening NVS handle! \n");
	    }

       err = nvs_get_u32(my_handle, "setpoint", &setpoint_buffer1);
        switch (err) 
		{
            case ESP_OK:
                  printf("initial setpoint 1 = %u\n", setpoint_buffer1);
                  break;
            case ESP_ERR_NVS_NOT_FOUND:
                   printf(" requested key 1 doesn’t exist \n");
				   notfound=true;
                    break;
            default :
                   printf("Error (%s) in reading setpoint 1 !\n", esp_err_to_name(err));
				    error=true;
				   break;
           }

		   if (notfound==false && error== false)
		   {
             err = nvs_get_u32(my_handle, "setpoint1", &setpoint_buffer2);
              switch (err) 
	 	      {
                 case ESP_OK:
                  printf("initial setpoint 2 = %u\n", setpoint_buffer2);
                  break;
                 case ESP_ERR_NVS_NOT_FOUND:
                   printf(" requested key 2 doesn’t exist \n");
				     notfound=true;
                    break;
                 default :
                   printf("Error (%s) in reading setpoint 2 !\n", esp_err_to_name(err));
				    error=true;
				   break;
               }
		   }
		    if (notfound==false && error== false)
		   {
		   	  setpoint= (float)setpoint_buffer1+((float)setpoint_buffer2/10);
              AIO_set(setpoint);
	        }
	   // Close
	   nvs_close(my_handle);
	    return ESP_OK;

}

esp_err_t setpoint_save(uint32_t setpoint1,uint32_t setpoint2)
{
	 nvs_handle_t my_handle;
	 esp_err_t err;
	 char* TAG= "NVS";

	 err = nvs_open("sp_storage", NVS_READWRITE, &my_handle);
	 if (err != ESP_OK)
	 {
        ESP_LOGD(TAG, " failed opening NVS Handle");
	    return err;
     } 
	else {
           ESP_LOGD(TAG, "NVS Handle oepned successfully");
	    }

     err = nvs_set_u32(my_handle, "setpoint", setpoint1);
	        switch (err)
			 {
            case ESP_OK:
                ESP_LOGD(TAG, "storing setpoint in NVS done");
                break;
            default :
               ESP_LOGD(TAG, " Error in storing setpoint in NVS ");
				 nvs_close(my_handle);
				return err;
            }
	// Commit
       ESP_LOGD(TAG, " Commiting updates in NVS ");

	    err = nvs_commit(my_handle);
	    if (err != ESP_OK)
		{
	    	ESP_LOGD(TAG, " Commiting updates in NVS failed ");
			 nvs_close(my_handle);
	        return err;
		}

		     err = nvs_set_u32(my_handle, "setpoint1", setpoint2);
	        switch (err)
			 {
            case ESP_OK:
                ESP_LOGD(TAG, "storing rest of setpoint in NVS done");
                break;
            default :
               ESP_LOGD(TAG, " Error in storing rest of setpoint in NVS ");
				 nvs_close(my_handle);
				return err;
            }

		err = nvs_commit(my_handle);
	    if (err != ESP_OK)
		{
	    	ESP_LOGD(TAG, " Commiting updates in NVS failed ");
			 nvs_close(my_handle);
	        return err;
		}

			   // Close
	   nvs_close(my_handle);
	    return ESP_OK;
}

esp_err_t init_AIO(aio_conf_t *aio_conf, datalink_t* input)
{
	ESP_LOGI(TAG, "Init AIO module");

	// init_led();
	init_GPIO();				//Init GPIOs for ADC and DAC
	init_MCP3021();
	init_PWM();
	init_DEV(aio_conf);

	input->num = 0;

	if (aio_conf->in_mode > 0)
	{
		input->num = 1;	
		ESP_LOGI(TAG, "Analog input: Enabled");
		input->in1.enable = 1;
		memcpy(input->in1.name, aio_conf->in_key, strlen(aio_conf->in_key));
		ESP_LOGD(TAG, "Input Name = \t%s", input->in1.name );
		// input->in1.min = aio_conf->tmp_1_min;
		// input->in1.max = aio_conf->tmp_1_max;
		input->in1.log_mode = aio_conf->in_log_mode;
		input->in1.log_mode_val = aio_conf->in_log_val;
		add_unit(input->in1.name, input->in1.unit);

	}
	printmode(aio_conf->out_mode, "input");

	if (aio_conf->out_mode > 0){
		ESP_LOGI(TAG, "Analog output: enabled");
		input->in2.enable = 1;
		memcpy(input->in2.name, aio_conf->out_key, strlen(aio_conf->out_key));
		ESP_LOGD(TAG, "Output Name = \t%s", input->in2.name );
		add_unit(input->in2.name, input->in2.unit);
		AO_Init();
	}
	printmode(aio_conf->out_mode, "output");

	return ESP_OK;
}

