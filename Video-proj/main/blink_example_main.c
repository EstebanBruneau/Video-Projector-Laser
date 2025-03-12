#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define ESP_INTR_FLAG_DEFAULT 0
#define PULSE_COUNT       50  // Number of complete cycles (high+low)
#define PULSE_DELAY_US    10  // Delay between state changes in microseconds

// Input pins
#define MOTOR_PIN           4   // GPIO4  - Motor rotation detection
#define MIRROR_PIN          5   // GPIO5  - Mirror position detection

// RGB select pins
#define RED_SELECT_PIN     15   // GPIO15 - Red color select
#define GREEN_SELECT_PIN   16   // GPIO16 - Green color select
#define BLUE_SELECT_PIN    17   // GPIO17 - Blue color select

// 8-bit data bus pins
#define DATA_PIN_0         43   // GPIO43 - Data bit 0 (LSB)
#define DATA_PIN_1         44   // GPIO44 - Data bit 1
#define DATA_PIN_2         1   // GPIO1 - Data bit 2
#define DATA_PIN_3         2   // GPIO2 - Data bit 3
#define DATA_PIN_4         42   // GPIO42 - Data bit 4
#define DATA_PIN_5         41   // GPIO41 - Data bit 5
#define DATA_PIN_6         40   // GPIO40 - Data bit 6
#define DATA_PIN_7         39   // GPIO39 - Data bit 7 (MSB)

// Define pixel matrix (10x10 array of RGB values)
static const uint8_t pixel_matrix[10][10][3] = {
    {{0,50,0}, {0,100,0}, {0,150,0}, {0,200,0}, {0,200,0}, {0,200,0}, {0,200,0}, {0,150,0}, {0,100,0}, {0,50,0}},
    {{0,100,0}, {0,150,0}, {0,200,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,200,0}, {0,150,0}, {0,100,0}},
    {{0,150,0}, {0,200,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,200,0}, {0,150,0}},
    {{0,200,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,200,0}},
    {{0,200,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,200,0}},
    {{0,200,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,200,0}},
    {{0,200,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,200,0}},
    {{0,150,0}, {0,200,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,200,0}, {0,150,0}},
    {{0,100,0}, {0,150,0}, {0,200,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,255,0}, {0,200,0}, {0,150,0}, {0,100,0}},
    {{0,50,0}, {0,100,0}, {0,150,0}, {0,200,0}, {0,200,0}, {0,200,0}, {0,200,0}, {0,150,0}, {0,100,0}, {0,50,0}}
};

// Define black square matrix
static const uint8_t black_square_matrix[10][10][3] = {
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}}
};

// Define white square matrix
static const uint8_t white_square_matrix[10][10][3] = {
    {{255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}},
    {{255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}},
    {{255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}},
    {{255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}},
    {{255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}},
    {{255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}},
    {{255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}},
    {{255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}},
    {{255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}},
    {{255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}, {255,255,255}}
};

static const char *TAG = "Signal_Repeater";

// State tracking variables
static volatile uint8_t current_row = 0;
static volatile uint8_t current_col = 0;
static volatile bool matrix_processing = false;
static volatile bool line_processing = false;

