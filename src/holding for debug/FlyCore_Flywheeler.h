#ifndef BLASTER_SETUP_H
#define BLASTER_SETUP_H

#include "Arduino.h"
#include "FireControl.h"

// ----------- required defines -----------
#define TRIGGER SWITCH2

// ----------- type specific defines -----------
#define rev_trigger_reading switch_1_reading
#define pusher_switch_reading switch_3_reading
#define pusher_driver 2
#define flywheel_driver 1

#define PUSHER_TIME_OUT 5000
#define DELAY_ON_FIRE_ERROR 1

// ----------- Tracking -----------
extern bool flywheelsSpinning;
extern int flywheel_speed;
extern int flywheel_idle_speed;

// ----------- required functions -----------
void inital_setup();        // anything needed to do on boot
bool blaster_setup();       // on trigger pull in safety
bool blaster_teardown();    // on trigger pull in safety after setup
unsigned int fire();
void blaster_background_task();  // a call to a non blocking function to do background tasks

// ----------- type specific function -----------

#endif