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
#include "driver/ledc.h"
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "rom/ets_sys.h"
#include <math.h>

// Constants
#define GPIO_PIN GPIO_NUM_5
#define DATA_PIN    12
#define CLOCK_PIN   14
#define LATCH_PIN   4
#define PIR_PIN   15
#define stepperDriverPin0 GPIO_NUM_10
#define stepperDriverPin1 GPIO_NUM_11
#define stepperDriverPin2 GPIO_NUM_9
#define stepperDriverPin3 GPIO_NUM_13
#define SSID "Secret2.0"
#define PASS "dogtime!"

#define STBY_GPIO   3
#define PWM_GPIO_R  1
#define AIN1_GPIO_R 5
#define AIN2_GPIO_R 6

#define PWM_GPIO_L  2 
#define AIN1_GPIO_L 7
#define AIN2_GPIO_L 21

typedef enum {
    LEFT = 0,
    RIGHT = 1
} Motor;

typedef enum {
    FORWARD = 0,
    BACKWARD = 1
} Direction;

// Globals
int _step = 0;
int totalSteps = 0;
bool dir = true;
int state = 0;                       // 0 is manual moving, 1 is scanning, 2 is recording path (not actually used), 3 is replaying path
int sendConnectionStatusCounter = 0; // resets at 6000
int motionDetected = 0;
int moveSpeed = 130; //minimum value from testing, up to 255
int turnSpeed = 250;
int turn90Duration = 1500; //divided by portTICK_PERIOD_MS 
int turn180Duration = 2500; //divided by portTICK_PERIOD_MS 
//***can set different turn speeds for 90 deg and 180 deg turns if necessary
long timeSinceOn = 0;
int lastTurned;
char lastDirection;
bool playBackModeEn = false; 
int timeSinceLastScan = 0;

#define bitSet(value, bit) ((value) |= (1UL << (bit)))

static const char *TAG = "websocket";
esp_websocket_client_handle_t client;

//---------------------LED--------------------------
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

// Initializes the GPIO pins needed for the shift register chain
void initialize_gpio_for_SR() {
    esp_rom_gpio_pad_select_gpio(DATA_PIN);
    gpio_set_direction(DATA_PIN, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(CLOCK_PIN);
    gpio_set_direction(CLOCK_PIN, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(LATCH_PIN);
    gpio_set_direction(LATCH_PIN, GPIO_MODE_OUTPUT);
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
        setLEDRange(8,24);
        printf("GPIO state set to LOW\n");
    }
}

//--------------------Battery Life----------------
void batteryLife() {
    timeSinceOn++;
    float batteryLeft = ((0.1 * 60 * 60) - timeSinceOn/1000.0) / (0.1 * 60 * 60);
    int numLedsOn = 24 * batteryLeft;

    printf("batteryLeft: %f, timeSinceOn: %ld, numLedsOn: %d\n", batteryLeft, timeSinceOn, numLedsOn);

    gpio_set_level(LATCH_PIN, 0);
    shiftOut(0);
    gpio_set_level(LATCH_PIN, 1);
    gpio_set_level(LATCH_PIN, 0);
    setLEDRange(0,numLedsOn);
}

//--------------------PIR------------------------
/*void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    int level = gpio_get_level(gpio_num);
    motionDetected = !motionDetected;
    ets_printf("GPIO[%ld] state: %d\n", gpio_num, level);
}*/

void initialize_gpio_for_PIR() {
    gpio_config_t io_conf;

    // Configure the GPIO pin as input
    //io_conf.intr_type = GPIO_INTR_ANYEDGE; // Interrupt on rising or falling edge
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIR_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);

    // Install ISR service
    /*gpio_install_isr_service(0);

    // Hook ISR handler for specific GPIO pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);*/
    //esp_rom_gpio_pad_select_gpio(PIR_PIN);
    //gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT);
}

// -------------------DC MOTOR-----------------------

