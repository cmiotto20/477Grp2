#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "antenna_switch.h"

#include <stdio.h>
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_websocket_client.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"

#define GPIO_PIN GPIO_NUM_4


#define SSID "ChickenNuggies"
#define PASS "3172940072M@tt"

static const char *TAG = "WEBSOCKET";

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    nvs_flash_init();
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

esp_websocket_client_handle_t client;

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "WebSocket Connected");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "WebSocket Disconnected");
            break;
        case WEBSOCKET_EVENT_DATA:
            ESP_LOGI(TAG, "WebSocket Received Data");
            //ESP_LOGI(TAG, "Received data: %.*s", data->data_len, (char*)data->data_ptr);
            // Assuming the target string is "Hello, WebSocket!"
            const char *ledOn = "ledStatus: 1";
            const char *ledOff = "ledStatus: 0";

            gpio_config_t io_conf = {
                    .pin_bit_mask = (1ULL << GPIO_PIN),
                    .mode = GPIO_MODE_OUTPUT,
                    .intr_type = GPIO_INTR_DISABLE,
                    .pull_up_en = 0,
                    .pull_down_en = 0,
            };
            gpio_config(&io_conf);

            if (data->data_len == strlen(ledOn) && memcmp(data->data_ptr, ledOn, data->data_len) == 0) {
                gpio_set_level(GPIO_PIN, 1);
                printf("GPIO state set to HIGH\n");
            } else if(data->data_len == strlen(ledOff) && memcmp(data->data_ptr, ledOff, data->data_len) == 0) {
                gpio_set_level(GPIO_PIN, 0);
                printf("GPIO state set to LOW\n");
            }
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGI(TAG, "WebSocket Error");
            break;
        default:
            break;
    }
}

void websocket_send_text(const char *text) {
    if (client) {
        esp_websocket_client_send_text(client, text, strlen(text), portMAX_DELAY);
    }
}

void websocket_app_start(const char *uri) {
    esp_websocket_client_config_t ws_cfg = {
        .uri = uri,
    };

    client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, client);

    esp_websocket_client_start(client);
}

void app_main(void)
{
    wifi_connection();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    // Start the WebSocket client
    websocket_app_start("ws://174.129.215.96:3000");

    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // Example: Send a text message every second
        websocket_send_text("get light status");
    }    
}