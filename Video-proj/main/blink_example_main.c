#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define BLINK_GPIO 21  // ⚡ Change ce numéro selon le GPIO de ta LED
#define BLINK_PERIOD 1000 // Temps en ms entre chaque changement d'état
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO      (BLINK_GPIO) // Define the output GPIO
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_DUTY_RES       LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY      (5000) // Frequency in Hertz. Set frequency at 5 kHz

static const char *TAG = "Blink_GPIO";
static uint8_t s_led_state = 0; // État de la LED (0 = OFF, 1 = ON)

static void configure_led(void)
{
    ESP_LOGI(TAG, "Configuration de la LED sur GPIO %d", BLINK_GPIO);

    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

static void set_led_intensity(uint32_t duty)
{
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void app_main(void)
{
    configure_led();

    uint32_t duty = 0;
    uint32_t max_duty = (1 << LEDC_DUTY_RES) - 1; // Maximum duty cycle value

    while (1) {
        // Turn on the LED with increasing intensity
        ESP_LOGI(TAG, "LED intensity: %d", duty);
        set_led_intensity(duty);
        vTaskDelay(BLINK_PERIOD / portTICK_PERIOD_MS); // Wait for the blink period

        // Turn off the LED
        set_led_intensity(0);
        vTaskDelay(BLINK_PERIOD / portTICK_PERIOD_MS); // Wait for the blink period

        // Increase intensity
        duty = (duty + max_duty / 10) % max_duty;
    }
}