/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/pulse_cnt.h"
#include "bdc_motor.h"
#include "pid_ctrl.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"

static const char *TAG = "example";

// Enable this config,  we will print debug formated string, which in return can be captured and parsed by Serial-Studio
#define SERIAL_STUDIO_DEBUG           CONFIG_SERIAL_STUDIO_DEBUG

#define BDC_MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
#define BDC_MCPWM_FREQ_HZ             25000    // 25KHz PWM
#define BDC_MCPWM_DUTY_TICK_MAX       (BDC_MCPWM_TIMER_RESOLUTION_HZ / BDC_MCPWM_FREQ_HZ) // maximum value we can set for the duty cycle, in ticks

#define BDC_MCPWM_STBY                3
//motor driver 1
#define BDC_MCPWM_MD1                 1
#define BDC_MCPWM_MD1_1               5
#define BDC_MCPWM_MD1_2               6

//motor driver 2
#define BDC_MCPWM_MD2                 2 
#define BDC_MCPWM_MD2_1               7
#define BDC_MCPWM_MD2_2               8

void motor_init(){
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0, 
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (
                (1ULL<<BDC_MCPWM_MD1) |
                (1ULL<<BDC_MCPWM_MD1_1) | 
                (1ULL<<BDC_MCPWM_MD1_2) | 
                (1ULL<<BDC_MCPWM_MD2)  |
                (1ULL<<BDC_MCPWM_MD2_1) | 
                (1ULL<<BDC_MCPWM_MD2_2) 
            ),
    };
    gpio_config(&io_conf);

    gpio_set_level(BDC_MCPWM_STBY, 0);

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, BDC_MCPWM_MD1);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, BDC_MCPWM_MD2);

    mcpwm_config_t pwm_config = {
        .frequency = BDC_MCPWM_FREQ_HZ,
        .cmpr_a = 50,
        .cmpr_b = 50,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };


    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);
}

void set_left_motor_speed(int speed){
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, speed);
}

void set_right_motor_speed(int speed){
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_A, speed);
}

void move_forward(){
    // Set Ain1 to high 
    gpio_set_level(BDC_MCPWM_MD1_1, 0);
    gpio_set_level(BDC_MCPWM_MD2_1, 0);
    // Set Ain2 to low 
    gpio_set_level(BDC_MCPWM_MD1_2, 1); 
    gpio_set_level(BDC_MCPWM_MD2_2, 1);
}


void move_backward(){
    // Set Ain1 to low 
    gpio_set_level(BDC_MCPWM_MD1_1, 1);
    gpio_set_level(BDC_MCPWM_MD2_1, 1);
    // Set Ain2 to high 
    gpio_set_level(BDC_MCPWM_MD1_2, 0); 
    gpio_set_level(BDC_MCPWM_MD2_2, 0);
}

void app_main(void)
{
    motor_init(); 
    // set_left_motor_speed(25);
    // set_right_motor_speed(75); 
    move_forward();
    int speed = 0;
    while (1) {
        vTaskDelay(100);
        set_left_motor_speed(speed);
        set_right_motor_speed(100 - speed);
        speed = (speed + 10) % 110;
        // also see the dashboard config file `serial-studio-dashboard.json` for more information
#if SERIAL_STUDIO_DEBUG
        printf("/*%d*/\r\n", motor_ctrl_ctx.report_pulses);
#endif
    }
}
