#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_websocket_client.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"


#include "protocol_examples_common.h"


#define WIFI_SSID "ChickenNuggies"
#define WIFI_PASS "3172940072M@tt"

static const char *TAG = "websocket_example";
esp_websocket_client_handle_t client;

static void on_websocket_event(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
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
            ESP_LOGI(TAG, "Received data: %.*s", data->data_len, (char*)data->data_ptr);
            break;
        default:
            break;
    }
}

static void ws_client_task(void *pvParameters) {
    // Wait for connection
    while (!esp_wifi_connect()) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // WebSocket Configuration
    esp_websocket_client_config_t ws_cfg = {
        .uri = "ws://174.129.215.96:3000",  // Set your WebSocket server URI
    };

    // Create WebSocket Client
    client = esp_websocket_client_init(&ws_cfg);
    esp_websocket_register_events(client, ESP_EVENT_ANY_ID, on_websocket_event, client);

    // Connect to WebSocket Server
    esp_websocket_client_start(client);

    // Wait for the connection to be established
    while (esp_websocket_client_is_connected(client) != true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while (1) {
        // Send WebSocket Text Message
        const char *message = "get light status";
        esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);

        // Wait for a while before sending the next message
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // Clean up and disconnect
        //esp_websocket_client_stop(client);
    }
}

static void wifi_init_sta(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait for connection
    wifi_config_t conf = {};
    esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &conf);
    if (err == ESP_OK) {
        ESP_LOGI("WiFi", "Connected to %s", conf.sta.ssid);
    }
}

void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();

    // Create WebSocket client task
    xTaskCreate(&ws_client_task, "ws_client_task", 20000, NULL, 5, NULL);
}