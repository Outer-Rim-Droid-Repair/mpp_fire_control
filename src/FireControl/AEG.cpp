#ifndef BLASTER_SETUP_cpp
#define BLASTER_SETUP_cpp

#include <Arduino.h>
#include "AEG.h"


fireMode selectableFireModes[3] = {SINGLE_FIRE, BURST_FIRE, AUTO_FIRE};
int selectableBurstAmount[3]    = {1,           3,          -1};
int max_fire_rates[3]           = {15,          5,         15};
bool use_idle[3]                = {false,       false,      false};
idleMode idlePossition = PRIMED_IDLE;
int currentSensorState = 0;

/*
Anything required to do on initial boot
*/
void inital_setup() {

}       

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
void blaster_background_task() {}

/*
What to do on trigger pull.
Should lead to a single dart being fired.multiple calls will be made if in burst or full
*/
unsigned int fire(){
    static unsigned long startTime = 0;
    startTime = millis();  // Used to track fire rate
    Serial.println("-------------------- Firing --------------------");

    static sensorState startStopPossition;
    // get the expected startstop possition
    if (idlePossition == DEPRIMED_IDLE) {
        startStopPossition = CLOSED_BREACH;
    } else if (idlePossition == PRIMED_IDLE) {
        startStopPossition = FIRE_READY;
    }

    static bool running_loop;
    static firingStates nextState;
    running_loop = true;
    nextState = LEAVING_STARTING_POSSITION;  // default state machine
    while(running_loop) { // loop till firing complete.
        Serial.println(stateMachineStr[nextState]);
        switch (nextState) {
            case LEAVING_STARTING_POSSITION:
            // mach sure the motor drives off the expected startstop possition
            {
                // select next state based on what the idle mode is
                if (idlePossition == DEPRIMED_IDLE) {
                    nextState = CYCLE_TO_DEPRIMED;
                } else if (idlePossition == PRIMED_IDLE) {
                    nextState = CYCLE_TO_PRIMED;
                } else {  // invalid idle possition
                    Serial.println("Error invalid idle possition");
                    nextState = ERROR_STATE;
                    break;
                }
                run_motor();
                if (!waitTillSensorChange(startStopPossition)) { // make sure motor drives off of base possition
                    Serial.println("Error from leaving start possition");
                    nextState = ERROR_STATE;
                    break;
                }
                break;
            }
            case CYCLE_TO_PRIMED:
            // drives motor till system in in the FIRE_READY sensorState
            {
                run_motor();
                // it is possible to have the plunger reach the back before chamber full closes
                int valid_states[2] = {FIRE_READY, PRIMED};
                if (!waitTillSensorChangeToValue(valid_states, 2)) { // drive till end possition
                    Serial.println("Error from CYCLE_TO_PRIMED");
                    nextState = ERROR_STATE;
                    break;
                }
                stop_motor();
                
                // make sure breach is closed
                if (!waitTillSensorChangeToValue(FIRE_READY)) { // wait till fire ready
                    Serial.println("Error from CYCLE_TO_PRIMED waiting for fire ready");
                    nextState = ERROR_STATE;
                    break;
                } 
                for (int i=0; i<100; i++){  // For Debugging TODO Remove
                    update_sensor_state();
                    Serial.print(i);
                    Serial.print(" : ");          
                    Serial.println(sensorStateStr[currentSensorState]);
                    delay(1);
                }

                nextState = COMPLETE_STATE;
                break;
            }
            case CYCLE_TO_DEPRIMED:
            // drives motor till system in in the CLOSED_BREACH sensorState
            {
                run_motor();
                if (!waitTillSensorChangeToValue(CLOSED_BREACH)) { // drive till end possition
                    Serial.println("Error from CYCLE_TO_DEPRIMED");
                    nextState = ERROR_STATE;
                    break;
                }
                stop_motor();
                nextState = COMPLETE_STATE;
                break;
            }
            case COMPLETE_STATE:
            {
                // fireing complete
                stop_motor();
                running_loop = false;
                nextState = COMPLETE_STATE;
                break;
            }
            case ERROR_STATE:
            {
                //something went wrong
                stop_motor();
                error_tone(2);
                Serial.println("Error State");
                read_selector();
                update_trigger_state();
                read_switchs();
                running_loop = false;
                break;
            }
            default:
            {
                // should never happen
                Serial.println("Default state?");
                stop_motor();
                error_tone(2);
                running_loop = false;
                break;
            }
        }
    }
    // check fire rate
    int minLoopTimeMs = 1000/max_fire_rates[selectorPosition];  // time per shot
    int neededDelay = minLoopTimeMs - (millis() - startTime);
    
    if (neededDelay < 0) {  // if needed delay is less than 0. This case is handld in the main but this is a back up
        return 0;
    }
    //  neededDelay;
    //error_tone(1);
    return 500;
}

