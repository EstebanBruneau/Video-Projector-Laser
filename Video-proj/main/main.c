#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_attr.h"

// States for state machine
#define STATE_ATTENTE_IMAGE 0
#define STATE_SWAP_BUFFER 1
#define STATE_ATTENTE_LIGNE 2
#define STATE_AFFICHE_LIGNE 3

// Pin definitions
#define RED_SELECT_PIN 15   // GPIO pin for red color
#define GREEN_SELECT_PIN 16  // GPIO pin for green color
#define BLUE_SELECT_PIN 17   // GPIO pin for blue color
#define MOTOR_PIN 4          // GPIO pin for motor rotation detection
#define MIRROR_PIN 5         // GPIO pin for mirror position detection

// Create array of data pins for data bus
static const uint8_t DATA_PINS[8] = {39, 40, 41, 42, 2, 1, 44, 43};  // GPIO pins for data bus

// Declare global state variables
static volatile uint8_t state = STATE_ATTENTE_IMAGE;
static volatile uint8_t pixel_counter = 0;
static volatile uint8_t line_counter = 0;
static bool using_red_matrix = true;

// Define the matrices
static const uint8_t red_matrix[100][100][3] = {
    [0 ... 99] = {
        [0 ... 99] = {255, 0, 0}  // Full red for all pixels
    }
};

static const uint8_t green_matrix[100][100][3] = {
    [0 ... 99] = {
        [0 ... 99] = {0, 255, 0}  // Full green for all pixels
    }
};

// Current matrix pointer
static const uint8_t (*current_matrix)[100][3] = NULL;

// ISR handlers
void IRAM_ATTR mirror_change_isr(void* arg) {
    state = STATE_AFFICHE_LIGNE;
    pixel_counter = 0;
}

void IRAM_ATTR motor_rotation_isr(void* arg) {
    state = STATE_SWAP_BUFFER;
}

void swap_buffer(void) {
    if (using_red_matrix) {
        current_matrix = green_matrix;
        using_red_matrix = false;
    } else {
        current_matrix = red_matrix;
        using_red_matrix = true;
    }
    state = STATE_ATTENTE_LIGNE;
}

void affiche_pixel(void) {
    uint8_t r = current_matrix[line_counter][pixel_counter][0];
    uint8_t g = current_matrix[line_counter][pixel_counter][1];
    uint8_t b = current_matrix[line_counter][pixel_counter][2];

    // Set RGB select pins low
    gpio_set_level(RED_SELECT_PIN, 0);
    gpio_set_level(GREEN_SELECT_PIN, 0);
    gpio_set_level(BLUE_SELECT_PIN, 0);
    
    // Set 8-bit color data for Red
    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PINS[i], (r >> i) & 0x01);
    }
    gpio_set_level(RED_SELECT_PIN, 1);
    esp_rom_delay_us(10);

    // Set 8-bit color data for Green
    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PINS[i], (g >> i) & 0x01);
    }
    gpio_set_level(GREEN_SELECT_PIN, 1);
    esp_rom_delay_us(10);

    // Set 8-bit color data for Blue
    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PINS[i], (b >> i) & 0x01);
    }
    gpio_set_level(BLUE_SELECT_PIN, 1);
    esp_rom_delay_us(10);
}

void setup(void) {
    // Configure input pins with interrupts
    gpio_config_t io_conf = {};
    
    // Configure input pins
    io_conf.pin_bit_mask = (1ULL << MOTOR_PIN) | (1ULL << MIRROR_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.intr_type = GPIO_INTR_POSEDGE;  // Trigger on rising edge
    gpio_config(&io_conf);

    // Configure output pins
    io_conf.pin_bit_mask = 0;
    io_conf.pin_bit_mask |= (1ULL << RED_SELECT_PIN);
    io_conf.pin_bit_mask |= (1ULL << GREEN_SELECT_PIN);
    io_conf.pin_bit_mask |= (1ULL << BLUE_SELECT_PIN);
    for(int i = 0; i < 8; i++) {
        io_conf.pin_bit_mask |= (1ULL << DATA_PINS[i]);
    }
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(0);
    
    // Add ISR handlers
    gpio_isr_handler_add(MOTOR_PIN, motor_rotation_isr, NULL);
    gpio_isr_handler_add(MIRROR_PIN, mirror_change_isr, NULL);

    // Initialize state
    state = STATE_ATTENTE_IMAGE;
    current_matrix = red_matrix;
    using_red_matrix = true;
    
    ESP_LOGI("SETUP", "GPIO configuration complete");
}

void app_main(void) {
    // Initialize
    setup();
    
    ESP_LOGI("MAIN", "Starting main loop");
    
    // Main loop
    while(1) {
        switch(state) {
            case STATE_ATTENTE_IMAGE:
                line_counter = 0;
                break;
                
            case STATE_SWAP_BUFFER:
                swap_buffer();
                break;
                
            case STATE_ATTENTE_LIGNE:
                pixel_counter = 0;
                break;
                
            case STATE_AFFICHE_LIGNE:
                affiche_pixel();
                pixel_counter++;
                if(pixel_counter >= 100) {
                    line_counter++;
                    if(line_counter >= 100) {
                        state = STATE_ATTENTE_IMAGE;
                    } else {
                        state = STATE_ATTENTE_LIGNE;
                    }
                }
                break;
        }
        
        // Small delay to prevent CPU overload
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}