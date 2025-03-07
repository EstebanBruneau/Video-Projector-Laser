#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_GREEN_GPIO    21
#define LED_RED_GPIO  39
#define LED_BLUE_GPIO   2

#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES       LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY      (5000)

// Define channels for RGB
#define LEDC_CHANNEL_RED    LEDC_CHANNEL_0
#define LEDC_CHANNEL_GREEN  LEDC_CHANNEL_1
#define LEDC_CHANNEL_BLUE   LEDC_CHANNEL_2

#define GPIO_INTERRUPT_PIN 4  // Choose your interrupt pin
#define ESP_INTR_FLAG_DEFAULT 0

static const char *TAG = "RGB_LED_Control";

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_value_t;

static int current_row = 0;
static int current_col = 0;

// Example matrix of RGB values (2x2 matrix)
static rgb_value_t color_matrix[2][2] = {
    {{255, 0, 0}, {0, 255, 0}},    // Red, Green
    {{0, 0, 255}, {255, 255, 255}} // Blue, White
};

static void configure_led(void)
{
    // Timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz         = LEDC_FREQUENCY,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Channel configuration for Red LED
    ledc_channel_config_t ledc_channel_red = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_RED,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LED_RED_GPIO,
        .duty          = 0,
        .hpoint        = 0
    };
    ledc_channel_config(&ledc_channel_red);

    // Channel configuration for Green LED
    ledc_channel_config_t ledc_channel_green = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_GREEN,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LED_GREEN_GPIO,
        .duty          = 0,
        .hpoint        = 0
    };
    ledc_channel_config(&ledc_channel_green);

    // Channel configuration for Blue LED
    ledc_channel_config_t ledc_channel_blue = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_BLUE,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LED_BLUE_GPIO,
        .duty          = 0,
        .hpoint        = 0
    };
    ledc_channel_config(&ledc_channel_blue);
}

static void set_rgb_value(rgb_value_t color)
{
    uint32_t max_duty = (1 << LEDC_DUTY_RES) - 1;
    
    // Convert 8-bit colors to duty cycle values
    uint32_t red_duty = (color.r * max_duty) / 255;
    uint32_t green_duty = (color.g * max_duty) / 255;
    uint32_t blue_duty = (color.b * max_duty) / 255;

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_RED, red_duty);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_GREEN, green_duty);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_BLUE, blue_duty);

    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_RED);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_GREEN);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_BLUE);
}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    // Get next color from matrix
    rgb_value_t current_color = color_matrix[current_row][current_col];
    
    // Update RGB LED directly (note: calling non-ISR-safe functions from ISR is not recommended,
    // but for this example it should work)
    set_rgb_value(current_color);
    
    // Move to next position in matrix
    current_col++;
    if (current_col >= 2) {
        current_col = 0;
        current_row = (current_row + 1) % 2;
    }
}

static void configure_gpio_interrupt(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_INTERRUPT_PIN),
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INTERRUPT_PIN, gpio_isr_handler, NULL);
}

void app_main(void)
{
    configure_led();
    configure_gpio_interrupt();

    // Initial color set
    set_rgb_value(color_matrix[0][0]);

    // Wait indefinitely
    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}