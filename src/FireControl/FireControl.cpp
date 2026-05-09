#ifndef FIRE_CONTROL_cpp
#define FIRE_CONTROL_cpp

#include <Arduino.h>
#include <DigitalIO.h>

#include "FireControl.h"

#include "RevCore_Flywheeler.h"

const char version[6] = "V0.1";

// quick accesses settings
#define DEBUG_MODE true

// tracking
int selectorPossition = 0;
int currentTriggerState = 0;
int switch_1_reading = 0;
int switch_2_reading = 0;
int switch_3_reading = 0;

// User Settings
int maxFireRate = 15;

// firemode
int selectedFireMode = 1;
fireMode selectableFireModes[3] = {SINGLE_FIRE, BURST_FIRE, AUTO_FIRE};
int selectableBurstAmount[3] = {1, 3, -1};
  
//digital pins
DigitalPin<SWITCH_1> switch1Pin;
DigitalPin<SWITCH_2> switch2Pin;
DigitalPin<SWITCH_3> switch3Pin;
DigitalPin<SELECTOR_SWITCH_1> selectorSwitch1Pin;
DigitalPin<SELECTOR_SWITCH_2> selectorSwitch2Pin;
DigitalPin<SELECTOR_SWITCH_3> selectorSwitch3Pin;
DigitalPin<SELECTOR_SWITCH_4> selectorSwitch4Pin;

void setup() {
  // inputs
  switch1Pin.mode(INPUT_PULLUP);
  switch2Pin.mode(INPUT_PULLUP);
  switch3Pin.mode(INPUT_PULLUP);
  selectorSwitch1Pin.mode(INPUT_PULLUP);
  selectorSwitch2Pin.mode(INPUT_PULLUP);
  selectorSwitch3Pin.mode(INPUT_PULLUP);
  selectorSwitch4Pin.mode(INPUT_PULLUP);
  // outputs
  pinMode(DRIVER_1_ACCELERATE, OUTPUT);
  digitalWrite(DRIVER_1_ACCELERATE, LOW);  // Turn off motor
  pinMode(DRIVER_1_BRAKE, OUTPUT);
  digitalWrite(DRIVER_1_BRAKE, LOW);  // turn off brake

  pinMode(DRIVER_2_ACCELERATE, OUTPUT);
  digitalWrite(DRIVER_2_ACCELERATE, LOW);  // Turn off motor
  pinMode(DRIVER_2_BRAKE, OUTPUT);
  digitalWrite(DRIVER_2_BRAKE, LOW);  // turn off brake

  Serial.begin(9600); // initialize serial communication:

  //inital_setup();
}

void loop() {
  static bool blasterSetup = false;     // setup completion flag
  static bool triggerReleased = true;   // Has the trigger been released after the last shot. Used to force trigger release when needed
  static int burstCount = 0;            // How many shots have been fired 
  // static long lastDevMessage = 0;       // for timing debug messages

  read_selector();
  update_trigger_state();
  blaster_background_task();

  if (!currentTriggerState) {
    triggerReleased = true;
    burstCount = 0;
  }
  if (selectorPossition == SAFETY_SELECTOR_VALUE) { // safty is on
    if (currentTriggerState and !blasterSetup and triggerReleased) {  // do initial setup
      triggerReleased = false;  // Require the trigger to be released
      blasterSetup = blaster_setup();  // set flag
    } else if (currentTriggerState) { // deprime
      blasterSetup = !blaster_teardown();
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

void read_switchs(){
  switch_1_reading = !switch1Pin;
  switch_2_reading = !switch2Pin;
  switch_3_reading = !switch3Pin;
}

void read_selector() {
  if (!selectorSwitch1Pin) {
    selectorPossition = 1;
  } else if (!selectorSwitch2Pin) {
    selectorPossition = 2;
  } else if (!selectorSwitch3Pin) {
    selectorPossition = 3;
  } else if (!selectorSwitch4Pin) {
    selectorPossition = 4;
  } else {
    selectorPossition = 0;
  }
}

/*
Get the current trigger state and sets it to currentTriggerState. 
Uses non blocking debounced to prevent doble fires when trigger is released.
*/
void update_trigger_state() {
  const unsigned long triggerDebounceTime = 2;
  static unsigned long lastTriggerDebounce = 0;
  static int lastTriggerState = 0;
  int reading = !trigger_pin;
  if (reading != lastTriggerState) {
    lastTriggerDebounce = millis();
    lastTriggerState = reading;
  }
  if ((millis() - lastTriggerDebounce) > triggerDebounceTime) {
    currentTriggerState = reading;
  }
}

// Run motor at full speed
void run_driver_1() {
  if (DRIVER_1_USING_BRAKE) {
    digitalWrite(DRIVER_1_BRAKE, LOW);  // brake off
    delay(1);
  }
  digitalWrite(DRIVER_1_ACCELERATE, HIGH);  // motor on
}

void run_driver_2() {
  if (DRIVER_2_USING_BRAKE) {
    digitalWrite(DRIVER_2_BRAKE, LOW);  // brake off
    delay(1);
  }
  digitalWrite(DRIVER_2_ACCELERATE, HIGH);  // motor on
}

// Stop motor
void stop_driver_1() {
  digitalWrite(DRIVER_1_ACCELERATE, LOW);  // motor off
  if (DRIVER_1_USING_BRAKE) {
    delay(1);
    digitalWrite(DRIVER_1_BRAKE, HIGH);  // brake on
  }
}

void stop_driver_2() {
  digitalWrite(DRIVER_2_ACCELERATE, LOW);  // motor off
  if (DRIVER_2_USING_BRAKE) {
    delay(1);
    digitalWrite(DRIVER_2_BRAKE, HIGH);  // brake on
  }
}

// test functions
void test_driver_1() {
  run_driver_1();
  delay(1000);
  stop_driver_1();
  delay(1000);
}

void test_driver_2() {
  run_driver_2();
  delay(1000);
  stop_driver_2();
  delay(1000);
}

#endif