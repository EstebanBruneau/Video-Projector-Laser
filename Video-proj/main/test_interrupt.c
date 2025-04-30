#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define GPIO_OUTPUT_PIN    21 // Output pin connected to LED

static void configure_gpio(void)
{
    gpio_config_t io_conf = {};
    
    // Configure output pin (LED)
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_OUTPUT_PIN);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}

void app_main(void)
{
    configure_gpio();
    
    // Turn on LED
    gpio_set_level(GPIO_OUTPUT_PIN, 1);
}
