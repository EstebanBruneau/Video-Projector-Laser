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

// Input pins
#define MOTOR_PIN           4   // GPIO4  - Motor rotation detection
#define MIRROR_PIN          5   // GPIO5  - Mirror position detection

// RGB select pins
#define RED_SELECT_PIN     22   // GPIO22 - Red color select
#define GREEN_SELECT_PIN   23   // GPIO23 - Green color select
#define BLUE_SELECT_PIN    25   // GPIO25 - Blue color select

// 8-bit data bus pins
#define DATA_PIN_0         26   // GPIO26 - Data bit 0 (LSB)
#define DATA_PIN_1         27   // GPIO27 - Data bit 1
#define DATA_PIN_2         28   // GPIO28 - Data bit 2
#define DATA_PIN_3         29   // GPIO29 - Data bit 3
#define DATA_PIN_4         30   // GPIO30 - Data bit 4
#define DATA_PIN_5         31   // GPIO31 - Data bit 5
#define DATA_PIN_6         32   // GPIO32 - Data bit 6
#define DATA_PIN_7         33   // GPIO33 - Data bit 7 (MSB)

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

static void affiche_pixel(void) {
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
