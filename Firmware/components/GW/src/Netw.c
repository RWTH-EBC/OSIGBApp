#include "Netw.h"
#include "esp_log.h"

#ifdef CONFIG_NETWORK_PPP
#include "esp_modem.h"
#include "esp_modem_netif.h"
#include "sqn3330.h"
#endif

#include "led.h"

#ifdef CONFIG_IOT_INTERFACE_TMP
#include "TMP_sleep.h"
#endif

#include "esp_http_server.h"
#include "BLE.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define NETW_CONNECT_BIT BIT0
#define NETW_FAIL_BIT      BIT1
static aio_conf_t *conf;
static datalink_t *input;

static EventGroupHandle_t netif_event_group;
static esp_netif_ip_info_t ip_info;
static httpd_handle_t server = NULL;
static wifi_config_t wifi_config;
static wifi_mode_t act_mode;

uint8_t counter = 0;


#ifdef CONFIG_NETWORK_PPP
static modem_dte_t * dte = NULL;
#endif

#ifdef CONFIG_NETWORK_PPP
static esp_netif_t * ppp_netif = NULL;
#endif

static esp_netif_t * sta_netif = NULL;
static esp_netif_t * ap_netif = NULL;

bool ap_set = false;

#ifdef CONFIG_NETWORK_PPP
static esp_modem_dte_config_t dte_config = {
        .port_num = CONFIG_UART_MODEM_PORT,
        .data_bits = UART_DATA_8_BITS,
        .stop_bits = UART_STOP_BITS_1,
        .parity = UART_PARITY_DISABLE,
        .baud_rate = CONFIG_UART_MODEM_BAUDRATE,
        .flow_control = MODEM_FLOW_CONTROL_HW
    };
	#endif
static wifi_sta_config_t sta = {
	.ssid = "",
	.password = ""
	};
static wifi_ap_config_t ap = {
	.ssid = "",
	.password = "",
	// .ssid_len = 0,
	.channel = 1,
	.authmode = WIFI_AUTH_WPA_WPA2_PSK,
	.ssid_hidden = 1,
	.max_connection = 5,
	// .beacon_interval = 100
	};

static const char *TAG = "AIO_Netw";

/* HTTP GET handler */
esp_err_t server_handler_get(httpd_req_t* req)
{
	ESP_LOGI(TAG, "HTTP GET handler");

	char* buf;
	size_t buf_len;

	/* Get header value string length and allocate memory for length + 1,
	 * extra byte for null termination */
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	ESP_LOGD(TAG, "Buffer Length = %d", buf_len);
	if (buf_len > 1) {
		buf = (char*)malloc(buf_len);
		/* Copy null terminated value string into buffer */
		if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
			ESP_LOGD(TAG, "Found header => Host: %s", buf);
		free(buf);
	}
 
	buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
	if (buf_len > 1) {
		buf = (char*)malloc(buf_len);
		if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK)
			ESP_LOGD(TAG, "Found header => Test-Header-2: %s", buf);
		free(buf);
	}

	buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
	if (buf_len > 1) {
		buf = (char*)malloc(buf_len);
		if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK)
			ESP_LOGD(TAG, "Found header => Test-Header-1: %s", buf);
		free(buf);
	}

	/* Read URL query string length and allocate memory for length + 1,
	 * extra byte for null termination */
	buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1) {
		buf = (char*)malloc(buf_len);
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
			ESP_LOGD(TAG, "Found URL query => %s", buf);

			char param[32];
			/* Get value of expected key from query string */
			if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK) {
				ESP_LOGD(TAG, "Found URL query parameter => query1=%s", buf);
			}
			if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK) {
				ESP_LOGD(TAG, "Found URL query parameter => query2=%s", buf);
			}
			if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK) {
				ESP_LOGD(TAG, "Found URL query parameter => query3=%s", buf);
			}
		}
		free(buf);
	}

	/* Set some custom headers */
	httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
	httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

	/* Send response with custom headers and body set as the
	 * string passed in user context*/
	ESP_LOGD(TAG,"Allocate Memory for HTML");
 	 
	char* HTML = (char*)malloc(16000);
	if (HTML != NULL)
	{
		ESP_LOGD(TAG,"Fill HTML file");
		export_aio_conf(conf, HTML, html_request);
		
		ESP_LOGD(TAG,"Send Response"); 
		httpd_resp_send(req, (const char*)HTML, strlen(HTML));
		free(HTML);
	} 

	/* After sending the HTTP response the old HTTP request
	 * headers are lost. Check if HTTP request headers can be read now. */
	if (httpd_req_get_hdr_value_len(req, "Host") == 0)
		ESP_LOGD(TAG, "Request headers lost");

	return ESP_OK;
}

