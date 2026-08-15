#ifndef BLASTER_SETUP_cpp
#define BLASTER_SETUP_cpp

#include <Arduino.h>
#include "FlyCore_Flywheeler.h"


bool flywheelsSpinning = false;
int flywheel_speed = 255;
int flywheel_idle_speed = 175;

fireMode selectableFireModes[3] = {SINGLE_FIRE, BURST_FIRE, AUTO_FIRE};
int selectableBurstAmount[3]    = {1,           3,          -1};
int max_fire_rates[3]           = {15,          5,         15};
bool use_idle[3]                = {false,       false,      false};

/*
Anything required to do on initial boot
*/
void inital_setup() {}       

/*
While in safty what to do on trigger pull while blaster is not setup
Return true if blaster should be marked as setup
*/
bool blaster_setup(){
    return true;
}

/*
While in safty what to do on trigger pull while blaster is setup
Return true if blaster should be marked as no longer setup
*/
bool blaster_teardown(){
    return true;
}

/*
Non blocking call to maintain blaster ready state.
Possible uses. Fly wheel reving, temperature checking
*/
void blaster_background_task() {
    if (selectorPosition == SAFE) {
        high_current_driver_move(COAST);
        flywheelsSpinning = false;
    } else if (rev_trigger_reading) {
        high_current_driver_move(FORWARD, flywheel_speed);
        flywheelsSpinning = true;
    } else if (use_idle[selectedFireMode]){
        high_current_driver_move(FORWARD, flywheel_idle_speed);
        flywheelsSpinning = false;
    } else {
        high_current_driver_move(COAST);
        flywheelsSpinning = false;
    }
}

/*
What to do on trigger pull.
Should lead to a single dart being fired.multiple calls will be made if in burst or full
*/
unsigned int fire(){
    static unsigned long startTime = 0;
    static unsigned long tinmeoutTimmer = 0;
    if (!flywheelsSpinning) {   // Make sure fly wheels are moving at speed
        high_current_driver_move(FORWARD, flywheel_speed);
        flywheelsSpinning = true;
        delay(500); // Can be changed based on testing
    }
    startTime = millis();  // Used to track fire rate

    Serial.println("fire");

    bidirection_driver_move(FORWARD, 255);  // start pusher

    // leave rear switch
    tinmeoutTimmer = millis();
    while(pusher_switch_reading){   // make sure motor moves off detector switch
        if ((millis() - tinmeoutTimmer) > PUSHER_TIME_OUT) {
            bidirection_driver_move(BRAKE);
            Serial.println("ERROR: Timeout in run motor");
            error_tone(2);
            return DELAY_ON_FIRE_ERROR;
        }
        delay(1);
        read_switchs(); // reread switches
    }

    // return to rear switch
    tinmeoutTimmer = millis();
    while(!pusher_switch_reading){  // make sure motor moves onto detector switch
        if ((millis() - tinmeoutTimmer) > PUSHER_TIME_OUT) {
            bidirection_driver_move(BRAKE);
            Serial.println("ERROR: Timeout in run motor");
            error_tone(2);
            return DELAY_ON_FIRE_ERROR;
        }
        delay(1);
        read_switchs(); // reread switches
    }
    bidirection_driver_move(BRAKE);  // fast stop pusher

    // check fire rate
    int minLoopTimeMs = 1000/max_fire_rates[selectorPosition];  // time per shot
    int neededDelay = minLoopTimeMs - (millis() - startTime);
    
    if (neededDelay < 0) {  // if needed delay is less than 0. This case is handld in the main but this is a back up
        return 0;
    }
    return neededDelay;
}

#endif