void init_motor_right(){
    esp_rom_gpio_pad_select_gpio(STBY_GPIO);
    gpio_set_direction(STBY_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(STBY_GPIO, 0); //initialize with standby active

    esp_rom_gpio_pad_select_gpio(PWM_GPIO_R);
    gpio_set_direction(PWM_GPIO_R, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(AIN1_GPIO_R);
    gpio_set_direction(AIN1_GPIO_R, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(AIN2_GPIO_R);
    gpio_set_direction(AIN2_GPIO_R, GPIO_MODE_OUTPUT);

    // Configure LEDC timer
    ledc_timer_config_t timer_config = {
        .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
        .freq_hz = 1000,                      // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,    // timer mode
        .timer_num = LEDC_TIMER_0             // timer index
    };
    ledc_timer_config(&timer_config);

    // Configure LEDC channel for PWM
    ledc_channel_config_t channel_config = {
        .gpio_num = PWM_GPIO_R,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&channel_config);
}

void init_motor_left(){
    esp_rom_gpio_pad_select_gpio(STBY_GPIO);
    gpio_set_direction(STBY_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(STBY_GPIO, 0); //initialize with standby active

    esp_rom_gpio_pad_select_gpio(PWM_GPIO_L);
    gpio_set_direction(PWM_GPIO_L, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(AIN1_GPIO_L);
    gpio_set_direction(AIN1_GPIO_L, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(AIN2_GPIO_L);
    gpio_set_direction(AIN2_GPIO_L, GPIO_MODE_OUTPUT);

    // Configure LEDC timer
    ledc_timer_config_t timer_config = {
        .duty_resolution = LEDC_TIMER_8_BIT, // resolution of PWM duty
        .freq_hz = 1000,                      // frequency of PWM signal
        .speed_mode = LEDC_LOW_SPEED_MODE,    // timer mode
        .timer_num = LEDC_TIMER_0             // timer index
    };
    ledc_timer_config(&timer_config);

    // Configure LEDC channel for PWM
    ledc_channel_config_t channel_config = {
        .gpio_num = PWM_GPIO_L,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&channel_config);
}

void stopMotors() {
    gpio_set_level(STBY_GPIO, 0); // enable standby
}

void startMotors() {
    gpio_set_level(STBY_GPIO, 1); // disably standby
}

void moveMotor(int motor, int speed, int direction) {
    startMotors(); // disable standby

    int inPin1;
    int inPin2;

    if(direction == FORWARD) {
        inPin1 = 0;
        inPin2 = 1;
    } else {
        inPin1 = 1;
        inPin2 = 0;
    }
    
    //right on motor == 1 and channel is 0
    if(motor == LEFT) {
        gpio_set_level(AIN1_GPIO_L, inPin1);
        gpio_set_level(AIN2_GPIO_L, inPin2);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, speed);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    } else { //left on motor != 1 and channel is 1
        gpio_set_level(AIN1_GPIO_R, inPin2);
        gpio_set_level(AIN2_GPIO_R, inPin1);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, speed);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    }
}

void moveMotorsForward(){
    printf("moving motors forward\n");
    startMotors();
    moveMotor(RIGHT, moveSpeed, FORWARD);
    moveMotor(LEFT, moveSpeed, FORWARD); 
}

void moveMotorsLeft(){
    printf("turning left\n");
    startMotors();
    //can tweak speed values and directions
    moveMotor(LEFT, turnSpeed, FORWARD);
    moveMotor(RIGHT, turnSpeed, BACKWARD);

    //wait some period of time - longer than right/left turns
    vTaskDelay(turn90Duration / portTICK_PERIOD_MS);
    stopMotors();
}

void moveMotorsRight(){
    printf("turning right\n");
    startMotors();
    //can tweak speed values and directions
    moveMotor(LEFT, turnSpeed, BACKWARD);
    moveMotor(RIGHT, turnSpeed, FORWARD);

    //wait some period of time - longer than right/left turns
    vTaskDelay(turn90Duration / portTICK_PERIOD_MS);
    stopMotors();
}

void moveMotors180(){
    printf("turning 180\n");

    startMotors();
    //can tweak speed values and directions
    moveMotor(LEFT, turnSpeed, FORWARD);
    moveMotor(RIGHT, turnSpeed, BACKWARD);

    //wait some period of time - longer than right/left turns
    vTaskDelay(turn180Duration / portTICK_PERIOD_MS);
    stopMotors();
}

void checkToTurnOnMotorsFromWebSocket(esp_websocket_event_data_t *data) {
    const char *moveRight = "R";
    const char *moveLeft = "L";
    const char *moveUp = "U";
    const char *moveDown = "D";
    const char *stop = "S";
    const char *differentCommand = "[";
    
    if (data->data_len > 0) {
        char direction = data->data_ptr[0];

        if(direction != *differentCommand) {
            if (direction == *moveRight || direction == *moveLeft || direction == *moveUp || direction == *moveDown) {
                int number = 0;
                const char *numberStr = data->data_ptr + 1; 

                while (*numberStr && isdigit((unsigned char)*numberStr)) {
                    number = number * 10 + (*numberStr - '0');
                    numberStr++;
                }

                if(moveSpeed <= 230) {
                    moveSpeed = 120 + 5 * number;
                }

                printf("Motors begin moving %c%d (final motorSpeed: %d)\n", direction, number, moveSpeed);

                if(direction == *moveUp) {
                    moveMotorsForward();
                }else if(direction == *moveDown && (number != lastTurned || lastDirection != 'd')) {
                    moveMotors180();
                    lastTurned = number;
                    lastDirection = 'd';
                } else if(direction == *moveLeft && (number != lastTurned || lastDirection != 'l')) {
                    moveMotorsLeft();
                    lastTurned = number;
                    lastDirection = 'l';
                } else if(direction == *moveRight && (number != lastTurned || lastDirection != 'r')) {
                    moveMotorsRight();
                    lastTurned = number;
                    lastDirection = 'r';
                }

            } else if (direction == *stop) {
                printf("Stop motors\n");
                moveSpeed = 0;
                stopMotors();
            } else {
                printf("Invalid direction: %c\n", direction);
            }
        }
    }
}
//--------------------STEPPER------------------------
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
            if(totalSteps >= 1300) {
                if(totalSteps == 1300) {
                    gpio_set_level(stepperDriverPin0, 0);
                    gpio_set_level(stepperDriverPin1, 0);
                    gpio_set_level(stepperDriverPin2, 0);
                    gpio_set_level(stepperDriverPin3, 0);
                    timeSinceLastScan += 10;
                    vTaskDelay(100/portTICK_PERIOD_MS);
                }
                totalSteps++;
                if(gpio_get_level(PIR_PIN) == 1) {
                    printf("PIR sensed motion\n");
                    motionDetected = 1;
                }
            } else {
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
                    totalSteps++;
                } else {
                    _step--;
                    totalSteps--;
                }
            }

            if(totalSteps == 2700) {
                totalSteps = 0;
                timeSinceLastScan = 0;
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
        batteryLife();
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
}

//--------------------Playback----------------------
void checkToTurnOnPlayBackState(esp_websocket_event_data_t *data) {
    const char *isInPlayBack = "1";
    if (data->data_len > 0) {
        char playBackState = data->data_ptr[0];
        if(playBackState == *isInPlayBack) {
            playBackModeEn = true;
        } else {
            state = 0;
        }
    }
}

void checkToTurnOnScanState(esp_websocket_event_data_t *data) {
    const char *isScanTypeMsg = "s";
    char isInScan = '1';
    char isNotInScan = '0';
    if (data->data_len >= 1) {
        char playBackState = data->data_ptr[0];
        printf("here1\n");
        if(playBackState == *isScanTypeMsg) {
            const char *playBackState2 = data->data_ptr + 1; 
            printf("here2\n");
            if(*playBackState2 == isInScan) {
                printf("turning scan mode on\n");
                state = 1;
            } else if(*playBackState2 == isNotInScan) {
                printf("turning scan mode off\n");
                state = 0;
            }
        }
    }
}

//PIR Scan

// void activateScanPIR(esp_websockect_event_data_t *data){
//     if(data->data_len > 0){

//     }
// }

//--------------------WiFi-----------------------
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
    printf("------------------Wifi Started-----------------------\n");
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
    printf("------------------WIFI CONNECTED---------------------\n");
}

//--------------------------Websocket---------------------------------
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

            //checkToTurnOnLEDsFromWebSocket(data);
            //checkToTurnOnPlayBackState(data);
            checkToTurnOnScanState(data);
            if(state == 0) {
                checkToTurnOnMotorsFromWebSocket(data);
            }
            //added for scan button ------------
            //     playBackModeEn = false;
            // } else if(state == 1) {
            //     playBackModeEn = true;
            // }
            //end added for scan button ---------
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

    // send request to reset movementLog in apiData.txt
    const char *message = "c";
    esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    while (1) {
        // Send WebSocket Text Message
        message = "gL";
        esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
        // Wait for a while before sending the next message
        vTaskDelay(500 / portTICK_PERIOD_MS);

        if(state == 0) {
            message = "gD";
            esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        } else if(state == 1) {
            if(motionDetected) {
                char *movementMsg = (char *)malloc(4 * sizeof(char));
                snprintf(movementMsg, 10, "[d]%d", motionDetected);
                printf("Sending movement message: %s\n", movementMsg);
                esp_websocket_client_send_text(client, movementMsg, strlen(movementMsg), portMAX_DELAY);
                free(movementMsg);
            }
        }

        if(sendConnectionStatusCounter % 5 == 0) {
            message = "m";
            esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
            vTaskDelay(500 / portTICK_PERIOD_MS);

            // Check to see if it should be in playback mode or not
            /*char *stateMsg = (char *)malloc(5 * sizeof(char));
            snprintf(stateMsg, 5, "[cp]");
            esp_websocket_client_send_text(client, stateMsg, strlen(stateMsg), portMAX_DELAY);*/

            // Check to see if it should be in scan mode or not
            char *stateMsg2 = (char *)malloc(5 * sizeof(char));
            snprintf(stateMsg2, 5, "[gs]");
            esp_websocket_client_send_text(client, stateMsg2, strlen(stateMsg2), portMAX_DELAY);

            /*if(playBackModeEn) {
                printf("playBackMode is enabled\n");
                if(timeSinceLastScan > 5000) {
                    state = 1; // scanning mode
                    printf("state set to 1\n");
                    char *stateMsg = (char *)malloc(5 * sizeof(char));
                    snprintf(stateMsg, 5, "[s]%d", state);
                    esp_websocket_client_send_text(client, stateMsg, strlen(stateMsg), portMAX_DELAY);
                } else {
                    printf("state set to 3\n");
                    state = 3; // replaying path mode
                    char *stateMsg = (char *)malloc(5 * sizeof(char));
                    snprintf(stateMsg, 5, "[s]%d", state);
                    esp_websocket_client_send_text(client, stateMsg, strlen(stateMsg), portMAX_DELAY);
                }
            }*/

            if(sendConnectionStatusCounter == 6000) {
                sendConnectionStatusCounter = 0;
            }
        }
        sendConnectionStatusCounter++;
    }
}

void app_main(void)
{   
    setupStepperDriverPins();
    initialize_gpio_for_PIR();
    initialize_gpio_for_SR();
    init_motor_right();
    init_motor_left();

    wifi_connection();
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("------------------Creating websocket task-----------------\n");
    xTaskCreate(&ws_client_task, "ws_client_task", 8192, NULL, 5, NULL);
    printf("------------------Creating stepper task-----------------\n");
    xTaskCreate(&stepper, "stepper", 8192, NULL, 5, NULL);
}