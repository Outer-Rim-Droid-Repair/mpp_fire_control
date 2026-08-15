#ifndef FIRE_CONTROL_h
#define FIRE_CONTROL_h

#include "FireControlStructsEnums.h"

// ----------- General controlboard -----------
// =========== Real Pins ===========
// Motor Pins
#define DRIVER_BI_IN1 9
#define DRIVER_BI_IN2 10

#define DRIVER_HIGH_IN 6

// Other Pins
#define SENSOR1 A0
#define SENSOR2 A1

#define BUZZER_Pin 5 


// =========== Virtual Pins ===========
// IO Expander Pin Number
#define SWITCH1 0  
#define SWITCH2 1  
#define SWITCH3 2 
#define SWITCH4 3 

#define SELECTOR1 7
#define SELECTOR2 6
#define SELECTOR3 5
#define SELECTOR4 4

// ----------- enums -----------
enum selector_positions {
    SAFE = -1,
    POS_1 = 0,
    POS_2 = 1,
    POS_3 = 2,
    INVALID = -2
};

enum driver_direction {
    COAST = 0,
    FORWARD = 1,
    BACKWARDS = 0,
    BRAKE = -10
};

// ----------- tracking -----------
extern int currentTriggerState;
extern int switch_1_reading;
extern int switch_2_reading;
extern int switch_3_reading;
extern int switch_4_reading;

extern selector_positions selectorPosition;
extern int selectedFireMode;
extern fireMode selectableFireModes[3];
extern int selectableBurstAmount[3];


// ----------- User Settings -----------

// ----------- Functions -----------
void read_switchs();
void read_selector();
void update_trigger_state();

void error_tone(int times_to_play);

// =========== Motor Functions ===========
void bidirection_driver_move(driver_direction action, int speed = 0);
void high_current_driver_move(driver_direction action, int speed = 0);

// =========== Debug Functions ===========
void print_states();

#endif