/* HTTP POST handler */
esp_err_t server_handler_post(httpd_req_t* req)
{
	ESP_LOGI(TAG, "HTTP POST handler");

	char buf[3000];
	int ret = 0;
	int remaining = req->content_len;

	while (remaining > 0) {
		/* Read the data for the request */
		if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
			if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
				/* Retry receiving if timeout occurred */
				continue;
			}
			return ESP_FAIL;
		}
		remaining -= ret;
	}

	/* Log data received */
	ESP_LOGI(TAG, "Data received: %d bytes", ret);
	ESP_LOGD(TAG, "Buffer:\n%.*s\n", ret, buf);

	if (import_aio_conf(conf, buf) == ESP_OK) {
		ESP_LOGI(TAG, "Configuration data succefully imported");

		if (write_aio_conf(conf) == ESP_OK) {
			ESP_LOGI(TAG, "Configuration data succefully written");
			print_aio_conf(conf);
			/* Send back succesfull response */
			httpd_resp_send(req, html_response_pass, strlen(html_response_pass));
			vTaskDelay(500 / portTICK_PERIOD_MS);

			uint8_t count = 5;
			for (int i = count; i > 0; i--) {
				ESP_LOGI(TAG, "Restarting ESP in %d s", i);
				vTaskDelay(500 / portTICK_PERIOD_MS);
			}

			esp_restart();
		}
	}

	httpd_resp_send(req, html_response_fail, strlen(html_response_fail));
	return ESP_OK;
}

static httpd_uri_t uri_get  = { "/", HTTP_GET, server_handler_get,  NULL };
static httpd_uri_t uri_post = { "/",HTTP_POST, server_handler_post, NULL };

/*	Start HTTP server
 */
httpd_handle_t server_start(void)
{
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.stack_size = 2*4096;
	esp_netif_get_ip_info(ap_netif, &ip_info);
	ESP_LOGI(TAG, "Starting HTTP local server on " IPSTR ":%d", IP2STR(&ip_info.ip), config.server_port);
	ESP_LOGD(TAG, "Stack Size:%d" , config.stack_size);
	// Start the httpd server	
	if (httpd_start(&server, &config) == ESP_OK) {
		// Set URI handlers
		ESP_LOGI(TAG, "Registering URI handlers");
		httpd_register_uri_handler(server, &uri_get);
		httpd_register_uri_handler(server, &uri_post);
		return server;
	}

	ESP_LOGE(TAG, "Could not start server");
	return NULL;
}

/*	Stop HTTP server
 */
esp_err_t server_stop(void)
{
	ESP_LOGI(TAG, "Halting HTTP server");
	// Stop the httpd server
	return httpd_stop(server);
}

/*	WiFi event handler
	Finite State Machine
 */
