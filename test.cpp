// #include <vector>
#include <wiringPi.h>
#include <stdio.h>

#define LED_PIN 29

int status = 0;

/*
régler pin mode up et down
simuler signal carré fréquence à def
mesurer le temps de réponse
utiliser un interrupt connecté à un pin
il va déclencher une fonction qui active un pin (allume ou éteind (passe de 0 à 1 ou de 1 à 0
stocker la valeur des pin dans des variables))
definir l'interrupt (sur quel Pin)
wiringpi ISR
*/

// std::vector<std::vector<int,int>> list_etat;

void changement_etat(){
    status = 1;
    if(digitalRead(LED_PIN) == 0){
        digitalWrite(LED_PIN, 1);
    }else{
        digitalWrite(LED_PIN, 0);
    }
}

void myInterrupt0 (void) {
    printf("Interrupt 0\n");
    changement_etat();
    return;
}

void myInterrupt1 (void) {
    printf("Interrupt 1\n");
    changement_etat();
    return;
}

int main(void){
    int error = wiringPiSetupGpio();
    if(error == -1){
        printf("Error setting up wiringPi\n");
        return 1;
    }
    printf("WiringPi setup\n");
    wiringPiSetup () ;

    wiringPiISR (0, INT_EDGE_RISING, &myInterrupt1) ;
    // wiringPiISR (0, INT_EDGE_FALLING, &myInterrupt0) ;


    while(1){
        if status == 1{
            printf("Interrupt 0\n");
            break
        }
    }

    
    return 0;
}