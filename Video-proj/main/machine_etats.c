#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_attr.h"  // Add this include for IRAM_ATTR

static const char* TAG = "VIDEO_PROJ";

// States definition
#define ETAT_ATTENTE_IMAGE 0
#define ETAT_SWAP_BUFFER   1
#define ETAT_ATTENTE_LIGNE 2
#define ETAT_AFFICHE_LIGNE 3

// Function declarations
void init_machine_etats(void);
void process_state(void);
void motor_rotation_isr(void* arg);  // Remove static and IRAM_ATTR from declaration
void mirror_change_isr(void* arg);   // Remove static and IRAM_ATTR from declaration

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

// Update COLOR_START_PIN to use first RGB select pin
#define COLOR_START_PIN    RED_SELECT_PIN

// Create array of data pins for easier iteration
static const uint8_t DATA_PINS[8] = {
    DATA_PIN_0, DATA_PIN_1, DATA_PIN_2, DATA_PIN_3,
    DATA_PIN_4, DATA_PIN_5, DATA_PIN_6, DATA_PIN_7
};

#define PIXEL_DELAY_US  10000 // 10ms delay between pixels
#define PIXELS_PER_LINE 100
#define LINES_PER_FRAME 100
#define SWAP_DELAY_MS 2000  // 2 seconds delay between image swaps

volatile uint8_t current_state = ETAT_ATTENTE_IMAGE;
volatile uint8_t pixel_counter = 0;
volatile uint8_t line_counter = 0;
static int64_t last_swap_time = 0;

// Define two static image matrices (100x100 RGB)
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

// Pointer to current matrix being displayed
static const uint8_t (*current_matrix)[100][3] = red_matrix;
static bool using_red_matrix = true;

// Add debug counters
static volatile uint32_t motor_interrupt_count = 0;
static volatile uint32_t mirror_interrupt_count = 0;

static void affiche_pixel(void) {
    uint8_t r = current_matrix[line_counter][pixel_counter][0];
    uint8_t g = current_matrix[line_counter][pixel_counter][1];
    uint8_t b = current_matrix[line_counter][pixel_counter][2];

    // Set all RGB select pins low initially
    gpio_set_level(RED_SELECT_PIN, 0);
    gpio_set_level(GREEN_SELECT_PIN, 0);
    gpio_set_level(BLUE_SELECT_PIN, 0);

    // Set and display Red
    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PINS[i], (r >> i) & 0x01);
    }
    gpio_set_level(RED_SELECT_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(RED_SELECT_PIN, 0);  // Turn off red before moving to green

    // Set and display Green
    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PINS[i], (g >> i) & 0x01);
    }
    gpio_set_level(GREEN_SELECT_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(GREEN_SELECT_PIN, 0);  // Turn off green before moving to blue

    // Set and display Blue
    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PINS[i], (b >> i) & 0x01);
    }
    gpio_set_level(BLUE_SELECT_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(BLUE_SELECT_PIN, 0);  // Turn off blue when done
}

void IRAM_ATTR motor_rotation_isr(void* arg) {
    motor_interrupt_count++;
    current_state = ETAT_SWAP_BUFFER;
}

void IRAM_ATTR mirror_change_isr(void* arg) {
    mirror_interrupt_count++;
    current_state = ETAT_AFFICHE_LIGNE;
    pixel_counter = 0;
}

void init_machine_etats(void) {
    // Configure GPIO pins
    gpio_config_t io_conf = {};
    
    // Configure input pins with appropriate filtering
    io_conf.pin_bit_mask = (1ULL << MOTOR_PIN) | (1ULL << MIRROR_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;  // Changed from POSEDGE to ANYEDGE
    gpio_config(&io_conf);

    // Set input filter parameters for more reliable detection
    gpio_set_intr_type(MOTOR_PIN, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(MIRROR_PIN, GPIO_INTR_ANYEDGE);
    
    // Enable glitch filter
    gpio_set_glitch_filter(MOTOR_PIN, true);
    gpio_set_glitch_filter(MIRROR_PIN, true);

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

    // Install ISR service and add ISR handlers
    gpio_install_isr_service(0);
    gpio_isr_handler_add(MOTOR_PIN, motor_rotation_isr, NULL);
    gpio_isr_handler_add(MIRROR_PIN, mirror_change_isr, NULL);

    ESP_LOGI(TAG, "GPIO interrupt configuration complete");
}

void process_state(void) {
    static uint32_t last_motor_count = 0;
    static uint32_t last_mirror_count = 0;

    // Log interrupt counts periodically
    if (motor_interrupt_count != last_motor_count) {
        ESP_LOGI(TAG, "Motor interrupts: %lu", motor_interrupt_count);
        last_motor_count = motor_interrupt_count;
    }
    if (mirror_interrupt_count != last_mirror_count) {
        ESP_LOGI(TAG, "Mirror interrupts: %lu", mirror_interrupt_count);
        last_mirror_count = mirror_interrupt_count;
    }

    switch(current_state) {
        case ETAT_ATTENTE_IMAGE:
            if (motor_interrupt_count == 0) {
                ESP_LOGI(TAG, "Waiting for first motor interrupt...");
                vTaskDelay(pdMS_TO_TICKS(1000));  // Log every second while waiting
            }
            break;

        case ETAT_SWAP_BUFFER:
            {
                int64_t current_time = esp_timer_get_time() / 1000;  // Convert to milliseconds
                if (current_time - last_swap_time >= SWAP_DELAY_MS) {
                    using_red_matrix = !using_red_matrix;
                    current_matrix = using_red_matrix ? red_matrix : green_matrix;
                    last_swap_time = current_time;
                    ESP_LOGI(TAG, "Buffer swapped to %s matrix", using_red_matrix ? "RED" : "GREEN");
                }
                line_counter = 0;
                current_state = ETAT_ATTENTE_LIGNE;
                ESP_LOGI(TAG, "Switching to ATTENTE_LIGNE state");
            }
            break;

        case ETAT_ATTENTE_LIGNE:
            if (mirror_interrupt_count == 0) {
                ESP_LOGI(TAG, "Waiting for first mirror interrupt...");
                vTaskDelay(pdMS_TO_TICKS(100));  // Log every 100ms while waiting
            }
            break;

        case ETAT_AFFICHE_LIGNE:
            affiche_pixel();
            pixel_counter++;
            
            if(pixel_counter >= PIXELS_PER_LINE) {
                line_counter++;
                if(line_counter >= LINES_PER_FRAME) {
                    current_state = ETAT_ATTENTE_IMAGE;
                    ESP_LOGI(TAG, "Frame complete - Switching to ATTENTE_IMAGE");
                } else {
                    current_state = ETAT_ATTENTE_LIGNE;
                    ESP_LOGI(TAG, "Line complete - Waiting for next line");
                }
            }
            break;
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Initializing state machine");
    init_machine_etats();
    ESP_LOGI(TAG, "State machine initialized, starting main loop");
    ESP_LOGI(TAG, "Expecting: Motor frequency=10Hz, Mirror frequency=1kHz");
    
    // Main loop
    while (1) {
        process_state();
        vTaskDelay(pdMS_TO_TICKS(1));  // Small delay to prevent watchdog triggers
    }
}