void network_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	
	
	ESP_LOGI(TAG, "Netw EVENT");
	if (event_base == WIFI_EVENT) {
		switch (event_id) {
		case WIFI_EVENT_STA_START:
			ESP_LOGI(TAG, "WiFi Event:\tSTA start");
			esp_wifi_connect();
			break;

		case WIFI_EVENT_STA_CONNECTED:
			ESP_LOGI(TAG, "WiFi Event:\tSTA connected");
			esp_wifi_get_mode(&act_mode);
			if (ap.ssid_hidden == 0) {
				ESP_LOGI(TAG, "WiFi Event:\tDisabling AP interface");
				ap.ssid_hidden = 1;
				 wifi_config.ap = ap;
				// wifi_config.sta = sta;
				 ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));  //Failed with this config if AP is startet first. 
				// ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
				ESP_ERROR_CHECK(esp_wifi_start());
			}
			break;

		case WIFI_EVENT_STA_DISCONNECTED:
			ESP_LOGI(TAG, "WiFi Event:\tSTA disconnected");
			
			// esp_wifi_connect();
			xEventGroupClearBits(netif_event_group, NETW_CONNECT_BIT);
			wifi_event_sta_disconnected_t* wifi_event = (wifi_event_sta_disconnected_t*) event_data;
			ESP_LOGD(TAG, "Reason for disconnect from Wifi: %d", wifi_event->reason);
			
			if (wifi_event->reason == WIFI_REASON_NO_AP_FOUND ||wifi_event->reason == WIFI_REASON_AUTH_FAIL ||wifi_event->reason == WIFI_REASON_CONNECTION_FAIL
			|| wifi_event->reason == WIFI_REASON_ASSOC_EXPIRE) {
				if (counter == 5 && rebootloop){
					#ifdef CONFIG_IOT_INTERFACE_TMP
					led_fast(2);
					esp_wifi_disconnect();
					esp_wifi_stop();
					long_sleep();
					#endif
					ESP_LOGE(TAG, "Not able to Connect to Access Point");
					vTaskDelay(60*1000 / portTICK_PERIOD_MS);
					esp_wifi_connect();
					counter = 0; 
					}else if (!ap_set){
					#ifdef CONFIG_IOT_INTERFACE_TMP
					vTaskDelay(60*1000 / portTICK_PERIOD_MS);
					#endif
					printf("reboot counter : %d\n", counter++);
					esp_wifi_connect();
					}
					
			}
			break;

		case WIFI_EVENT_AP_START:
			ESP_LOGI(TAG, "WiFi Event:\tAP start");
			esp_wifi_connect();
			break;

		case WIFI_EVENT_AP_STACONNECTED:
			ESP_LOGI(TAG, "WiFi Event:\tAP connected STA");
			xEventGroupClearBits(netif_event_group, NETW_CONNECT_BIT);
			// #ifdef CONFIG_testmode
			// xTaskCreate(&blink, "blink", 2048, NULL, 2, NULL);
			// #endif
			break;

		case WIFI_EVENT_AP_STADISCONNECTED:
			ESP_LOGI(TAG, "WiFi Event:\tAP disconnected STA");
			xEventGroupClearBits(netif_event_group, NETW_CONNECT_BIT);
			break;

		default:
			break;
		}
	}
	else if(event_base == NETIF_PPP_STATUS) {
		switch (event_id) {
		case NETIF_PPP_ERRORUSER:{
			/* User interrupted event from esp-netif */
        	esp_netif_t *netif = event_data;
        	ESP_LOGI(TAG, "PPP Event:\tUser interrupted event from netif:%p", netif);
			}
			break;
		
		default:
			ESP_LOGI(TAG, "PPP Event:\tState changed event %d", event_id);
			break;
		}
	}
	else if(event_base == IP_EVENT) {
		switch (event_id) {
		case IP_EVENT_STA_GOT_IP:{
			esp_netif_dns_info_t dns_info;
			ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
			esp_netif_t *netif = event->esp_netif;

			ESP_LOGI(TAG, "IP Event:\tSTA Connect to Server ");
			ESP_LOGI(TAG, "IP          : " IPSTR, IP2STR(&event->ip_info.ip));
			ESP_LOGI(TAG, "Netmask     : " IPSTR, IP2STR(&event->ip_info.netmask));
			ESP_LOGI(TAG, "Gateway     : " IPSTR, IP2STR(&event->ip_info.ip));
			esp_netif_get_dns_info(netif, 0, &dns_info);
			ESP_LOGI(TAG, "Name Server1: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
			esp_netif_get_dns_info(netif, 1, &dns_info);
			ESP_LOGI(TAG, "Name Server2: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));

			sprintf(input->ip, IPSTR ,IP2STR(&event->ip_info.ip));
			getURI_Eddy(input->ip);
			// ESP_LOGD(TAG, "Aktuelle IP %s", input->ip);
			
			

			xEventGroupSetBits(netif_event_group, NETW_CONNECT_BIT);
			}
			break;
		
		case IP_EVENT_PPP_GOT_IP:{
			esp_netif_dns_info_t dns_info;
			ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
			esp_netif_t *netif = event->esp_netif;

			ESP_LOGI(TAG, "IP Event:\tModem Connect to PPP Server");
			ESP_LOGI(TAG, "IP          : " IPSTR, IP2STR(&event->ip_info.ip));
			ESP_LOGI(TAG, "Netmask     : " IPSTR, IP2STR(&event->ip_info.netmask));
			ESP_LOGI(TAG, "Gateway     : " IPSTR, IP2STR(&event->ip_info.ip));
			esp_netif_get_dns_info(netif, 0, &dns_info);
			ESP_LOGI(TAG, "Name Server1: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
			esp_netif_get_dns_info(netif, 1, &dns_info);
			ESP_LOGI(TAG, "Name Server2: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));

			xEventGroupSetBits(netif_event_group, NETW_CONNECT_BIT);
			}
			break;

		case IP_EVENT_PPP_LOST_IP:
			ESP_LOGI(TAG, "Modem Disconnect from PPP Server");
			break;

		default:
			break;
		}
	}
	
}

/*	Init the WiFi in
	STA mode using aio_conf
 */
esp_err_t init_STA()
{
	ESP_LOGI(TAG, "Init STA mode");

	memset(sta.ssid, 0x00, 32);
	memset(sta.password, 0x00, 64);

	if (strlen(conf->wifi_sta_ssid) > 1)
		memcpy(sta.ssid, conf->wifi_sta_ssid, strlen(conf->wifi_sta_ssid));
	else
		memcpy(sta.ssid, "*", 2);

	memcpy(sta.password, conf->wifi_sta_pass, strlen(conf->wifi_sta_pass));
	wifi_config.sta = sta;

	if (server == NULL)
		server = server_start();


	ESP_LOGI(TAG, "STA mode credentials: SSID:[%s] password:[%s]", sta.ssid, sta.password);

	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

	return esp_wifi_start();
	// return ESP_OK;
}

/*	Init the WiFi in
	AP mode using aio_conf
 */
esp_err_t init_AP()
{
	ESP_LOGI(TAG, "Init AP mode");
	ap_set = true; 

	memset(ap.ssid, 0x00, 32);
	memset(ap.password, 0x00, 64);

	if ( strlen(conf->wifi_ap_ssid) > 0)
		memcpy(ap.ssid, conf->wifi_ap_ssid, strlen(conf->wifi_ap_ssid));
	else {
		uint8_t chipid[6];
		char ssid_ap[32];
		esp_efuse_mac_get_default(chipid);
		sprintf(ssid_ap, "ESP_%02x%02x%02x%02x%02x%02x", chipid[0], chipid[1], chipid[2], chipid[3], chipid[4], chipid[5]);
		memcpy(ap.ssid, ssid_ap, strlen(ssid_ap));
	}

	if (strlen(conf->wifi_ap_pass) > 8)
		memcpy(ap.password, conf->wifi_ap_pass, strlen(conf->wifi_ap_pass));
	else
		memcpy(ap.password, "123456789", 10);

	wifi_config.ap = ap;
	if (server == NULL)
		server = server_start();
		
	ESP_LOGI(TAG, "AP mode credentials: SSID:[%s] password:[%s]", ap.ssid, ap.password);
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
		
	if(ap.ssid_hidden == 0){
		return esp_wifi_start();
	}else{
		ESP_LOGD(TAG, "AP Hidden. Try to connect to Station");
	 	return ESP_OK;
	}
}

#ifdef CONFIG_NETWORK_PPP
/*	Init PPP
 */
esp_err_t init_PPP()
{
	ESP_LOGI(TAG, "Init PPP mode");

    modem_dce_t *dce = sqn3330_init(dte);
    ESP_ERROR_CHECK(dce->set_flow_ctrl(dce, MODEM_FLOW_CONTROL_HW));

    ESP_LOGI(TAG, "Module: %s", dce->name);
    ESP_LOGI(TAG, "Operator: %s", dce->oper);
    ESP_LOGI(TAG, "IMEI: %s", dce->imei);
    ESP_LOGI(TAG, "IMSI: %s", dce->imsi);

    /* Get signal quality */
    uint32_t rssi = 0, ber = 0;
    ESP_ERROR_CHECK(dce->get_signal_quality(dce, &rssi, &ber));
    ESP_LOGI(TAG, "RSSI: -%d dBm, BER: %d", rssi, ber);

	return esp_modem_start_ppp(dte);
}
#endif

/* 	AP Connect and Disconnect
	*/
esp_err_t check_wifi_mode()
{
	esp_wifi_get_mode(&act_mode);
	switch (act_mode)
	{
	case WIFI_MODE_APSTA:
		if (ap.ssid_hidden == 1) {
			ap.ssid_hidden = 0;
			wifi_config.ap = ap;
			vTaskDelay(1000/portTICK_PERIOD_MS);
			ESP_LOGI(TAG, "AP mode credentials: SSID:[%s] password:[%s]", ap.ssid, ap.password);
			ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
			ESP_ERROR_CHECK(esp_wifi_start());
		}
		return ESP_OK;

	case WIFI_MODE_AP || WIFI_MODE_STA:
		esp_wifi_set_mode(WIFI_MODE_APSTA);
		ESP_ERROR_CHECK(init_AP());
		
		return ESP_OK;

	default:
		return ESP_FAIL;
	}
}

/*	Init the Network connection.
 */
esp_err_t init_NETW(aio_conf_t *aio_conf, datalink_t *input_in)
{
	ESP_LOGI(TAG, "Init Network Interface");
	conf = aio_conf;
	input = input_in;

	netif_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

#ifdef CONFIG_NETWORK_WIFI
    sta_netif = esp_netif_create_default_wifi_sta();
	ap_netif  = esp_netif_create_default_wifi_ap();
	assert(sta_netif);
	assert( ap_netif);
#elif CONFIG_NETWORK_PPP
	dte = esp_modem_dte_init(&dte_config);
	ppp_netif = esp_netif_create_default_ppp(dte);
	ap_netif  = esp_netif_create_default_wifi_ap();
	assert(ppp_netif);
	assert( ap_netif);
#endif

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &network_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &network_event_handler, NULL));
#ifdef CONFIG_NETWORK_PPP
	ESP_ERROR_CHECK(esp_event_handler_register(NETIF_PPP_STATUS, ESP_EVENT_ANY_ID, &network_event_handler, NULL));
#endif
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

#ifdef CONFIG_NETWORK_WIFI
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	// ESP_ERROR_CHECK(init_AP());
	ESP_ERROR_CHECK(init_STA());
#elif CONFIG_NETWORK_PPP
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(init_AP());
	ESP_ERROR_CHECK(init_PPP());
#endif

	xEventGroupWaitBits(netif_event_group, NETW_CONNECT_BIT | NETW_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

	input_in = input;
	return ESP_OK;
}
