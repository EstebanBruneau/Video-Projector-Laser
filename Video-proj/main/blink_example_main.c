#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/gptimer.h"

#define GPIO_INPUT_PIN     4
#define GPIO_OUTPUT_PIN    21
#define ESP_INTR_FLAG_DEFAULT 0
#define PULSE_COUNT       50  // Number of complete cycles (high+low)
#define PULSE_DELAY_US    10  // Delay between state changes in microseconds

static const char *TAG = "Signal_Repeater";

static gptimer_handle_t gptimer = NULL;
static volatile bool timer_expired = false;

static bool IRAM_ATTR timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    timer_expired = true;
    return false;
}

static void timer_init(void) {
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_on_alarm_cb,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
}

static void IRAM_ATTR precise_delay_us(uint32_t us) {
    timer_expired = false;
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = us,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = false
    };
    gptimer_set_raw_count(gptimer, 0);
    gptimer_set_alarm_action(gptimer, &alarm_config);
    gptimer_start(gptimer);
    while (!timer_expired) {
        // Wait for timer
    }
    gptimer_stop(gptimer);
}

// Function to generate multiple pulses
static void IRAM_ATTR generate_pulses(void) {
    for (int i = 0; i < PULSE_COUNT; i++) {
        gpio_set_level(GPIO_OUTPUT_PIN, 1);
        precise_delay_us(PULSE_DELAY_US);
        gpio_set_level(GPIO_OUTPUT_PIN, 0);
        precise_delay_us(PULSE_DELAY_US);
    }
}

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    // Only react to rising edge
    if (gpio_get_level(GPIO_INPUT_PIN) == 1) {
        generate_pulses();
    }
}

static void configure_gpio(void) {
    // Configure output pin
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_OUTPUT_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Configure input pin with interrupt on rising edge only
    io_conf.pin_bit_mask = (1ULL << GPIO_INPUT_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.intr_type = GPIO_INTR_POSEDGE;  // Changed to trigger only on rising edge
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_PIN, gpio_isr_handler, NULL);

    ESP_LOGI(TAG, "GPIO configured: Input pin %d, Output pin %d", GPIO_INPUT_PIN, GPIO_OUTPUT_PIN);
    ESP_LOGI(TAG, "Interrupt set for rising edge only");
}

void app_main(void)
{
    timer_init();
    configure_gpio();
    
    // Initial state of the output
    gpio_set_level(GPIO_OUTPUT_PIN, 0);

    while(1) {
        vTaskDelay(portMAX_DELAY);
    }
}