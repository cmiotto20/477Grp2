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
#include "esp_http_client.h"
#include "esp_websocket_client.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include <ctype.h>
#include <stdio.h>

// Constants
#define GPIO_PIN GPIO_NUM_5
#define DATA_PIN    12
#define CLOCK_PIN   14
#define LATCH_PIN   4
#define stepperDriverPin0 GPIO_NUM_10
#define stepperDriverPin1 GPIO_NUM_11
#define stepperDriverPin2 GPIO_NUM_9
#define stepperDriverPin3 GPIO_NUM_13
#define SSID "Secret2.0"
#define PASS "dogtime!"

// Globals
int _step = 0;
bool dir = true;
int state = 0; // 0 is manual moving, 1 is scanning
int sendConnectionStatusCounter = 0; // resets at 6000

// Macros
#define bitSet(value, bit) ((value) |= (1UL << (bit)))

static const char *TAG = "websocket";
esp_websocket_client_handle_t client;

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

void wifi_connection() {
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
            .password = PASS,
        }};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

// Shift out binary bits to shift registers to control LEDs
void shiftOut(uint8_t bits) {
    for (uint8_t i = 0; i < 8; i++)  {
        gpio_set_level(DATA_PIN, (bits & (1 << (7 - i))) ? 1 : 0);
        gpio_set_level(CLOCK_PIN, 1);
        gpio_set_level(CLOCK_PIN, 0);
    }
    gpio_set_level(LATCH_PIN, 1);
    gpio_set_level(LATCH_PIN, 0);
}

// Initializes the GPIO pins needed for the shift register chain
void initialize_gpio_for_SR() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<DATA_PIN) | (1ULL<<CLOCK_PIN) | (1ULL<<LATCH_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
}

// Given an upper and lower bound, turn on LEDs within the 24 LED array
void setLEDRange(int left, int right) {
    int ledBits = 0;
    for(int i = left; i < right; i++) {
        bitSet(ledBits, i);
        shiftOut(ledBits>>16);
        shiftOut(ledBits>>8);
        shiftOut(ledBits);
        gpio_set_level(LATCH_PIN, 1);
    }
}

void checkToTurnOnLEDsFromWebSocket(esp_websocket_event_data_t *data) {
    const char *ledOn = "[ledStatus]: 1";
    const char *ledOff = "[ledStatus]: 0";
    if (data->data_len == strlen(ledOn) && memcmp(data->data_ptr, ledOn, data->data_len) == 0) {
        gpio_set_level(LATCH_PIN, 0);
        shiftOut(0);
        gpio_set_level(LATCH_PIN, 1);
        gpio_set_level(LATCH_PIN, 0);
        setLEDRange(0,8);
        printf("GPIO state set to HIGH\n");
    } else if(data->data_len == strlen(ledOff) && memcmp(data->data_ptr, ledOff, data->data_len) == 0) {
        int ledBits = 0;
        gpio_set_level(LATCH_PIN, 0);
        shiftOut(ledBits);
        gpio_set_level(LATCH_PIN, 1);
        gpio_set_level(LATCH_PIN, 0);
        setLEDRange(8,16);
        printf("GPIO state set to LOW\n");
    }
}

void checkToTurnOnMotorsFromWebSocket(esp_websocket_event_data_t *data) {
    const char *moveRight = "R";
    const char *moveLeft = "L";
    const char *moveUp = "U";
    const char *moveDown = "D";
    
    if (data->data_len > 0) {
        char direction = data->data_ptr[0];

        if (direction == *moveRight || direction == *moveLeft || direction == *moveUp || direction == *moveDown) {
            int number = 0;
            const char *numberStr = data->data_ptr + 1; 

            while (*numberStr && isdigit((unsigned char)*numberStr)) {
                number = number * 10 + (*numberStr - '0');
                numberStr++;
            }

            printf("Motors begin moving %c%d\n", direction, number);
        } else {
            printf("Invalid direction: %c\n", direction);
        }
    }
}

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
            //ESP_LOGI(TAG, "Received data: %.*s", data->data_len, (char*)data->data_ptr);

            checkToTurnOnLEDsFromWebSocket(data);
            if(state == 0) {
                checkToTurnOnMotorsFromWebSocket(data);
            }
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
            .uri = "ws://174.129.215.96:3000", 
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
        const char *message = "gL";
        esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
        // Wait for a while before sending the next message
        vTaskDelay(500 / portTICK_PERIOD_MS);

        if(state == 0) {
            message = "gD";
            esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else if(state == 1) {
            char *movementMsg = (char *)malloc(4 * sizeof(char));
            bool movementDetectionStatus = 0; // temp until we get PIR sensor
            snprintf(movementMsg, 10, "[d]%u", movementDetectionStatus);
            printf("Sending movement message: %s\n", movementMsg);
            esp_websocket_client_send_text(client, movementMsg, strlen(movementMsg), portMAX_DELAY);
            free(movementMsg);
        }

        if(sendConnectionStatusCounter % 5 == 0) {
            message = "m";
            esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
            vTaskDelay(500 / portTICK_PERIOD_MS);

            if(sendConnectionStatusCounter % 60 == 0 && sendConnectionStatusCounter != 0) {
                state = 1; // scanning mode
                printf("state set to 1\n");
            } else if(sendConnectionStatusCounter % 80 == 0) {
                printf("state set to 0\n");
                state = 0; // manual movement mode
            }

            if(sendConnectionStatusCounter == 6000) {
                sendConnectionStatusCounter = 0;
            }
        }
        sendConnectionStatusCounter++;
    }
}

