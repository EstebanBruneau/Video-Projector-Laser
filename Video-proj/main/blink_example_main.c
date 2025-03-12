#include "esp_wifi.h"
#include "esp_bt.h"
#include <stdio.h>

void disable_wifi() {
    printf("Disabling Wi-Fi...\n");
    // Deinitialize Wi-Fi
    esp_wifi_stop();         // Stop Wi-Fi driver
    esp_wifi_deinit();       // Deinitialize Wi-Fi
    printf("Wi-Fi disabled successfully\n");
}

void disable_bluetooth() {
    printf("Disabling Bluetooth...\n");
    // Deinitialize Bluetooth controller
    esp_bluedroid_disable(); // Disable Bluedroid stack
    esp_bluedroid_deinit();  // Deinitialize Bluedroid stack
    esp_bt_controller_disable(); // Disable the Bluetooth controller
    esp_bt_controller_deinit();  // Deinitialize the Bluetooth controller
    printf("Bluetooth disabled successfully\n");
}

void app_main() {
    printf("Starting initialization...\n");
    disable_wifi();
    disable_bluetooth();
    printf("Initialization complete\n");
}
