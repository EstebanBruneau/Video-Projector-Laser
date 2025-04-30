#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_attr.h"

// Pin Definitions
#define GPIO_MOTOR_PIN     4   // Motor rotation detection
#define GPIO_MIRROR_PIN    5   // Mirror position detection

// RGB Control Pins
#define GPIO_RED_SELECT    1  // Red color select
#define GPIO_GREEN_SELECT  2  // Green color select
#define GPIO_BLUE_SELECT   42  // Blue color select

// 8-bit Data Bus (starting from GPIO26)
#define GPIO_DATA_START_PIN 26

// Image Parameters
#define IMAGE_WIDTH  100
#define IMAGE_HEIGHT 100

// Interrupt Configuration
#define ESP_INTR_FLAG_DEFAULT 0

// Example Images (100x100 RGB matrices)
// Heart shape in red
static const uint8_t heart_image[IMAGE_HEIGHT][IMAGE_WIDTH][3] = {
    // Pre-defined heart pattern
    // Row 0-9: Empty rows
    [0 ... 9] = {{0,0,0}},
    // Row 10-19: Top curves of heart
    [10 ... 19] = {
        [0 ... 29] = {0,0,0},
        [30 ... 39] = {255,0,0},
        [40 ... 59] = {255,0,0},
        [60 ... 69] = {255,0,0},
        [70 ... 99] = {0,0,0}
    },
    // Row 20-59: Main heart body
    [20 ... 59] = {
        [20 ... 79] = {255,0,0},
        [0 ... 19] = {0,0,0},
        [80 ... 99] = {0,0,0}
    },
    // Row 60-89: Bottom point
    [60 ... 89] = {
        [30 ... 69] = {255,0,0},
        [0 ... 29] = {0,0,0},
        [70 ... 99] = {0,0,0}
    },
    // Row 90-99: Empty rows
    [90 ... 99] = {{0,0,0}}
};

// // Smiley face in yellow
// static const uint8_t smiley_image[IMAGE_HEIGHT][IMAGE_WIDTH][3] = {
//     // Pre-defined smiley pattern (yellow color: RGB 255,255,0)
//     // Implementation similar to heart pattern but with yellow pixels
// };

// State tracking
static volatile uint8_t current_row = 0;
static volatile uint8_t current_col = 0;
static const uint8_t (*current_image)[IMAGE_WIDTH][3] = heart_image;

// Function to set 8-bit value on data bus
static inline void IRAM_ATTR set_data_bus(uint8_t value) {
    for (int i = 0; i < 8; i++) {
        gpio_set_level(GPIO_DATA_START_PIN + i, (value >> i) & 0x01);
    }
}

// Function to output RGB values for current pixel
static void IRAM_ATTR output_rgb_values(void) {
    const uint8_t r = current_image[current_row][current_col][0];
    const uint8_t g = current_image[current_row][current_col][1];
    const uint8_t b = current_image[current_row][current_col][2];

    // Output Red
    gpio_set_level(GPIO_RED_SELECT, 1);
    gpio_set_level(GPIO_GREEN_SELECT, 0);
    gpio_set_level(GPIO_BLUE_SELECT, 0);
    set_data_bus(r);
    esp_rom_delay_us(1000);

    // Output Green
    gpio_set_level(GPIO_RED_SELECT, 0);
    gpio_set_level(GPIO_GREEN_SELECT, 1);
    gpio_set_level(GPIO_BLUE_SELECT, 0);
    set_data_bus(g);
    esp_rom_delay_us(1000);

    // Output Blue
    gpio_set_level(GPIO_RED_SELECT, 0);
    gpio_set_level(GPIO_GREEN_SELECT, 0);
    gpio_set_level(GPIO_BLUE_SELECT, 1);
    set_data_bus(b);
    esp_rom_delay_us(1000);

    // Reset all select pins
    gpio_set_level(GPIO_RED_SELECT, 0);
    gpio_set_level(GPIO_GREEN_SELECT, 0);
    gpio_set_level(GPIO_BLUE_SELECT, 0);
}

// Motor rotation ISR - Triggers start of new frame
static void IRAM_ATTR motor_isr_handler(void* arg) {
    current_row = 0;
    current_col = 0;
}

// Mirror position ISR - Triggers new line output
static void IRAM_ATTR mirror_isr_handler(void* arg) {
    if (current_row < IMAGE_HEIGHT) {
        output_rgb_values();
        
        // Move to next pixel
        current_col++;
        if (current_col >= IMAGE_WIDTH) {
            current_col = 0;
            current_row++;
        }
    }
}

static void configure_gpio(void) {
    gpio_config_t io_conf = {};
    
    // Configure input pins (motor and mirror sensors)
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pin_bit_mask = (1ULL << GPIO_MOTOR_PIN) | (1ULL << GPIO_MIRROR_PIN);
    gpio_config(&io_conf);

    // Configure output pins (RGB select and data bus)
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pin_bit_mask = (1ULL << GPIO_RED_SELECT) |
                          (1ULL << GPIO_GREEN_SELECT) |
                          (1ULL << GPIO_BLUE_SELECT);
    
    // Add data bus pins to output configuration
    for (int i = 0; i < 8; i++) {
        io_conf.pin_bit_mask |= (1ULL << (GPIO_DATA_START_PIN + i));
    }
    gpio_config(&io_conf);

    // Install GPIO ISR service and handlers
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_MOTOR_PIN, motor_isr_handler, NULL);
    gpio_isr_handler_add(GPIO_MIRROR_PIN, mirror_isr_handler, NULL);
}

void app_main(void) {
    // Configure GPIO pins and interrupts
    configure_gpio();
    
    // Initialize all outputs to 0
    for (int i = 0; i < 8; i++) {
        gpio_set_level(GPIO_DATA_START_PIN + i, 0);
    }
    gpio_set_level(GPIO_RED_SELECT, 0);
    gpio_set_level(GPIO_GREEN_SELECT, 0);
    gpio_set_level(GPIO_BLUE_SELECT, 0);

    // To switch images, simply change the current_image pointer to the desired image:
    // current_image = smiley_image;  // Uncomment to display smiley instead of heart

    // Main loop - system runs on interrupts, so we just need to keep the program alive
    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}