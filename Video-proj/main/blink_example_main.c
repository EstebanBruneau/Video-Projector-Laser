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

// Define color control pins
#define GPIO_RED_SELECT    22
#define GPIO_GREEN_SELECT  23
#define GPIO_BLUE_SELECT   25

// Define 8 data pins for color intensity (assuming consecutive pins for simplicity)
#define GPIO_DATA_START_PIN 26  // First pin of the 8-bit data bus

// Define pixel matrix (10x10 array of RGB values)
static const uint8_t pixel_matrix[10][10][3] = {
    // Initialize with some example values - you can modify these
    [0 ... 9] = {
        [0 ... 9] = {200, 127, 255}  // RGB values for each pixel
    }
};

static const char *TAG = "Signal_Repeater";

// Optimize GPIO access with direct port registers
#define GPIO_OUT_REG     0x3FF44004
#define GPIO_OUT_W1TS    0x3FF44008
#define GPIO_OUT_W1TC    0x3FF4400C

// Pre-calculate color select patterns
#define COLOR_SELECT_MASK ((1ULL << GPIO_RED_SELECT) | (1ULL << GPIO_GREEN_SELECT) | (1ULL << GPIO_BLUE_SELECT))
#define RED_PATTERN      (1ULL << GPIO_RED_SELECT)
#define GREEN_PATTERN    (1ULL << GPIO_GREEN_SELECT)
#define BLUE_PATTERN     (1ULL << GPIO_BLUE_SELECT)

// Pre-calculate data patterns for faster access
static uint32_t data_patterns[256];

// Initialize lookup table for faster bit manipulation
static void init_data_patterns(void) {
    for (int i = 0; i < 256; i++) {
        uint32_t pattern = 0;
        for (int bit = 0; bit < 8; bit++) {
            if (i & (1 << bit)) {
                pattern |= (1ULL << (GPIO_DATA_START_PIN + bit));
            }
        }
        data_patterns[i] = pattern;
    }
}

// Function to generate precise delays using esp_timer
static inline void precise_delay_us(uint32_t us) {
    uint32_t start = esp_timer_get_time();
    while (esp_timer_get_time() - start < us);
}

// Optimized color value setting using direct port manipulation
static inline void IRAM_ATTR set_color_value_fast(uint8_t value) {
    // Clear previous data bits
    REG_WRITE(GPIO_OUT_W1TC, data_patterns[0xFF]);
    // Set new data bits
    REG_WRITE(GPIO_OUT_W1TS, data_patterns[value]);
}

// Modified pulse generation function to handle colors
static void IRAM_ATTR generate_pulses(void) {
    uint32_t start_time = esp_timer_get_time();
    
    // Rising edge
    REG_WRITE(GPIO_OUT_W1TS, (1ULL << GPIO_OUTPUT_PIN));
    
    // Send RGB values with minimal overhead
    const uint32_t colors[] = {200, 127, 255};
    const uint32_t color_patterns[] = {RED_PATTERN, GREEN_PATTERN, BLUE_PATTERN};
    
    for (int i = 0; i < 3; i++) {
        // Clear previous color select
        REG_WRITE(GPIO_OUT_W1TC, COLOR_SELECT_MASK);
        // Set new color select and value simultaneously
        REG_WRITE(GPIO_OUT_W1TS, color_patterns[i]);
        set_color_value_fast(colors[i]);
    }
    
    // Ensure minimum pulse width
    while (esp_timer_get_time() - start_time < PULSE_DELAY_US);
    
    // Falling edge
    REG_WRITE(GPIO_OUT_W1TC, (1ULL << GPIO_OUTPUT_PIN) | COLOR_SELECT_MASK);
}

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    // Only react to rising edge
    if (gpio_get_level(GPIO_INPUT_PIN) == 1) {
        generate_pulses();
    }
}

static void configure_gpio(void) {
    gpio_config_t io_conf = {};
    
    // Configure output pins (including color select and data pins)
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    
    // Set bit mask for all output pins
    io_conf.pin_bit_mask = (1ULL << GPIO_OUTPUT_PIN) |
                          (1ULL << GPIO_RED_SELECT) |
                          (1ULL << GPIO_GREEN_SELECT) |
                          (1ULL << GPIO_BLUE_SELECT);
    
    // Add data pins to bit mask
    for (int i = 0; i < 8; i++) {
        io_conf.pin_bit_mask |= (1ULL << (GPIO_DATA_START_PIN + i));
    }
    gpio_config(&io_conf);

    // Configure input pin
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

void app_main(void) {
    init_data_patterns();  // Initialize lookup table
    configure_gpio();
    
    // Initial state of the output
    gpio_set_level(GPIO_OUTPUT_PIN, 0);

    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}