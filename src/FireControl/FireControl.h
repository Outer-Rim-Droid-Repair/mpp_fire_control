#ifndef FIRE_CONTROL_h
#define FIRE_CONTROL_h

#include "FireControlStructsEnums.h"

// input pins
#define TRIGGER_PIN_NUMBER 8
#define BREACH_PIN_NUMBER 6
#define PLUNGER_PIN_NUMBER 7   
#define SAFETY_PIN_NUMBER 10
#define FIRE_SELECT_PIN_1_NUMBER 16
#define FIRE_SELECT_PIN_2_NUMBER 14
// output pins
#define MOTOR_PIN 9

// input high speed pins
DigitalPin<TRIGGER_PIN_NUMBER> tiggerPin;
DigitalPin<BREACH_PIN_NUMBER> breachPin;
DigitalPin<PLUNGER_PIN_NUMBER> plungerPin;
DigitalPin<SAFETY_PIN_NUMBER> safetyPin;
DigitalPin<FIRE_SELECT_PIN_1_NUMBER> FireModePin1;
DigitalPin<FIRE_SELECT_PIN_2_NUMBER> FireModePin2;

// tracking
volatile int currentTriggerState = LOW;           // Is the trigger pulled? High: pulled, LOW: released           
int lastTriggerState = LOW;                       // History of currentTriggerState 
volatile int currentSensorState = CLOSED_BREACH;  // Internal fire sensors read. Default to CLOSED_BREACH as this should be power down state
volatile int safetyState = LOW;                   // Is the Safty Enabled? High: Enabled, LOW: Disabled  
firingStates nextState;                                 // Firing State machine state. 

// User controlled settings.
idleMode idlePossition = PRIMED_IDLE;             // where should the firing cycle end?
// Could make this a by mode option. i.e. user could select burst 2 and burst 4 as their fire modes
int maxDPS = 5;                                  // limit on number of darts per second

// firemode
int selectedFireMode;
fireMode selectableFireModes[3] = {SINGLE_FIRE, BURST_FIRE, AUTO_FIRE};
int selectableBurstAmount[3] = {1, 3, -1};


// functions
void fire();
void fireStateMachine();
bool waitTillSensorChange(int initial_state);
void _getAllSensorStatesBut(int *list, int state);
bool waitTillSensorChangeToValue(int target_state);
bool waitTillSensorChangeToValue(int target_states[], int length);
bool isValueInList(int value, int list[], int length);
void run_motor();
void stop_motor();
void update_sensor_state();
void update_trigger_state();
void update_safety_state();
void update_fire_mode();

void fillStatus();
void fillIdentifier();
void setSettings();
void fillSettings();

void dev_write_serial_all_states();

#endif