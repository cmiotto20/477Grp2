#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Define the GPIO pin you want to control
#define GPIO_PIN GPIO_NUM_4

void app_main() {
    // Configure the GPIO pin as an output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = 0,
        .pull_down_en = 0,
    };
    gpio_config(&io_conf);

    while (1) {
        // Toggle the state of the GPIO pin every second
        gpio_set_level(GPIO_PIN, 1);
        printf("GPIO state set to HIGH\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        gpio_set_level(GPIO_PIN, 0);
        printf("GPIO state set to LOW\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}