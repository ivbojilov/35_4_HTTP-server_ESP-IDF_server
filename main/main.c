#include <string.h>
#include "esp_log_timestamp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_macros.h"
#include "esp_timer.h"
#include "driver/i2s.h"




#define WIFI_SSID "ESP32_SERVER"
#define WIFI_PASS ""
#define MAX_STA_CONN 1

#ifndef MIN
	#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define CHAR_ARR_LEN 9000
#define INT_ARR_LEN 2000

#define MAX_SIZE 10
#define COLS CHAR_ARR_LEN


char i2s_string[CHAR_ARR_LEN] = {0};
char i2s_localCopy[CHAR_ARR_LEN] = {0};
int16_t i2s_numbers_setup[INT_ARR_LEN] = {0};
int16_t i2s_numbers[INT_ARR_LEN] = {0};
static int16_t bufferA[INT_ARR_LEN] = {0};
static int16_t bufferB[INT_ARR_LEN] = {0};
static int16_t silence[INT_ARR_LEN] = {0};
int16_t* activeBuffer = NULL;
int16_t* currentBuffer = NULL;
char* dequeued = NULL;

volatile int8_t bufferReady = 0;
volatile int8_t usingBufferA = 1;

int j = 0;

char* i2s_saveptr;
char* i2s_token;

// Queue stuff
char queue[MAX_SIZE][COLS] = {0};
char data[COLS] = {0};
int8_t front = -1;
int8_t rear = -1;

int8_t _index = 0;
int8_t first = 1;

int isFull()
{
	return (rear + 1) % MAX_SIZE == front;
}

int isEmpty()
{
	return front == -1;
}

char* dequeue()
{
	if(isEmpty())
	{
		//printf("Queue underflow\n");
		return NULL;
	}
	
	memcpy(data, queue[front], strlen(queue[front]));
	
	if(front == rear)
	{
		front = rear = -1;
	} else
	{
		front = (front + 1) % MAX_SIZE;
	}
	
	return data;
}






size_t BytesWritten;

static const i2s_port_t i2s_num = I2S_NUM_0;

static const i2s_config_t i2s_config = {
    .mode = I2S_MODE_MASTER | I2S_MODE_TX,
    .sample_rate = 8000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = -1
};

static const i2s_pin_config_t pin_config = {
    .bck_io_num = 27,
    .ws_io_num = 26,
    .data_out_num = 25,
    .data_in_num = I2S_PIN_NO_CHANGE
};













static const char* TAG = "WiFi_AP_Server";

TaskHandle_t Task1;

char content[CHAR_ARR_LEN] = {0};
char localCopy[CHAR_ARR_LEN] = {0};

int16_t value = 0;
int8_t numbers[INT_ARR_LEN] = {0};
int i = 0;

char* saveptr;
char* token;



void Task1code(void* parameter)
{
	while (1) {
		
		/*
		if(first)
		{
			vTaskDelay(pdMS_TO_TICKS(5000));
		}
		*/
		dequeued = dequeue();
		if(dequeued == NULL)
		{
			i2s_write(i2s_num, silence, INT_ARR_LEN*2, &BytesWritten, portMAX_DELAY);
			vTaskDelay(pdMS_TO_TICKS(1));
			continue;
		}
		
		strncpy(i2s_localCopy, dequeued, CHAR_ARR_LEN-1);	
		//strncpy(i2s_localCopy, i2s_string, CHAR_ARR_LEN-1);



		//ESP_LOGI(TAG, "Received POST data TEXT: %s", localCopy);

		j = 0;



		i2s_token = strtok_r(i2s_localCopy, "|", &i2s_saveptr);

		
		activeBuffer = usingBufferA ? bufferA : bufferB;

		    
		while(i2s_token != NULL && j < INT_ARR_LEN)
		{
			activeBuffer[j] = atoi(i2s_token);
			//i2s_numbers[j] = atoi(i2s_token);
			j++;
			
			i2s_token = strtok_r(NULL, "|", &i2s_saveptr);
		}
		
		usingBufferA = !usingBufferA;
		

		currentBuffer = usingBufferA ? bufferA : bufferB;
		
		if(currentBuffer != NULL)
		{			    
			i2s_write(i2s_num, currentBuffer, INT_ARR_LEN*2, &BytesWritten, portMAX_DELAY);
		} else {
			i2s_write(i2s_num, silence, INT_ARR_LEN*2, &BytesWritten, portMAX_DELAY);
		}
		//i2s_write(i2s_num, i2s_numbers, INT_ARR_LEN*2, &BytesWritten, portMAX_DELAY);
		
		vTaskDelay(pdMS_TO_TICKS(1));
	    
		//i2s_write(i2s_num, &Value16Bit, sizeof(Value16Bit), &BytesWritten, portMAX_DELAY);
	}
	
	
}











// HTTP POST handler
esp_err_t post_handler(httpd_req_t *req) {
	
	//ESP_LOGI(TAG, "Start of reception: %lld", esp_timer_get_time()/1000);
    
	int total_received = 0;
	int remaining = req->content_len;
	_index = 0;
	
	//ESP_LOGI(TAG, "Size = %d", remaining);
	
	while(remaining > 0)
	{
		if(_index == 0)
		{
			_index = 1;
			
			if(isFull())
			{
				//printf("Queue overflow\n");
				return ESP_FAIL;
			}
			
			if(front == -1)
			{
				front = 0;
			}
			
			rear = (rear + 1) % MAX_SIZE;
			
			//memset(queue[rear], 0, COLS);
			
		}
		
		int received = httpd_req_recv(req, queue[rear] + total_received, remaining);		
    	//int received = httpd_req_recv(req, content + total_received, remaining);
	    if (received <= 0) return ESP_FAIL;
		
		total_received += received;
		remaining -= received;
    }
	
	
	//ESP_LOGI(TAG, "Char: %c", content[3598]);
	//ESP_LOGI(TAG, "String copied to buffer: %lld", esp_timer_get_time()/1000);
	
	//strncpy(localCopy, content, 1599);
	
	//content[total_received] = '\0'; // Null-terminate
	
	
	/*	
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
	
	*/
	
	//ESP_LOGI(TAG, "Received POST data NUMS: %d %d", numbers[0], numbers[1]);
	/*
	i = 0;
	for(i = 0; i < 400; i++)
	{
		ESP_LOGI(TAG, "%d. %d", i, numbers[i]);
	}
	*/
	
	
	if(first) first = 0;
	
    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
	//ESP_LOGI(TAG, "Response sent: %lld", esp_timer_get_time()/1000);
	
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
	

	
	// Install and start I2S driver
	i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
	i2s_set_pin(i2s_num, &pin_config);	
	
	
	for(j = 0; j < INT_ARR_LEN; j++)
	{
		i2s_numbers_setup[j] = (j / 20) % 2 == 0 ? 127 : -128;
	}
	
	for(j = 0; j < INT_ARR_LEN; j++)
	{
		char temp[8];
		
		snprintf(temp, 8, "%d", i2s_numbers_setup[j]);
		
		strcat(i2s_string, temp);
		
		if(j < INT_ARR_LEN-1) strcat(i2s_string, "|");
	}
	
	
	ESP_LOGI(TAG, "i2s_string = %s", i2s_string);	
	
	
	
	xTaskCreatePinnedToCore(
	    Task1code,       // Task function
	    "Task1",         // Name
	    2048,            // Stack size in words (adjust if needed)
	    NULL,            // Parameter
	    1,               // Priority
	    &Task1,          // Handle
	    1                // Core 0 (PRO_CPU)
	);		
	
	
    wifi_init_softap();
    start_webserver();
}
