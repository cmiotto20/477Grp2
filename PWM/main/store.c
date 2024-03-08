#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#define STBY_GPIO   3
#define PWMA_GPIO   1
#define AIN1_GPIO   5
#define AIN2_GPIO   6

void move(int motor, int speed, int direction);
void stop();

void app_main(void)
{
    esp_rom_gpio_pad_select_gpio(STBY_GPIO);
    gpio_set_direction(STBY_GPIO, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(PWMA_GPIO);
    gpio_set_direction(PWMA_GPIO, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(AIN1_GPIO);
    gpio_set_direction(AIN1_GPIO, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(AIN2_GPIO);
    gpio_set_direction(AIN2_GPIO, GPIO_MODE_OUTPUT);

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
        .gpio_num = PWMA_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };
    ledc_channel_config(&channel_config);

    while(1) {
        move(1, 200, 0); // motor 1, half speed, right

        /*vTaskDelay(1000 / portTICK_PERIOD_MS);
        stop();
        vTaskDelay(250 / portTICK_PERIOD_MS);*/
    }
}

void move(int motor, int speed, int direction) {
    gpio_set_level(STBY_GPIO, 1); // disable standby

    int inPin1 = 0;
    int inPin2 = 1;

    if(direction == 1) {
        inPin1 = 1;
        inPin2 = 0;
    }

    if(motor == 1) {
        gpio_set_level(AIN1_GPIO, inPin1);
        gpio_set_level(AIN2_GPIO, inPin2);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, speed);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
}

void stop() {
    gpio_set_level(STBY_GPIO, 0); // enable standby
}