/* 
Wrapper for waitTillSensorChangeToValue for when there is a sensor state to chacge out of.
initial_state: valid sensorState to look for leave.
*/
bool waitTillSensorChange(int initial_state) {
    static int list[3];
    _getAllSensorStatesBut(list, initial_state);
    return waitTillSensorChangeToValue(list, 3);
}

/*
Support funtion for waitTillSensorChange. Gets a list of the valid sensorState except for one.
*list: list to write into
state:  valid sensorState to exclude
*/
void _getAllSensorStatesBut(int *list, int state) {
    int i = 0;
    if (MID_CYCLE != state) {
        list[i] = MID_CYCLE;
        i++;
    }
    if (PRIMED != state) {
        list[i] = PRIMED;
        i++;
    }
    if (CLOSED_BREACH != state) {
        list[i] = CLOSED_BREACH;
        i++;
    }
    if (FIRE_READY != state) {
        list[i] = FIRE_READY;
        i++;
    }
}

/* 
Wrapper for waitTillSensorChangeToValue for when there is only a single target sensor state
target_state: valid sensorState to look for.
*/
bool waitTillSensorChangeToValue(int target_state) {
    int list[] = {target_state};
    return waitTillSensorChangeToValue(list, 1);
}

/*
Reads the current sensor state and waits till one of the target states is reached. This function is blocking.
target_states[]: list of valid sensorState to look for.
length: length of target_states[].
*/
bool waitTillSensorChangeToValue(int target_states[], int length) {
    // blocking
    static unsigned long total_wait_time;
    static unsigned long wait_debounce_time;

    total_wait_time = millis(); // start timeout timer
    wait_debounce_time = micros(); // start time of debounce

        while (true) {
            update_sensor_state();  // get newest reading

            if (!isValueInList(currentSensorState, target_states, length)) { 
                // if the currentSensorState is not in target_states
                wait_debounce_time = micros();  // reset the debouncing timer
            }

            if ((micros() - wait_debounce_time) > 100) {  //uSec
                // whatever the reading is at, it's been there for longer than the debounce delay
                if (isValueInList(currentSensorState, target_states, length)) {
                    // if the currentSensorState is in target_states
                    update_sensor_state();
                    return true;
                }     
            }
            if ((millis() - total_wait_time) > Possition_change_timeout) { 
                // if we have been in this loop for too long break
                // 200mSec is one cycle time at 5 dps.
                return false;
        }
    }
    return false;
}

/*
Goes through int[] and returns true if the value is in the list.
value: the valuse to seach for.
list[]: list to search.
length: length of list.
*/
bool isValueInList(int value, int list[], int length) {
    for ( int i = 0; i < length; ++i ) {
        if (list[i] == value) {
            return true;
        }
    }
    return false;
}

/*
Get internal switch reading and set it to currentSensorState. 
Use sensorState enum to access by name. 
format: BREACH_PIN,PLUNGER_PIN
*/
void update_sensor_state() {
    read_switchs();
    currentSensorState = 2 * (switch_2_reading) + (switch_3_reading);
}

// Run motor at full speed
void run_motor() {
    Serial.println("running motor");
}

// Stop motor
void stop_motor() {
    Serial.println("stopped motor");
}

#endif