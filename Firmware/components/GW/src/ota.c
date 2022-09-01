#include "ota.h"

// static datalink_t input;

static const char *TAG = "ota";

SemaphoreHandle_t ota_semaphore;
// #ifdef CONFIG_IOT_INTERFACE_TMP	
//   SemaphoreHandle_t sleep_semaphore;
// #endif
extern const uint8_t git_cer_start[]   asm("_binary_git_cer_start");

esp_err_t client_event_handler(esp_http_client_event_t *evt)
{
  return ESP_OK;
}

esp_err_t validate_image_header(esp_app_desc_t *incoming_ota_desc)
{
  const esp_partition_t *running_partition = esp_ota_get_running_partition();
  esp_app_desc_t running_partition_description;
  esp_ota_get_partition_description(running_partition, &running_partition_description);

  ESP_LOGI(TAG, "current version is %s\n", running_partition_description.version);
  ESP_LOGI(TAG, "new version is %s\n", incoming_ota_desc->version);
  printf("%s: %s,%s\n", incoming_ota_desc->version, incoming_ota_desc->date, incoming_ota_desc->time);
  // printf ("%s\n", input->ver);

  if (strcmp(running_partition_description.version, incoming_ota_desc->version) == 0)
  {
    ESP_LOGW(TAG, "NEW VERSION IS THE SAME AS CURRENT VERSION. ABORTING");
    return ESP_FAIL;
  }
  return ESP_OK;
}

void run_ota(void *params)
{
  ESP_ERROR_CHECK(nvs_flash_init());
  // ESP_ERROR_CHECK(esp_event_loop_create_default());
  while (true)
  {
    xSemaphoreTake(ota_semaphore, portMAX_DELAY);
    led_fast(3);
    ESP_LOGI(TAG, "Invoking OTA");

    esp_http_client_config_t clientConfig = {
      #ifdef CONFIG_IOT_INTERFACE_AIO
      .url = "", // ota data location
      #elif CONFIG_IOT_INTERFACE_DIO
      .url = "", // ota data location
      #elif CONFIG_IOT_INTERFACE_TMP
      .url = "", // ota data location
      #endif
      .event_handler = client_event_handler,
      .cert_pem = (char *)git_cer_start
    };


  esp_https_ota_config_t ota_config = {
        .http_config = &clientConfig};

    esp_https_ota_handle_t ota_handle = NULL;

    if (esp_https_ota_begin(&ota_config, &ota_handle) != ESP_OK)
    {
      ESP_LOGE(TAG, "esp_https_ota_begin failed");
      goto end;
    }

    esp_app_desc_t incoming_ota_desc;
    if (esp_https_ota_get_img_desc(ota_handle, &incoming_ota_desc) != ESP_OK)
    {
      ESP_LOGE(TAG, "esp_https_ota_get_img_desc failed");
      esp_https_ota_finish(ota_handle);
      goto end;
    }

    if (validate_image_header(&incoming_ota_desc) != ESP_OK)
    {
      ESP_LOGE(TAG, "validate_image_header failed");
      esp_https_ota_finish(ota_handle);
      goto end;
    }
    else
    {
      ESP_LOGI(TAG, "Starting Update");
        disconnect_mqtt();
        vTaskDelay(500/portTICK_PERIOD_MS);
    }

    while (true)
    {
      esp_err_t ota_result = esp_https_ota_perform(ota_handle);
      if (ota_result != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
        break;
    }

    if (esp_https_ota_finish(ota_handle) != ESP_OK)
    {
      ESP_LOGE(TAG, "esp_https_ota_finish failed");
      led_fast(3);
      printf("restarting in 5 seconds\n");
      vTaskDelay(pdMS_TO_TICKS(5000));
      esp_restart();
    }
    else
    {
      printf("restarting in 5 seconds\n");
      led_blink(5);
      vTaskDelay(pdMS_TO_TICKS(5000));
      esp_restart();
    }

    end:
      ESP_LOGE(TAG, "Failed to update firmware");
      // #ifdef CONFIG_IOT_INTERFACE_TMP	
      //   xSemaphoreGive(sleep_semaphore);
      // #endif
      led_fast(8);
      continue;
      // ESP_LOGI(TAG,"restarting in 5 seconds\n");
      // vTaskDelay(pdMS_TO_TICKS(5000));

  }
}


void on_button_pushed(void *params)
{
  xSemaphoreGiveFromISR(ota_semaphore, pdFALSE);
}

void ota(datalink_t* input)
{
  // char ver[128];
  //printf("HAY!!! THIS IS OTA\n");
  const esp_partition_t *running_partition = esp_ota_get_running_partition();
  esp_app_desc_t running_partition_description;
  esp_ota_get_partition_description(running_partition, &running_partition_description);
  printf("current firmware version is: %s\n", running_partition_description.version);
  sprintf(input->ver,"%s: %s,%s", running_partition_description.version, running_partition_description.date, running_partition_description.time);
  printf ("%s\n", input->ver);
  // memcpy(input->ver,ver,strlen(ver));
  // strcpy(input->ver,running_partition_description.version);
  // strcat(input->ver,)

  ota_semaphore = xSemaphoreCreateBinary();
  // #ifdef CONFIG_IOT_INTERFACE_TMP	
  //   sleep_semaphore = xSemaphoreCreateBinary();
  // #endif
  xTaskCreate(run_ota, "run_ota", 1024 * 8, NULL, 2, NULL);
}

