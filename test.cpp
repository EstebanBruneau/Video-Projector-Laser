#include <wiringPi.h>
#include <stdio.h>

#define LED_READ 17
#define LED_WRITE 27

int status=0;

void pinISR (void) {
    status=!status;
    digitalWrite(LED_WRITE, status);
}

int main(void){
    int error = wiringPiSetupGpio();
    if(error == -1){
        printf("Error setting up wiringPi\n");
        return 1;
    }

    pinMode(LED_READ, INPUT);
    pinMode(LED_WRITE, OUTPUT);

    digitalWrite(LED_WRITE, status);

    if(wiringPiISR(LED_READ, INT_EDGE_BOTH, &pinISR)<0){
        printf("Error setting up ISR\n");
        return 1;
    }

    while(1){
        delay(1000);
    }

    return 0;
}