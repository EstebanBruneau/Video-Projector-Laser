#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

// States definition
#define ETAT_ATTENTE_IMAGE 0
#define ETAT_SWAP_BUFFER   1
#define ETAT_ATTENTE_LIGNE 2
#define ETAT_AFFICHE_LIGNE 3

// Function declarations
void init_machine_etats(void);
void process_state(void);
void IRAM_ATTR motor_rotation_isr(void* arg);
void IRAM_ATTR mirror_change_isr(void* arg);

#define MOTOR_PIN       4  // Pin for motor rotation detection
#define MIRROR_PIN      5  // Pin for mirror position detection
#define COLOR_START_PIN 22 // Starting pin for RGB control
#define DATA_START_PIN  26 // Starting pin for 8-bit color data

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

static void affiche_pixel(void) {
    uint8_t r = current_matrix[line_counter][pixel_counter][0];
    uint8_t g = current_matrix[line_counter][pixel_counter][1];
    uint8_t b = current_matrix[line_counter][pixel_counter][2];

    // Set RGB select pins
    for(int i = 0; i < 3; i++) {
        gpio_set_level(COLOR_START_PIN + i, 0);
    }

    // Set 8-bit color data
    // Red
    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_START_PIN + i, (r >> i) & 0x01);
    }
    gpio_set_level(COLOR_START_PIN, 1);
    esp_rom_delay_us(10);

    // Green
    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_START_PIN + i, (g >> i) & 0x01);
    }
    gpio_set_level(COLOR_START_PIN + 1, 1);
    esp_rom_delay_us(10);

    // Blue
    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_START_PIN + i, (b >> i) & 0x01);
    }
    gpio_set_level(COLOR_START_PIN + 2, 1);
    esp_rom_delay_us(10);
}

void IRAM_ATTR motor_rotation_isr(void* arg) {
    current_state = ETAT_SWAP_BUFFER;
}

void IRAM_ATTR mirror_change_isr(void* arg) {
    current_state = ETAT_AFFICHE_LIGNE;
    pixel_counter = 0;
}

void init_machine_etats(void) {
    // Configure GPIO pins
    gpio_config_t io_conf = {};
    
    // Configure input pins
    io_conf.pin_bit_mask = (1ULL << MOTOR_PIN) | (1ULL << MIRROR_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    gpio_config(&io_conf);

    // Configure output pins
    io_conf.pin_bit_mask = 0;
    for(int i = 0; i < 3; i++) {
        io_conf.pin_bit_mask |= (1ULL << (COLOR_START_PIN + i));
    }
    for(int i = 0; i < 8; i++) {
        io_conf.pin_bit_mask |= (1ULL << (DATA_START_PIN + i));
    }
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    // Install ISR service and add ISR handlers
    gpio_install_isr_service(0);
    gpio_isr_handler_add(MOTOR_PIN, motor_rotation_isr, NULL);
    gpio_isr_handler_add(MIRROR_PIN, mirror_change_isr, NULL);
}

void process_state(void) {
    switch(current_state) {
        case ETAT_ATTENTE_IMAGE:
            // Wait for motor rotation interrupt
            break;

        case ETAT_SWAP_BUFFER:
            {
                int64_t current_time = esp_timer_get_time() / 1000;  // Convert to milliseconds
                if (current_time - last_swap_time >= SWAP_DELAY_MS) {
                    using_red_matrix = !using_red_matrix;
                    current_matrix = using_red_matrix ? red_matrix : green_matrix;
                    last_swap_time = current_time;
                }
                line_counter = 0;
                current_state = ETAT_ATTENTE_LIGNE;
            }
            break;

        case ETAT_ATTENTE_LIGNE:
            // Wait for mirror change interrupt
            break;

        case ETAT_AFFICHE_LIGNE:
            affiche_pixel();
            pixel_counter++;
            
            if(pixel_counter >= PIXELS_PER_LINE) {
                line_counter++;
                if(line_counter >= LINES_PER_FRAME) {
                    current_state = ETAT_ATTENTE_IMAGE;
                } else {
                    current_state = ETAT_ATTENTE_LIGNE;
                }
            }
            break;
    }
}
