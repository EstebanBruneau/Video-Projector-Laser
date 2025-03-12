#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_rom.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_attr.h"

//states for switch
#define STATE_ATTENTE_IMAGE 0
#define STATE_SWAP_BUFFER 1
#define STATE_AFFICHE_LIGNE 3
#define STATE_ATTENTE_LIGNE 2

//pin selction
#define GPIO_INTR_Motor 0
#define GPIO_INTR_Mirror 1
#define RED_SELECT_PIN 0
#define GREEN_SELECT_PIN 1
#define BLUE_SELECT_PIN 2
#define DATA_PINS {3, 4, 5, 6, 7, 8, 9, 10}
#define MOTOR_PIN 11
#define MIRROR_PIN 12

// Define the matrix
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

void setup(){
    attachinterrupt(GPIO_INTR_Motor, motorChange, CHANGE);
    attachinterrupt(GPIO_INTR_Mirror, mirrorCycle , CHANGE);
    state=STATE_ATTENTE_IMAGE;
    current_matrix = red_matrix;
    using_red_matrix = true;
}

void IRAM_ATTR mirrorChange(){
    state = STATE_AFFICHE_LIGNE;
}

void IRAM_ATTR motorCycle(){
    state = STATE_SWAP_BUFFER;
}

void swap_buffer(){
    if (using_red_matrix) {
        current_matrix = green_matrix;
        !using_red_matrix;
    } else {
        current_matrix = red_matrix;
        using_red_matrix;
    }
    state = STATE_ATTENTE_LIGNE;
}

void affiche_pixel(){
    uint8_t r = current_matrix[line_counter][pixel_counter][0];
    uint8_t g = current_matrix[line_counter][pixel_counter][1];
    uint8_t b = current_matrix[line_counter][pixel_counter][2];

    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PINS[i], (r >> i) & 0x01);
    }
    gpio_set_level(RED_SELECT_PIN, 1);
    esp_rom_delay_us(10);

    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PINS[i], (g >> i) & 0x01);
    }
    gpio_set_level(GREEN_SELECT_PIN, 1);
    esp_rom_delay_us(10);

    for(int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PINS[i], (b >> i) & 0x01);
    }
    gpio_set_level(BLUE_SELECT_PIN, 1);
    esp_rom_delay_us(10);
}

int main(){
    setup();
    while(1){
        switch(state){
            case STATE_ATTENTE_IMAGE:
                break;
            case STATE_SWAP_BUFFER:
                swap_buffer();
                break;
            case STATE_ATTENTE_LIGNE:
                break;
            case STATE_AFFICHE_LIGNE:
                affiche_pixel();
                pixel_counter++;
                if(pixel_counter >= 100) {
                    line_counter++;
                    if(line_counter >= 100) {
                        current_state = STATE_ATTENTE_IMAGE;
                    } else {
                        current_state = STATE_ATTENTE_LIGNE;
                    }
                }
                break;
        }
    }
    return 0;
}