#ifndef FIRE_CONTROL_h
#define FIRE_CONTROL_h

#include "FireControlStructsEnums.h"

//general controlboard
// input pins
#define SWITCH_1 14
#define SWITCH_2 16
#define SWITCH_3 10  
#define SELECTOR_SWITCH_1 2
#define SELECTOR_SWITCH_2 3
#define SELECTOR_SWITCH_3 4
#define SELECTOR_SWITCH_4 5

// output pins
#define DRIVER_1_ACCELERATE 8
#define DRIVER_1_BRAKE 9
#define DRIVER_2_ACCELERATE 7
#define DRIVER_2_BRAKE 6



//#define trigger_pin switch2Pin

// tracking
extern int selectorPossition;
extern int currentTriggerState;
extern int switch_1_reading;
extern int switch_2_reading;
extern int switch_3_reading;

// User Settings
extern int maxFireRate;

// functions
void read_switchs();
void read_selector();
void update_trigger_state();
void run_driver_1();
void run_driver_2();
void stop_driver_1();
void stop_driver_2();

// dev functions
// void dev_write_serial_all_states();

// test functions
void test_driver_1();
void test_driver_2();


#endif