// /*
//  * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
//  *
//  * SPDX-License-Identifier: Apache-2.0
//  */

// #include <stdio.h>
// #include "sdkconfig.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "esp_log.h"
// #include "esp_timer.h"
// #include "driver/pulse_cnt.h"
// #include "bdc_motor.h"
// #include "pid_ctrl.h"
// #include "driver/gpio.h"
// #include "driver/mcpwm.h"

// static const char *TAG = "example";

// // Enable this config,  we will print debug formated string, which in return can be captured and parsed by Serial-Studio
// #define SERIAL_STUDIO_DEBUG           CONFIG_SERIAL_STUDIO_DEBUG

// #define BDC_MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
// #define BDC_MCPWM_FREQ_HZ             10000    // 25KHz PWM
// #define BDC_MCPWM_DUTY_TICK_MAX       (BDC_MCPWM_TIMER_RESOLUTION_HZ / BDC_MCPWM_FREQ_HZ) // maximum value we can set for the duty cycle, in ticks

// #define BDC_MCPWM_STBY                3
// //motor driver 1
// #define BDC_MCPWM_MD1                 1
// #define BDC_MCPWM_MD1_1               5
// #define BDC_MCPWM_MD1_2               6

// //motor driver 2
// #define BDC_MCPWM_MD2                 2 
// #define BDC_MCPWM_MD2_1               7
// #define BDC_MCPWM_MD2_2               21

// void motor_init(){
//     gpio_config_t io_conf = {
//         .mode = GPIO_MODE_OUTPUT,
//         .pull_down_en = 0,
//         .pull_up_en = 0, 
//         .intr_type = GPIO_INTR_DISABLE,
//         .pin_bit_mask = (
//                 (1ULL<<BDC_MCPWM_MD1) |
//                 (1ULL<<BDC_MCPWM_MD1_1) | 
//                 (1ULL<<BDC_MCPWM_MD1_2) | 
//                 (1ULL<<BDC_MCPWM_MD2)  |
//                 (1ULL<<BDC_MCPWM_MD2_1) | 
//                 (1ULL<<BDC_MCPWM_MD2_2) 
//             ),
//     };
//     gpio_config(&io_conf);

//     gpio_set_level(BDC_MCPWM_STBY, 1);

//     mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, BDC_MCPWM_MD1);
//     mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, BDC_MCPWM_MD2);

//     mcpwm_config_t pwm_config = {
//         .frequency = BDC_MCPWM_FREQ_HZ,
//         .cmpr_a = 50,
//         .cmpr_b = 50,
//         .counter_mode = MCPWM_UP_COUNTER,
//         .duty_mode = MCPWM_DUTY_MODE_0,
//     };


//     mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
//     mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);
// }

// void set_left_motor_speed(int speed){
//     mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, speed);
// }

// void set_right_motor_speed(int speed){
//     mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_A, speed);
// }

// void move_forward(){
//     // Set Ain1 to high 
//     gpio_set_level(BDC_MCPWM_MD1_1, 0);
//     gpio_set_level(BDC_MCPWM_MD2_1, 0);
//     // Set Ain2 to low 
//     gpio_set_level(BDC_MCPWM_MD1_2, 1); 
//     gpio_set_level(BDC_MCPWM_MD2_2, 1);
// }


// void move_backward(){
//     // Set Ain1 to low 
//     gpio_set_level(BDC_MCPWM_MD1_1, 1);
//     gpio_set_level(BDC_MCPWM_MD2_1, 1);
//     // Set Ain2 to high 
//     gpio_set_level(BDC_MCPWM_MD1_2, 0); 
//     gpio_set_level(BDC_MCPWM_MD2_2, 0);
// }

// void app_main(void)
// {
//     motor_init(); 
//     set_left_motor_speed(100);
//     set_right_motor_speed(100); 
//     move_forward();
//     while(1);
//     // int speed = 0;
//     // while (1) {
//     //     vTaskDelay(100);
//     //     set_left_motor_speed(speed);
//     //     set_right_motor_speed(100 - speed);
//     //     speed = (speed + 10) % 110;
//     //     // also see the dashboard config file `serial-studio-dashboard.json` for more information
// #if SERIAL_STUDIO_DEBUG
//         printf("/*%d*/\r\n", motor_ctrl_ctx.report_pulses);
// #endif
//     // }
// }



#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#define STBY_GPIO   3
#define PWM_GPIO_R  1
#define AIN1_GPIO_R 5
#define AIN2_GPIO_R 6

#define PWM_GPIO_L  2 
#define AIN1_GPIO_L 7
#define AIN2_GPIO_L 21

void init_motor_right();
void init_motor_left();
void turnLeft(int speed);
void turnRight(int speed);
void move(int motor, int speed, int direction);
void stop();
void start();

typedef enum {
    LEFT = 0,
    RIGHT = 1
} Motor;

typedef enum {
    FORWARD = 0,
    BACKWARD = 1
} Direction;


void app_main(void)
{
    init_motor_right();
    init_motor_left();

    while(1) {
        move(RIGHT, 200, FORWARD); // motor 1, half speed, right

        /*vTaskDelay(1000 / portTICK_PERIOD_MS);
        stop();
        vTaskDelay(250 / portTICK_PERIOD_MS);*/
    }
}

void init_motor_right(){
    esp_rom_gpio_pad_select_gpio(STBY_GPIO);
    gpio_set_direction(STBY_GPIO, GPIO_MODE_OUTPUT);

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
    //esp_rom_gpio_pad_select_gpio(STBY_GPIO); 
    //gpio_set_direction(STBY_GPIO, GPIO_MODE_OUTPUT);

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
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&channel_config);
}

void move(int motor, int speed, int direction) {
    gpio_set_level(STBY_GPIO, 1); // disable standby

    int inPin1 = 0;
    int inPin2 = 1;

    if(direction == 1) {
        inPin1 = 1;
        inPin2 = 0;
    } else {
        inPin1 = 0;
        inPin2 = 1;
    }
    
    //right on motor == 1 and channel is 0
    if(motor == 1) {
        gpio_set_level(AIN1_GPIO_R, inPin1);
        gpio_set_level(AIN2_GPIO_R, inPin2);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, speed);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    } else { //left on motor != 1 and channel is 1
        gpio_set_level(AIN1_GPIO_L, inPin1);
        gpio_set_level(AIN2_GPIO_L, inPin2);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, speed);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    }
}

void stop() {
    gpio_set_level(STBY_GPIO, 0); // enable standby
}

void start() {
    gpio_set_level(STBY_GPIO, 1); // disably standby
}

void turnLeft(int speed){
    //can tweak speed values and directions
    move(RIGHT, speed, FORWARD);
    move(LEFT, speed, BACKWARD);
}

void turnRight(int speed){
    //can tweak speed values and directions
    move(LEFT, speed, FORWARD);
    move(RIGHT, speed, BACKWARD);
}