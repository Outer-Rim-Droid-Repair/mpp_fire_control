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
#define DRIVER_2_ACCELERATE 6
#define DRIVER_2_BRAKE 7

#define SAFETY_SELECTOR_VALUE 1

//enums
enum selector_positions {
    SAFE = -1,
    POS_1 = 0,
    POS_2 = 1,
    POS_3 = 2,
    INVALID = -2
};

// tracking
extern int currentTriggerState;
extern int switch_1_reading;
extern int switch_2_reading;
extern int switch_3_reading;

extern selector_positions selectorPosition;
extern int selectedFireMode;
extern fireMode selectableFireModes[3];
extern int selectableBurstAmount[3];


// User Settings
extern int maxFireRate;

// functions
void read_switchs();
void read_selector();
void update_trigger_state();

void full_speed_driver(int driver);
void stop_driver(int driver);
void run_driver(int driver, int speed);

// dev functions
// void dev_write_serial_all_states();

// test functions
void test_driver_1();
void test_driver_2();


#endif