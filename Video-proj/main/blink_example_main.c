#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BLINK_GPIO 21  // ⚡ Change ce numéro selon le GPIO de ta LED
#define BLINK_PERIOD 1000 // Temps en ms entre chaque changement d'état

static const char *TAG = "Blink_GPIO";
static uint8_t s_led_state = 0; // État de la LED (0 = OFF, 1 = ON)

static void blink_led(void)
{
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Configuration de la LED sur GPIO %d", BLINK_GPIO);
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
    configure_led();

    while (1) {
        ESP_LOGI(TAG, "LED %s", s_led_state ? "ON" : "OFF");
        blink_led();
        s_led_state = !s_led_state; // Toggle LED state
        vTaskDelay(BLINK_PERIOD / portTICK_PERIOD_MS); // Attendre avant de changer l’état
    }
}
