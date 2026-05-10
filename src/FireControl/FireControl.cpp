#ifndef FIRE_CONTROL_cpp
#define FIRE_CONTROL_cpp

#include <Arduino.h>
#include <DigitalIO.h>

#include "FireControl.h"

#include "FlyCore_Flywheeler.h"

const char version[6] = "V0.1";

// quick accesses settings
#define DEBUG_MODE true

// tracking
int currentTriggerState = 0;
int switch_1_reading = 0;
int switch_2_reading = 0;
int switch_3_reading = 0;

// User Settings
int maxFireRate = 10;

// firemode
selector_positions selectorPosition = SAFE;
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

  inital_setup();
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
  if (selectorPosition == SAFE) { // safty is on
    if (currentTriggerState and !blasterSetup and triggerReleased) {  // do initial setup
      triggerReleased = false;  // Require the trigger to be released
      blasterSetup = blaster_setup();  // set flag
    } else if (currentTriggerState) { // deprime
      blasterSetup = !blaster_teardown();
    }
  } else if (currentTriggerState and triggerReleased) {  // fire next dart
    // if blaster not set up pullingthe trigger will do a similar process
    blasterSetup = true;  // set flag
    switch (selectableFireModes[selectorPosition]) {
      case SINGLE_FIRE:
        fire();
        triggerReleased = false;  // Require the trigger to be released
        break;
      case BURST_FIRE:
        fire();
        burstCount += 1;  // increase fire count
        if (burstCount >= selectableBurstAmount[selectorPosition]) { // once burst limit has been reached
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
  if (!selectorSwitch1Pin) {  // safe
    selectorPosition = SAFE;
  } else if (!selectorSwitch2Pin) { 
    selectorPosition = POS_1;
  } else if (!selectorSwitch3Pin) {
    selectorPosition = POS_2;
  } else if (!selectorSwitch4Pin) {
    selectorPosition = POS_3;
  } else {
    selectorPosition = INVALID;
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
void full_speed_driver(int driver) {
  run_driver(driver, 255);
}

void stop_driver(int driver) {
  run_driver(driver, 0);
}

void run_driver(int driver, int speed) {
  bool using_brake;
  int brake_pin;
  int drive_pin;

  if (driver == 1){
    using_brake = DRIVER_1_USING_BRAKE;
    brake_pin = DRIVER_1_BRAKE;
    drive_pin = DRIVER_1_ACCELERATE;
  } else if (driver == 2){
    using_brake = DRIVER_2_USING_BRAKE;
    brake_pin = DRIVER_2_BRAKE;
    drive_pin = DRIVER_2_ACCELERATE;
  }
  else {
    return;
  }
  if (speed == 0){
    digitalWrite(drive_pin, LOW);
    if (using_brake) {
      digitalWrite(brake_pin, HIGH);  // brake off
      delay(1);
    }
  } else {
    if (using_brake) {
      digitalWrite(brake_pin, LOW);  // brake off
      delay(1);
    }
    analogWrite(drive_pin, speed);
  }

  
}

// test functions
void test_driver_1() {
  full_speed_driver(1);
  delay(1000);
  full_speed_driver(1);
  delay(1000);
}

void test_driver_2() {
  full_speed_driver(2);
  delay(1000);
  full_speed_driver(2);
  delay(1000);
}

#endif