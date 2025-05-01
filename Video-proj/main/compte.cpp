#include <Arduino.h>

//Pin qui fonctionnent 1,2,42,41,40,39,38,0,45,21,20,19
//37,36,35 pose un pb dès qu'on a un pin desus
//48,47 s'allume juste pas
// Définition des broches pour les LEDs
#define GPIO_DATA_PIN1 1   // Bit le moins significatif (LSB)
#define GPIO_DATA_PIN2 2
#define GPIO_DATA_PIN3 42
#define GPIO_DATA_PIN4 41  // Bit le plus significatif (MSB)
#define GPIO_DATA_PIN5 40
#define GPIO_DATA_PIN6 39
#define GPIO_DATA_PIN7 38
#define GPIO_DATA_PIN8 0
#define GPIO_DATA_PIN9 45
#define GPIO_DATA_PIN10 21
#define GPIO_DATA_PIN11 20
#define GPIO_DATA_PIN12 19

// Fonction pour afficher le compteur binaire
void binary_counter() {
  // Compte de 0 à 15 en binaire en utilisant une boucle
  for (int i = 0; i < 4095; i++) {
    // Set each bit according to the binary representation of i
    digitalWrite(GPIO_DATA_PIN1, (i & 0x01) ? HIGH : LOW); // Bit 0 (LSB)
    digitalWrite(GPIO_DATA_PIN2, (i & 0x02) ? HIGH : LOW); // Bit 1
    digitalWrite(GPIO_DATA_PIN3, (i & 0x04) ? HIGH : LOW); // Bit 2
    digitalWrite(GPIO_DATA_PIN4, (i & 0x08) ? HIGH : LOW); // Bit 3
    digitalWrite(GPIO_DATA_PIN5, (i & 0x10) ? HIGH : LOW); // Bit 4
    digitalWrite(GPIO_DATA_PIN6, (i & 0x20) ? HIGH : LOW); // Bit 5
    digitalWrite(GPIO_DATA_PIN7, (i & 0x40) ? HIGH : LOW); // Bit 6
    digitalWrite(GPIO_DATA_PIN8, (i & 0x80) ? HIGH : LOW); // Bit 7
    digitalWrite(GPIO_DATA_PIN9, (i & 0x100) ? HIGH : LOW); // Bit 8
    digitalWrite(GPIO_DATA_PIN10, (i & 0x200) ? HIGH : LOW); // Bit 9
    digitalWrite(GPIO_DATA_PIN11, (i & 0x400) ? HIGH : LOW); // Bit 10
    digitalWrite(GPIO_DATA_PIN12, (i & 0x800) ? HIGH : LOW); // Bit 11
    
    delay(1000); // Wait a second before moving to the next number
  }
}

void setup() {
  // Configuration des broches LED en sortie
  pinMode(GPIO_DATA_PIN1, OUTPUT);
  pinMode(GPIO_DATA_PIN2, OUTPUT);
  pinMode(GPIO_DATA_PIN3, OUTPUT);
  pinMode(GPIO_DATA_PIN4, OUTPUT);
}

void loop() {
  binary_counter(); // Exécuter le compteur binaire
}