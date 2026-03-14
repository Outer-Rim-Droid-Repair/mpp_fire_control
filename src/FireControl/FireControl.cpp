#include <Arduino.h>
#include <DigitalIO.h>

#include "FireControl.h"
#include "MEDIC_Comms/MEDIC_Comms.h"

const char version[6] = "V0.1";

MEDIC_FIRE_CONTROL_RECEIVER communicator;

// quick accesses settings
#define DEBUG_MODE true

void setup() {
  // inputs
  tiggerPin.mode(INPUT_PULLUP);
  breachPin.mode(INPUT_PULLUP);
  plungerPin.mode(INPUT_PULLUP);
  safetyPin.mode(INPUT_PULLUP);
  FireModePin1.mode(INPUT_PULLUP);
  FireModePin2.mode(INPUT_PULLUP);
  // outputs
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);  // Turn off motor
  
  pinMode(BRAKE_PIN, OUTPUT);
  digitalWrite(BRAKE_PIN, LOW);  // turn off brake


  stop_motor(); // turn off motor on boot

  Serial.begin(9600); // initialize serial communication:

  communicator = MEDIC_FIRE_CONTROL_RECEIVER();
  communicator.connectOnRequestIdentifyFunction(fillIdentifier);
  communicator.connectOnRequestSettingsFunction(fillSettings);
  communicator.connectSetSettingFunction(setSettings);
  communicator.connectOnRequestStatusFunction(fillStatus);
  communicator.begin();
}

void loop() {
  static bool blasterSetup = false;     // setup completion flag
  static bool triggerReleased = true;   // Has the trigger been released after the last shot. Used to force trigger release when needed
  static int burstCount = 0;            // How many shots have been fired 
  static long lastDevMessage = 0;       // for timing debug messages


  if (DEBUG_MODE) {  // if debug print states
    if (millis() - lastDevMessage >= 5*1000UL) // Only write once every 2 seconds
    {
      lastDevMessage = millis();      // Update timer
      dev_write_serial_all_states();
    }
  }

  // update all readings
  update_trigger_state();
  update_sensor_state();
  update_safety_state();
  update_fire_mode();
  
  // if trigger has been released clear variables
  if (!currentTriggerState) {
    triggerReleased = true;
    burstCount = 0;
  }
  if (safetyState) { // safty is on
    if (currentTriggerState and !blasterSetup and triggerReleased) {  // do initial setup
      triggerReleased = false;  // Require the trigger to be released
      if (idlePossition == DEPRIMED_IDLE) {  // ready possition is 1,0: CLOSED_BREACH
        run_motor();
        if (!waitTillSensorChangeToValue(CLOSED_BREACH)) {
          Serial.println("Error in blaster setup");
          stop_motor();
          return;
        }        
        stop_motor();
      } else if (idlePossition == PRIMED_IDLE) { // ready possition is 1,1: FIRE_READY
        Serial.println("blaster setup");
        run_motor();
        // there is a timing where the plunger isin the back possition before breach fully closes
        int valid_states[2] = {FIRE_READY, PRIMED};  
        if (!waitTillSensorChangeToValue(valid_states, 2)) {
          Serial.println("Error in blaster setup");
          stop_motor();
          return;
        }        
        stop_motor();
      } else {  // unknown idle possition
        Serial.println("invalid idle possition");
        return;
      }
      blasterSetup = true;  // set flag
    } else if (currentTriggerState) { // deprime
     static long triggerHoldTime = 0;  // Used for timeing how long the trigger has been held down
      if (triggerHoldTime == 0) {  // first time through after trigger pull
        if (triggerReleased) {
          Serial.println("starting trigger time");
          triggerReleased = false;
          triggerHoldTime = millis();
        }
      } else {  //trigger is being held down
        if ((millis() - triggerHoldTime) > 5000) {  // require 5 second hold to deprime
          if (currentSensorState != CLOSED_BREACH) {
            run_motor();
            if (!waitTillSensorChangeToValue(CLOSED_BREACH)) {
              Serial.println("Error in blaster setup");
            }  
            stop_motor();
          }
          blasterSetup = false;
          triggerHoldTime = 0;
        }
      }
    }
  } else if (currentTriggerState and triggerReleased) {  // fire next dart
    // if blaster not set up pullingthe trigger will do a similar process
    blasterSetup = true;  // set flag
    switch (selectableFireModes[selectedFireMode]) {
      case SINGLE_FIRE:
        fire();
        triggerReleased = false;  // Require the trigger to be released
        break;
      case BURST_FIRE:
        fire();
        burstCount += 1;  // increase fire count
        if (burstCount >= selectableBurstAmount[selectedFireMode]) { // once burst limit has been reached
          triggerReleased = false; // Require the trigger to be released
        }
        break;
      case AUTO_FIRE:
        fire();
        break;
    }
  } 
}

/*
Fire the Blaster. This is responsable for managing fire rate limit
*/
void fire() {
  // default state machine to expected start location.
  static unsigned long fireStart;
  unsigned long fireStop;
  Serial.println("-------------------- Firing --------------------");
  fireStart = millis(); // start fire rate timmer   
  fireStateMachine(); // go through firing process
  fireStop = millis();  // stop fire timer
  //               max fire rate - time firing
  int delaytime = (1000/maxDPS) - (fireStop - fireStart);
  Serial.println(delaytime);
  if (delaytime < 50) { // if delay too low
    delaytime = 50;
  }
  delay(delaytime);
}

