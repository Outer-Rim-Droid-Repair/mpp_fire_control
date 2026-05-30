#ifndef BLASTER_SETUP_cpp
#define BLASTER_SETUP_cpp

#include <Arduino.h>

//#include "FireControl.h"
#include "FlyCore_Flywheeler.h"


bool flywheelsSpinning = false;
int flywheel_speed = 255;
int flywheel_idle_speed = 175;

//int maxFireRate = 15;

fireMode selectableFireModes[3] = {SINGLE_FIRE, BURST_FIRE, AUTO_FIRE};
int selectableBurstAmount[3]    = {1,           3,          -1};
int max_fire_rates[3]           = {15,          15,         15};
bool use_idle[3]                = {false,       false,      false};

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
    if (selectorPosition == SAFE) {
        driver_coast(flywheel_driver);
        flywheelsSpinning = false;
    } else if (rev_trigger_reading) {
        driver_forward(flywheel_driver, flywheel_speed);
        flywheelsSpinning = true;
    } else if (use_idle[selectedFireMode]){
        driver_forward(flywheel_driver, flywheel_idle_speed);
        flywheelsSpinning = false;
    } else {
        driver_coast(flywheel_driver);
        flywheelsSpinning = false;
    }
}

void fire(){
    static unsigned long startTime = 0;
    static unsigned long tinmeoutTimmer = 0;
    if (!flywheelsSpinning) {
        driver_forward(flywheel_driver, flywheel_speed);
        flywheelsSpinning = true;
        delay(500);
    }
    startTime = millis();

    Serial.println("fire");

    driver_forward(pusher_driver);

    // leave rear switch
    tinmeoutTimmer = millis();
    while(pusher_switch_reading){
        if ((millis() - tinmeoutTimmer) > PUSHER_TIME_OUT) {
            driver_brake(pusher_driver);
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
            driver_brake(pusher_driver);
            Serial.println("ERROR: Timeout in run motor");
            return;
        }
        delay(1);
        read_switchs();
    }
    driver_brake(pusher_driver);

    // insure fire rate
    int minLoopTimeMs = 1000/max_fire_rates[selectedFireMode];
    int neededDelay = minLoopTimeMs - (millis() - startTime);
    Serial.println(neededDelay);
    if (neededDelay > 0) {
        delay(neededDelay);
    }
}

#endif