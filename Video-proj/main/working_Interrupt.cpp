#include <Arduino.h>

// Pin definitions
const int LED_PIN = GPIO_NUM_2;
const int BUTTON_PIN = GPIO_NUM_1; // Button pi
const bool BUTTON_STATE = LOW; // Button state when pressed

void IRAM_ATTR buttonPressOn() {
  //print to serial
    // Serial.println("Button pressed! LED should be OFF");
    if(digitalRead(LED_PIN) == HIGH) {
        digitalWrite(LED_PIN, LOW); // Turn off LED
    } else {
        digitalWrite(LED_PIN, HIGH); // Turn on LED
    }
}


void setup() {
  // Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Enable pull-up resistor for button
  digitalWrite(LED_PIN, LOW); // Turn off LED initially
  
  // Attach interrupts to button pin
  attachInterrupt(BUTTON_PIN, buttonPressOn, RISING); 
}

void loop() {
  // Main loop - no infinite loop needed
  // The interrupts will handle the button events
}