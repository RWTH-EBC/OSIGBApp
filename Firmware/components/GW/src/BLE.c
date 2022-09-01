  
#include "BLE.h"
#include "led.h"

// static const char *TAG = "BLE";

char ipchar[32] = "192.168.4.1:80"; 

void ble_app_on_sync(void)
{
    ble_addr_t addr;
    ble_hs_id_gen_rnd(1, &addr);
    ble_hs_id_set_rnd(addr.val);

   

    struct ble_hs_adv_fields fields = (struct ble_hs_adv_fields){0};
    ble_eddystone_set_adv_data_url(&fields,
                        BLE_EDDYSTONE_URL_SCHEME_HTTP,
                        ipchar,
                        strlen(ipchar),
                        BLE_EDDYSTONE_URL_SUFFIX_NONE,
                        -30
        );

    struct ble_gap_adv_params adv_params = (struct ble_gap_adv_params){0};
    ble_gap_adv_start(BLE_OWN_ADDR_RANDOM, NULL, 30000, &adv_params, NULL, NULL);
}

void host_task(void *param)
{
    nimble_port_run();
}

void getURI_Eddy(char *uri)
{
    memcpy(ipchar, uri, strlen(uri)+1);
    ipchar[strlen(uri)+1] = '\0';
}

void eddy_stone(bool led)
{
    nvs_flash_init();

    // ESP_LOGD(TAG, "URI = %s", ipchar);
    esp_nimble_hci_and_controller_init();
    nimble_port_init();
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
    if (led == true)
        led_blink(2);
}