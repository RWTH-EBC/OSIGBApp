#include "sntp.h"

static const char *TAG = "AIO_SNTP";
bool init_sntp = false;
SemaphoreHandle_t mutexsntp;
extern char timerbuf[37];
extern char ntp_address[128];
struct timeval tv_now;


static void obtain_time(void);

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

int64_t gettime_ns()
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if(xSemaphoreTake(mutexsntp, 1000/portTICK_PERIOD_MS))
    {
        if (timeinfo.tm_year < (2016 - 1900)) {
           ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
            obtain_time();
            // update 'now' variable with current time
            time(&now);
        }
        xSemaphoreGive(mutexsntp);
    }

    char strftime_buf[64];

    // Set timezone to West Europe (Berlin) and print local time 
    setenv("TZ", "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGD(TAG, "The current date/time in Germany is: %s", strftime_buf);

    gettimeofday(&tv_now, NULL);
    int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
   // printf ("Time in μs= %ld \n", tv_now.tv_sec );
    ESP_LOGD(TAG, "Time in μs= %lld", time_us);
    int64_t time_ns = time_us*1000;
    ESP_LOGD(TAG, "Time in ns= %lld", time_ns);

    return time_ns;
}

void gettime_meta(char *timevar)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if(xSemaphoreTake(mutexsntp, 1000/portTICK_PERIOD_MS))
    {
        if (timeinfo.tm_year < (2016 - 1900)) {
            ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
            obtain_time();
            // update 'now' variable with current time
            time(&now);
        }
        xSemaphoreGive(mutexsntp);
    }

    char strftime_buf[64];

    // Set timezone to West Europe (Berlin) and print local time 
    setenv("TZ", "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGD(TAG, "The current date/time in Germany is: %s", strftime_buf);
    memcpy(timevar, strftime_buf, strlen(strftime_buf));
}
void refresh_time()
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    if(xSemaphoreTake(mutexsntp, 1000/portTICK_PERIOD_MS))
    {
        obtain_time();
        // update 'now' variable with current time
        time(&now);
        xSemaphoreGive(mutexsntp);
    }

    char strftime_buf[64];

    // Set timezone to West Europe (Berlin) and print local time 
    setenv("TZ", "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGD(TAG, "The current date/time in Germany is: %s", strftime_buf);
    // sntp_restart();
}

static void obtain_time(void)
{
        ESP_LOGI(TAG, "Initializing SNTP");

    	if (!init_sntp)
        {
            sntp_setoperatingmode(SNTP_OPMODE_POLL);
            sntp_setservername(0, ntp_address);
            sntp_set_time_sync_notification_cb(time_sync_notification_cb);
            sntp_init();
            init_sntp = true;
        }
        else 
         sntp_restart();

        // wait for time to be set
        time_t now = 0;
        struct tm timeinfo = { 0 };
        int retry = 0;
        const int retry_count = 10;
        while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
            ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
        time(&now);
        localtime_r(&now, &timeinfo);
}

void init_time(void)
{
    mutexsntp = xSemaphoreCreateMutex();
}

