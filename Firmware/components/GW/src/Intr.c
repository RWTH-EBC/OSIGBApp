#include "Intr.h"
#include <esp_log.h>
#include "ota.h"



#ifdef CONFIG_IOT_INTERFACE_TMP
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#endif


static const char *TAG = "AIO_Intr";
// extern bool reboot = true;

static bool intr_status;
static uint32_t intr_time_ms;
static QueueHandle_t gpio_evt_queue = NULL;

/*	Asynchronous GPIO ISR
*/
static void IRAM_ATTR intr_isr_handler(void* arg)
{
	uint32_t gpio_num = (uint32_t)arg;
	xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

/*	Synchronous GPIO ISR
	Finite State Machine
 */
static void intr_task_handler(void* arg)
{
	uint32_t io_num;
	while(1)
	{
		if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
			printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level((gpio_num_t)io_num));
			//Disable reboot counter
			ESP_LOGI(TAG, "Disable Reboot Counter");
			rebootloop = false;

			if (intr_status) {
				intr_time_ms = (uint32_t)(clock() * 1000 / CLOCKS_PER_SEC) - intr_time_ms;

				if (intr_time_ms < 1000){
					ESP_LOGI(TAG, "Config Key pressed for 1 sec");
					eddy_stone(true);
					
				}
				
				if ((intr_time_ms > 1000) && (intr_time_ms < 3000)) {
					
					wifi_config_t cfg;
					if (check_wifi_mode() == ESP_OK)
					{
						esp_wifi_get_config(ESP_IF_WIFI_AP, &cfg);
						if (cfg.ap.ssid_hidden == 1) {
							cfg.ap.ssid_hidden = 0;
							vTaskDelay(1000/portTICK_PERIOD_MS);
							ESP_LOGI(TAG, "AP mode credentials: SSID:[%s] password:[%s]", cfg.ap.ssid, cfg.ap.password);
							ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &cfg));
							ESP_ERROR_CHECK(esp_wifi_start());
							led_fast(3);
							vTaskDelay(1000/portTICK_PERIOD_MS);
							led_fast(3);
						}
					}
				
				}
				if ((intr_time_ms > 3000) && (intr_time_ms < 7000)) {
					// disconnect_mqtt();
					xSemaphoreGiveFromISR(ota_semaphore, pdFALSE);
					}

				if (intr_time_ms > 7000) {
					write_aio_conf((aio_conf_t*)&aio_conf_default);
					vTaskDelay(500 / portTICK_PERIOD_MS);
					esp_restart();
				}
			}
			else {
				intr_time_ms = (uint32_t)(clock() * 1000/ CLOCKS_PER_SEC);
			}

			intr_status = !intr_status;
		}
	}
}

esp_err_t init_INTR()
{
	ESP_LOGI(TAG, "Init GPIO interrupt module");

	intr_status = false;
	intr_time_ms = 0;
#ifdef CONFIG_IOT_INTERFACE_TMP
	ESP_LOGD(TAG,"Deinit Config PIN");
	ESP_ERROR_CHECK(rtc_gpio_deinit(PIN_RESET));
#endif
	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << PIN_RESET),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_ANYEDGE
	};
	ESP_ERROR_CHECK(gpio_config(&io_conf));

	//create a queue to handle gpio event from isr
	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
	
	//start gpio task
	ESP_LOGI(TAG,"Start Interrupt task handler");
	xTaskCreate(intr_task_handler, "intr_task_handler", 2048, NULL, 10, NULL);
	// xTaskCreatePinnedToCore(intr_task_handler, "intr_task_handler", 2048, NULL, 10, NULL,1);

	//install gpio isr service
	ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT));
	
	//hook isr handler for specific gpio pin
	return gpio_isr_handler_add((gpio_num_t)PIN_RESET, intr_isr_handler, (void*)PIN_RESET);
}