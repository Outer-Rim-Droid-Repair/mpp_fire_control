#ifndef BLASTER_SETUP_H
#define BLASTER_SETUP_H

#include "Arduino.h"
#include "FireControl.h"

// required defines
#define trigger_pin switch2Pin
#define SAFETY_SELECTOR_VALUE 1

#define DRIVER_1_USING_BRAKE true
#define DRIVER_2_USING_BRAKE false

// type specific defines
#define rev_trigger_reading switch_1_reading
#define pusher_switch_reading switch_3_reading

#define PUSHER_TIME_OUT 100

extern bool flywheelsSpinning;

// required functions
void inital_setup();        // anything needed to do on boot
bool blaster_setup();       // on trigger pull in safety
bool blaster_teardown();    // on trigger pull in safety after setup
void fire();
void blaster_background_task();  // a call to a non blocking function to do background tasks

// type specific function

#endif