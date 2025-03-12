#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define GPIO_INPUT_PIN     4
#define GPIO_OUTPUT_PIN    21
#define ESP_INTR_FLAG_DEFAULT 0
#define PULSE_COUNT       50  // Number of complete cycles (high+low)
#define PULSE_DELAY_US    10  // Delay between state changes in microseconds

static const char *TAG = "Signal_Repeater";

// Function to generate precise delays using esp_timer
static inline void precise_delay_us(uint32_t us) {
    uint32_t start = esp_timer_get_time();
    while (esp_timer_get_time() - start < us);
}

// Function to generate multiple pulses with improved precision
static void IRAM_ATTR generate_pulses(void) {
    // Ensure we start from a known state
    gpio_set_level(GPIO_OUTPUT_PIN, 0);
    
    uint32_t start_time = esp_timer_get_time();
    uint32_t next_edge;
    
    // Generate exact square wave pulses
    for (int i = 0; i < PULSE_COUNT; i++) {
        // Rising edge
        next_edge = start_time + (i * 2 * PULSE_DELAY_US);
        while (esp_timer_get_time() < next_edge);
        gpio_set_level(GPIO_OUTPUT_PIN, 1);
        
        // Falling edge
        next_edge = start_time + (i * 2 * PULSE_DELAY_US) + PULSE_DELAY_US;
        while (esp_timer_get_time() < next_edge);
        gpio_set_level(GPIO_OUTPUT_PIN, 0);
    }
}

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    // Only react to rising edge
    if (gpio_get_level(GPIO_INPUT_PIN) == 1) {
        generate_pulses();
    }
}

static void configure_gpio(void) {
    // Configure output pin
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_OUTPUT_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Configure input pin with interrupt on rising edge only
    io_conf.pin_bit_mask = (1ULL << GPIO_INPUT_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.intr_type = GPIO_INTR_POSEDGE;  // Changed to trigger only on rising edge
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_PIN, gpio_isr_handler, NULL);

    ESP_LOGI(TAG, "GPIO configured: Input pin %d, Output pin %d", GPIO_INPUT_PIN, GPIO_OUTPUT_PIN);
    ESP_LOGI(TAG, "Interrupt set for rising edge only");
}

void app_main(void)
{
    configure_gpio();
    
    // Initial state of the output
    gpio_set_level(GPIO_OUTPUT_PIN, 0);

    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}