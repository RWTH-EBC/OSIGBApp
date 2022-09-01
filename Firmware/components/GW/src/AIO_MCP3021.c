#include "AIO_MCP3021.h"
#include "esp_log.h"

static const char *TAG = "AIO_MCP3021";


//uint64_t start4 =0;

//uint64_t stop4 =0;

esp_err_t err_test =0;


uint8_t raw[2];


void init_MCP3021()
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_MCP_SDA,
        .scl_io_num = PIN_MCP_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000};
    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

}

float get_voltage(void)
{

    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create(); 

    int16_t data = 0x0000;
    i2c_master_start(cmd_handle);
    
 
    i2c_master_write_byte(cmd_handle, (MCP3021_ADDRESS << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd_handle, (uint8_t *)&raw, 2, I2C_MASTER_ACK);
    i2c_master_stop(cmd_handle);

     // start4=xTaskGetTickCount();
    err_test= i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 7/ portTICK_PERIOD_MS);

    //stop4=xTaskGetTickCount()-start4;
    //printf ("cmd begin time= %llu ",stop4 );
  //  printf ("err= %d ",err_test );

    i2c_cmd_link_delete(cmd_handle);


    data = (data | raw[0]) << 6; //  shift the MSB left 6 times
    raw[1] = raw[1] >> 2; // shift the LSB right two times 
    data = data | raw[1]; // put the LSB to data

    float _voltage = data*(3.3/1024);
    float involt = _voltage * (33 + 15.8) / 15.8;

    ESP_LOGD(TAG,"Actual _voltage = \t%f\n", _voltage);
    ESP_LOGD(TAG,"Input voltage = \t%f\n", involt);

    return _voltage;
}