void setupStepperDriverPins() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<stepperDriverPin0) | (1ULL<<stepperDriverPin1) | (1ULL<<stepperDriverPin2) | (1ULL<<stepperDriverPin3),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
}

void stepper(void *pvParameters) {
    while (true) {
        if(state == 1) {
            switch (_step) {
                case 0:
                    gpio_set_level(stepperDriverPin0, 0);
                    gpio_set_level(stepperDriverPin1, 0);
                    gpio_set_level(stepperDriverPin2, 0);
                    gpio_set_level(stepperDriverPin3, 1);
                    break;
                case 1:
                    gpio_set_level(stepperDriverPin0, 0);
                    gpio_set_level(stepperDriverPin1, 0);
                    gpio_set_level(stepperDriverPin2, 1);
                    gpio_set_level(stepperDriverPin3, 1);
                    break;
                case 2:
                    gpio_set_level(stepperDriverPin0, 0);
                    gpio_set_level(stepperDriverPin1, 0);
                    gpio_set_level(stepperDriverPin2, 1);
                    gpio_set_level(stepperDriverPin3, 0);
                    break;
                case 3:
                    gpio_set_level(stepperDriverPin0, 0);
                    gpio_set_level(stepperDriverPin1, 1);
                    gpio_set_level(stepperDriverPin2, 1);
                    gpio_set_level(stepperDriverPin3, 0);
                    break;
                case 4:
                    gpio_set_level(stepperDriverPin0, 0);
                    gpio_set_level(stepperDriverPin1, 1);
                    gpio_set_level(stepperDriverPin2, 0);
                    gpio_set_level(stepperDriverPin3, 0);
                    break;
                case 5:
                    gpio_set_level(stepperDriverPin0, 1);
                    gpio_set_level(stepperDriverPin1, 1);
                    gpio_set_level(stepperDriverPin2, 0);
                    gpio_set_level(stepperDriverPin3, 0);
                    break;
                case 6:
                    gpio_set_level(stepperDriverPin0, 1);
                    gpio_set_level(stepperDriverPin1, 0);
                    gpio_set_level(stepperDriverPin2, 0);
                    gpio_set_level(stepperDriverPin3, 0);
                    break;
                case 7:
                    gpio_set_level(stepperDriverPin0, 1);
                    gpio_set_level(stepperDriverPin1, 0);
                    gpio_set_level(stepperDriverPin2, 0);
                    gpio_set_level(stepperDriverPin3, 1);
                    break;
                default:
                    gpio_set_level(stepperDriverPin0, 0);
                    gpio_set_level(stepperDriverPin1, 0);
                    gpio_set_level(stepperDriverPin2, 0);
                    gpio_set_level(stepperDriverPin3, 0);
                    break;
            }

            if (dir) {
                _step++;
            } else {
                _step--;
            }

            if (_step > 7) {
                _step = 0;
            } else if (_step < 0) {
                _step = 7;
            }
        } else {
            gpio_set_level(stepperDriverPin0, 0);
            gpio_set_level(stepperDriverPin1, 0);
            gpio_set_level(stepperDriverPin2, 0);
            gpio_set_level(stepperDriverPin3, 0);
        }
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
}

void app_main(void)
{   
    wifi_connection();
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    initialize_gpio_for_SR();
    setupStepperDriverPins();

    xTaskCreate(&ws_client_task, "ws_client_task", 8192, NULL, 5, NULL);
    xTaskCreate(&stepper, "stepper", 8192, NULL, 5, NULL);
}