#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_macros.h"

#define WIFI_SSID "ESP32_SERVER"
#define WIFI_PASS ""
#define MAX_STA_CONN 1

#ifndef MIN
	#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

static const char* TAG = "WiFi_AP_Server";
char content[1800];
char localCopy[1800];

int16_t value = 0;
int8_t numbers[400] = {0};
int i = 0;

char* saveptr;
char* token;


// HTTP POST handler
esp_err_t post_handler(httpd_req_t *req) {
    
	int total_received = 0;
	int remaining = req->content_len;
	
	
	while(remaining > 0)
	{
    	int received = httpd_req_recv(req, content + total_received, remaining);
	    if (received <= 0) return ESP_FAIL;
		
		total_received += received;
		remaining -= received;
    }
	
	//strncpy(localCopy, content, 1599);
	
	content[total_received] = '\0'; // Null-terminate
		
	strncpy(localCopy, content, 1799);
	
	
	
	//ESP_LOGI(TAG, "Received POST data TEXT: %s", localCopy);
	
	i = 0;
	
	
	
	token = strtok_r(localCopy, "|", &saveptr);
	
	localCopy[1799] = '\0';
	
	
	    
	while(token != NULL && i < 400)
	{
		numbers[i] = atoi(token);
		i++;
		
		token = strtok_r(NULL, "|", &saveptr);
		
	}
	
	
	
	//ESP_LOGI(TAG, "Received POST data NUMS: %d %d", numbers[0], numbers[1]);
	i = 0;
	for(i = 0; i < 400; i++)
	{
		ESP_LOGI(TAG, "%d. %d", i, numbers[i]);
	}
	
	

	
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// URI handler setup
httpd_uri_t uri_post = {
    .uri      = "/post",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

// Start HTTP server
httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_post);
    }

    return server;
}

// Initialize AP mode
void wifi_init_softap(void) 
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = 1,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_OPEN,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started: SSID:%s", WIFI_SSID);
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_softap();
    start_webserver();
}
