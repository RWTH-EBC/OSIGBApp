#include "led.h"
#include "esp_log.h"

static const char *TAG = "led";

//#ifdef CONFIG_testmode
// #define BLINK_GPIO CONFIG_LED_GPIO


void init_led()
{
    ESP_LOGD(TAG, "init_led start");
    esp_rom_gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

}

/* The defined LED will blink 1s on 1s of
 */
void blink()
{
        while(1) {
        /* Blink off (output low) */
	ESP_LOGD(TAG, "Turning off the LED");
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        /* Blink on (output high) */
	ESP_LOGD(TAG, "Turning on the LED");
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
}

void led_blink(uint8_t times)
{
       ESP_LOGD(TAG, "Blinkin LED for %d times", times);
        for (size_t i = 0; i < times; i++)
        {
             gpio_set_level(BLINK_GPIO, 1);  
             vTaskDelay(300 / portTICK_PERIOD_MS);
             gpio_set_level(BLINK_GPIO, 0); 
             vTaskDelay(300 / portTICK_PERIOD_MS);
        }    
}

void led_fast(uint8_t times)
{
       ESP_LOGD(TAG, "Blinkin LED for %d times", times);
        for (size_t i = 0; i < times; i++)
        {
             gpio_set_level(BLINK_GPIO, 1);  
             vTaskDelay(200 / portTICK_PERIOD_MS);
             gpio_set_level(BLINK_GPIO, 0); 
             vTaskDelay(100 / portTICK_PERIOD_MS);
        }    
}
//#endif