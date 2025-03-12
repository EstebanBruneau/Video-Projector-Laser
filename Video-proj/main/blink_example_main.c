#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"

#define LED_GREEN_GPIO    21
#define GPIO_INTERRUPT_PIN 4  // Choose your interrupt pin
#define ESP_INTR_FLAG_DEFAULT 0

static const char *TAG = "GPIO_Interrupt";

// Interrupt handler
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    int input_level = gpio_get_level(GPIO_INTERRUPT_PIN);
    gpio_set_level(LED_GREEN_GPIO, input_level);
}

static void configure_gpio(void)
{
    // Configure LED_GREEN_GPIO as output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_GREEN_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Configure GPIO_INTERRUPT_PIN as input with interrupt on rising edge
    io_conf.pin_bit_mask = (1ULL << GPIO_INTERRUPT_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    gpio_config(&io_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // Attach the interrupt handler to the GPIO_INTERRUPT_PIN
    gpio_isr_handler_add(GPIO_INTERRUPT_PIN, gpio_isr_handler, NULL);
}

void app_main(void)
{
    // Configure GPIOs
    configure_gpio();

    // Initial state of the LED
    gpio_set_level(LED_GREEN_GPIO, 0);

    // Wait indefinitely
    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}