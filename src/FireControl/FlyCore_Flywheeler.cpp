#ifndef BLASTER_SETUP_cpp
#define BLASTER_SETUP_cpp

#include <Arduino.h>
#include <DigitalIO.h>

//#include "FireControl.h"
#include "RevCore_Flywheeler.h"


bool flywheelsSpinning = false;

// anything needed to do on boot
void inital_setup() {}       

// on trigger pull in safety
bool blaster_setup(){
    return true;
}

// on trigger pull in safety after setup
bool blaster_teardown(){
    return true;
}

void blaster_background_task() {
    if (rev_trigger_reading) {
        run_driver_2();
        flywheelsSpinning = true;
    } else {
        stop_driver_2();
        flywheelsSpinning = false;
    }
}

void fire(){
    static unsigned long startTime = 0;
    static unsigned long tinmeoutTimmer = 0;
    if (!flywheelsSpinning) {
        run_driver_2();
        flywheelsSpinning = true;
        delay(200);
    }
    startTime = millis();

    Serial.println("fire");

    run_driver_1();

    // leave rear switch
    tinmeoutTimmer = millis();
    while(pusher_switch_reading){
        if ((millis() - tinmeoutTimmer) > PUSHER_TIME_OUT) {
            stop_driver_1();
            Serial.println("ERROR: Timeout in run motor");
            return;
        }
        delay(1);
        read_switchs();
    }

    // return to rear switch
    tinmeoutTimmer = millis();
    while(!pusher_switch_reading){
        if ((millis() - tinmeoutTimmer) > PUSHER_TIME_OUT) {
            stop_driver_1();
            Serial.println("ERROR: Timeout in run motor");
            return;
        }
        delay(1);
        read_switchs();
    }
    stop_driver_1();

    // insure fire rate
    int minLoopTimeMs = 1000/maxFireRate;
    int neededDelay = minLoopTimeMs - (millis() - startTime);
    Serial.println(neededDelay);
    if (neededDelay > 0) {
        delay(neededDelay);
    }
}

#endif