/*
Runs through the firing process. This is blocking.
*/
void fireStateMachine() {
  static sensorState startStopPossition;
  // get the expected startstop possition
  if (idlePossition == DEPRIMED_IDLE) {
    startStopPossition = CLOSED_BREACH;
  } else if (idlePossition == PRIMED_IDLE) {
    startStopPossition = FIRE_READY;
  }

  nextState = LEAVING_STARTING_POSSITION;  // default state machine
  while(true) { // loop till firing complete.
    if (DEBUG_MODE) { // if debug print states
      update_sensor_state();
      dev_write_serial_all_states();
    }
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
        //analogWrite(MOTOR_PIN, 0);
        /*
        // make sure breach is closed
        if (!waitTillSensorChangeToValue(FIRE_READY)) { // wait till fire ready
          Serial.println("Error from CYCLE_TO_PRIMED waiting for fire ready");
          nextState = ERROR_STATE;
          break;
        } */
        /*for (int i=0; i<100; i++){
          update_sensor_state();
          Serial.print(i);
          Serial.print(" : ");          
          Serial.println(sensorStateStr[currentSensorState]);
          delay(1);
        }*/

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
        return;
      }
      case ERROR_STATE:
      {
        //something went wrong
        // TODO inform user
        stop_motor();
        Serial.println("Error State");
        update_trigger_state();
        update_sensor_state();
        update_safety_state();
        update_fire_mode();
        dev_write_serial_all_states();
        return;
      }
      default:
      {
        // should never happen
        // TODO infor user
        stop_motor();
        return;
      }
    }
  }
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
void _getAllSensorStatesBut(int *list, int state){
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
    if ((millis() - total_wait_time) > 300) { 
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

// Run motor at full speed
void run_motor() {
  Serial.println("running motor");
  //analogWrite(MOTOR_PIN, 255);
  if (USING_BRAKE) {
    digitalWrite(BRAKE_PIN, LOW);  // brake off
    delay(1);

  }
  digitalWrite(MOTOR_PIN, HIGH);  // motor on
}

// Stop motor
void stop_motor() {
  //analogWrite(MOTOR_PIN, 0);
  digitalWrite(MOTOR_PIN, LOW);  // motor off
  if (USING_BRAKE) {
    delay(1);
    digitalWrite(BRAKE_PIN, HIGH);  // brake on
  }

  Serial.println("stopped motor");
}

/*
Get internal switch reading and set it to currentSensorState. 
Use sensorState enum to access by name. 
format: BREACH_PIN,PLUNGER_PIN
*/
void update_sensor_state() {
  currentSensorState = 2 * (!breachPin) + (!plungerPin);
}

/*
Get the current trigger state and sets it to currentTriggerState. 
Uses none blocking debounced to prevent doble fires when trigger is released.
*/
void update_trigger_state() {
  const unsigned long triggerDebounceTime = 2;
  static unsigned long lastTriggerDebounce = 0;
  int reading = !tiggerPin;
  if (reading != lastTriggerState) {
    lastTriggerDebounce = millis();
    lastTriggerState = reading;
  }
  if ((millis() - lastTriggerDebounce) > triggerDebounceTime) {
    lastTriggerState = currentTriggerState;
    currentTriggerState = reading;
  }
}

/*
Get safty switch state and sets it to safetyState.
*/
void update_safety_state() {
  safetyState = safetyPin;
}

/*
Get fire mode option fromthe selector switch and sets it to selectedFireMode.
Use fireMode enum to access by name. 
*/
void update_fire_mode() {
  // TODO update with selecting fire mode from a user selected array
  if (!FireModePin1) {  // first possition
    selectedFireMode = 0;
  } else if (!FireModePin2) { // third possition
    selectedFireMode = 2;
  } else {  // second possition
    selectedFireMode = 1;
  }
}

void fillStatus() {
  communicator.statusStruct.FireMode = selectableFireModes[selectedFireMode];
  communicator.statusStruct.BurstAmount = selectableBurstAmount[selectedFireMode];
  communicator.statusStruct.safteyState = safetyState;
  communicator.statusStruct.triggerState = currentTriggerState;
}

void fillIdentifier() {
  strcpy(communicator.identifyStruct.version, version);
}

void setSettings() {
  memcpy(&selectableFireModes, &communicator.settingStruct.selectableFireModes[0], sizeof(selectableFireModes));
  memcpy(&selectableBurstAmount, &communicator.settingStruct.selectableFireModes[0], sizeof(selectableBurstAmount));
  maxDPS = communicator.settingStruct.maxFireRate;
  idlePossition = (idleMode) communicator.settingStruct.idlePossition;
}

void fillSettings() {
  // convert firemode to int this should get changed back. see TODO in MEDIC_Comms
  unsigned int modes[3];
  for (unsigned int i = 0; i < 3; i++){
    modes[i] = (unsigned int) selectableFireModes;
  }
  memcpy(&communicator.settingStruct.selectableFireModes, &modes[0], sizeof(communicator.settingStruct.selectableFireModes));
  memcpy(&communicator.settingStruct.selectableBurstAmounts, &selectableBurstAmount[0], sizeof(communicator.settingStruct.selectableBurstAmounts));
  communicator.settingStruct.maxFireRate = maxDPS;
  communicator.settingStruct.idlePossition = (int) idlePossition;
}


void dev_write_serial_all_states() {
  /* Write variables to serial port */
  Serial.print("System State: ");
  Serial.print("    Safety: ");
  Serial.print(safetyState);
  Serial.print("    Trigger: ");
  Serial.print(currentTriggerState);
  Serial.print("    Fire Mode: ");
  Serial.print(fireModeStr[selectableFireModes[selectedFireMode]]);
  Serial.print("    Loading State: ");
  Serial.print(currentSensorState);
  Serial.print(": ");
  Serial.print(sensorStateStr[currentSensorState]);
  Serial.print("    State: ");
  Serial.print(stateMachineStr[nextState]);
  Serial.println("");
}

