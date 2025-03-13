#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define GPIO_INPUT_PIN     4  // Input pin connected to LFG
#define GPIO_OUTPUT_PIN    21 // Output pin connected to LED
#define ESP_INTR_FLAG_DEFAULT 0

static const char *TAG = "gpio_interrupt";

// Interrupt handler - will be called on both rising and falling edges
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    // Get the current level of the input pin
    int level = gpio_get_level(GPIO_INPUT_PIN);
    // Set the LED to match the input level
    gpio_set_level(GPIO_OUTPUT_PIN, level); // Corrected from 'high' to 'level'
    // Print the level
    ESP_LOGI(TAG, "GPIO interrupt level: %d", level);
}

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

    // Configure input pin (LFG signal)
    io_conf.pin_bit_mask = (1ULL << GPIO_INPUT_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;  // Trigger on both rising and falling edges
    gpio_config(&io_conf);

    // Install GPIO interrupt service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_PIN, gpio_isr_handler, NULL);

    ESP_LOGI(TAG, "GPIO configured successfully");
}

void app_main(void)
{
    configure_gpio();
    
    // Initial state of the LED
    gpio_set_level(GPIO_OUTPUT_PIN, 0);

    // Keep the application running
    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}