// Function to simultaneously set color select and data pins
static void IRAM_ATTR output_pixel(uint8_t red, uint8_t green, uint8_t blue) {
    // First color (Red)
    gpio_set_level(RED_SELECT_PIN, 1);
    gpio_set_level(GREEN_SELECT_PIN, 0);
    gpio_set_level(BLUE_SELECT_PIN, 0);
    uint32_t gpio_mask = 0;
    for(int i = 0; i < 8; i++) {
        if((red >> i) & 1) {
            gpio_mask |= (1ULL << (DATA_PIN_0 + i));
        }
    }
    GPIO.out_w1ts.val = gpio_mask;
    GPIO.out_w1tc.val = (~gpio_mask) & ((1ULL << (DATA_PIN_0 + 8)) - (1ULL << DATA_PIN_0));
    precise_delay_us(PULSE_DELAY_US);

    // Second color (Green)
    gpio_set_level(RED_SELECT_PIN, 0);
    gpio_set_level(GREEN_SELECT_PIN, 1);
    gpio_set_level(BLUE_SELECT_PIN, 0);
    gpio_mask = 0;
    for(int i = 0; i < 8; i++) {
        if((green >> i) & 1) {
            gpio_mask |= (1ULL << (DATA_PIN_0 + i));
        }
    }
    GPIO.out_w1ts.val = gpio_mask;
    GPIO.out_w1tc.val = (~gpio_mask) & ((1ULL << (DATA_PIN_0 + 8)) - (1ULL << DATA_PIN_0));
    precise_delay_us(PULSE_DELAY_US);

    // Third color (Blue)
    gpio_set_level(RED_SELECT_PIN, 0);
    gpio_set_level(GREEN_SELECT_PIN, 0);
    gpio_set_level(BLUE_SELECT_PIN, 1);
    gpio_mask = 0;
    for(int i = 0; i < 8; i++) {
        if((blue >> i) & 1) {
            gpio_mask |= (1ULL << (DATA_PIN_0 + i));
        }
    }
    GPIO.out_w1ts.val = gpio_mask;
    GPIO.out_w1tc.val = (~gpio_mask) & ((1ULL << (DATA_PIN_0 + 8)) - (1ULL << DATA_PIN_0));
    precise_delay_us(PULSE_DELAY_US);

    // Reset all pins to 0
    gpio_set_level(RED_SELECT_PIN, 0);
    gpio_set_level(GREEN_SELECT_PIN, 0);
    gpio_set_level(BLUE_SELECT_PIN, 0);
    GPIO.out_w1tc.val = ((1ULL << (DATA_PIN_0 + 8)) - (1ULL << DATA_PIN_0));
}

// Motor signal ISR (Pin 4)
static void IRAM_ATTR motor_isr_handler(void* arg) {
    matrix_processing = true;
    current_row = 0;  // Reset row counter
    line_processing = true;  // Enable line processing
}

// Mirror signal ISR (Pin 5)
static void IRAM_ATTR mirror_isr_handler(void* arg) {
    if (matrix_processing && line_processing) {
        // Output current pixel
        output_pixel(
            pixel_matrix[current_row][current_col][0],
            pixel_matrix[current_row][current_col][1],
            pixel_matrix[current_row][current_col][2]
        );

        // Move to next column
        current_col++;
        if (current_col >= 10) {
            current_col = 0;
            line_processing = false;  // End of line
            current_row++;
            if (current_row >= 10) {
                matrix_processing = false;  // End of matrix
                current_row = 0;
            }
        }
    }
}

static void configure_gpio(void) {
    gpio_config_t io_conf = {};
    
    // Configure input pins (motor and mirror)
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = (1ULL << MOTOR_PIN) | (1ULL << MIRROR_PIN);
    gpio_config(&io_conf);

    // Configure output pins (RGB select and data pins)
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << RED_SELECT_PIN) | 
                          (1ULL << GREEN_SELECT_PIN) | 
                          (1ULL << BLUE_SELECT_PIN) |
                          (1ULL << DATA_PIN_0) | (1ULL << DATA_PIN_1) |
                          (1ULL << DATA_PIN_2) | (1ULL << DATA_PIN_3) |
                          (1ULL << DATA_PIN_4) | (1ULL << DATA_PIN_5) |
                          (1ULL << DATA_PIN_6) | (1ULL << DATA_PIN_7);
    gpio_config(&io_conf);

    // Install GPIO ISR service and handlers
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(MOTOR_PIN, motor_isr_handler, NULL);
    gpio_isr_handler_add(MIRROR_PIN, mirror_isr_handler, NULL);

    ESP_LOGI(TAG, "GPIO configured with motor pin %d and mirror pin %d", MOTOR_PIN, MIRROR_PIN);
}

void app_main(void) {
    // Initialize all outputs to 0
    gpio_set_level(RED_SELECT_PIN, 0);
    gpio_set_level(GREEN_SELECT_PIN, 0);
    gpio_set_level(BLUE_SELECT_PIN, 0);
    for (int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PIN_0 + i, 0);
    }
    
    configure_gpio();
    
    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}