#ifndef BLASTER_SETUP_H
#define BLASTER_SETUP_H

#include "Arduino.h"
#include "FireControl.h"

// ----------- required defines -----------
#define TRIGGER SWITCH1

// ----------- type specific defines -----------
#define breach_switch_reading switch_2_reading
#define plunger_switch_reading switch_3_reading
#define Possition_change_timeout 300

// ----------- Tracking -----------


// ----------- required functions -----------
void inital_setup();        // anything needed to do on boot
bool blaster_setup();       // on trigger pull in safety
bool blaster_teardown();    // on trigger pull in safety after setup
unsigned int fire();
void blaster_background_task();  // a call to a non blocking function to do background tasks

// ----------- type specific function -----------
bool waitTillSensorChange(int initial_state);
void _getAllSensorStatesBut(int *list, int state);
bool waitTillSensorChangeToValue(int target_state);
bool waitTillSensorChangeToValue(int target_states[], int length);
bool isValueInList(int value, int list[], int length);
void update_sensor_state();
void run_motor();
void stop_motor();

#endif