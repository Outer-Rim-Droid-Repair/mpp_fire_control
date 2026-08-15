#ifndef FIRE_CONTROL_cpp
#define FIRE_CONTROL_cpp

#include <Arduino.h>
#include <Adafruit_MCP23X08.h>

#include "FireControl.h"

#include "AEG.h"

const char version[6] = "V0.3";

// quick accesses settings
#define DEBUG_MODE false
#define DEBUG_PERIOD 0.1  // Time in seconds between debug messages

// tracking
int currentTriggerState = 0;
int switch_1_reading = 0;
int switch_2_reading = 0;
int switch_3_reading = 0;
int switch_4_reading = 0;
int switch_5_reading = 0;

// User Settings


// firemode
selector_positions selectorPosition = SAFE;
int selectedFireMode = 1;

// objects
Adafruit_MCP23X08 io_expander = Adafruit_MCP23X08();


void setup() {
  // IO Expander
  if (!io_expander.begin_I2C()) {
    Serial.println("Error.");
    while (1);
  }

  // inputs
  io_expander.pinMode(SWITCH1, INPUT_PULLUP);
  io_expander.pinMode(SWITCH2, INPUT_PULLUP);
  io_expander.pinMode(SWITCH3, INPUT_PULLUP);
  io_expander.pinMode(SWITCH4, INPUT_PULLUP);
  io_expander.pinMode(SELECTOR1, INPUT_PULLUP);
  io_expander.pinMode(SELECTOR2, INPUT_PULLUP);
  io_expander.pinMode(SELECTOR3, INPUT_PULLUP);
  io_expander.pinMode(SELECTOR4, INPUT_PULLUP);

  // outputs
  pinMode(DRIVER_BI_IN1, OUTPUT);
  pinMode(DRIVER_BI_IN2, OUTPUT);
  bidirection_driver_move(COAST);

  pinMode(DRIVER_HIGH_IN, OUTPUT);
  high_current_driver_move(COAST);

  Serial.begin(9600); // initialize serial communication:

  inital_setup();
  //while (!Serial);
  delay(1000);
  Serial.println("Running");
}

void loop() {
  static bool blasterSetup = false;     // setup completion flag
  static bool triggerReleased = true;   // Has the trigger been released after the last shot. Used to force trigger release when needed
  static int burstCount = 0;            // How many shots have been fired 
  static long lastDevMessage = 0;       // for timing debug messages
  static unsigned long nextFireTime = 0;

  read_selector();
  update_trigger_state();
  read_switchs();

  if (DEBUG_MODE) {
    if (millis() - lastDevMessage >= DEBUG_PERIOD*1000UL) {
      lastDevMessage = millis();      // Update timer
      print_states();
    }
  }

  blaster_background_task();

  if (!currentTriggerState) {  // Check if trigger has been released
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
  } else if (millis() < nextFireTime) { // wait for fire rate limitor
    delay(1);
  } else if (currentTriggerState and triggerReleased) {  // fire next dart
    static unsigned int neededFireDelay;
    // if blaster not set up pullingthe trigger will do a similar process
    blasterSetup = true;  // set flag
    switch (selectableFireModes[selectorPosition]) {
      case SINGLE_FIRE:
        neededFireDelay = fire();
        triggerReleased = false;  // Require the trigger to be released
        break;
      case BURST_FIRE:
        neededFireDelay = fire();
        burstCount += 1;  // increase fire count
        if (burstCount >= selectableBurstAmount[selectorPosition]) { // once burst limit has been reached
          triggerReleased = false; // Require the trigger to be released
        }
        break;
      case AUTO_FIRE:
        neededFireDelay = fire();
        break;
    }
    nextFireTime = millis() + neededFireDelay;
  }
}

void read_switchs(){
  switch_1_reading = !io_expander.digitalRead(SWITCH1);
  switch_2_reading = !io_expander.digitalRead(SWITCH2);
  switch_3_reading = !io_expander.digitalRead(SWITCH3);
  switch_4_reading = !io_expander.digitalRead(SWITCH4);
}

void read_selector() {  // TODO Needed updated
  if (!io_expander.digitalRead(SELECTOR1)) { 
    selectorPosition = SAFE;
  } else if (!io_expander.digitalRead(SELECTOR2)) {
    selectorPosition = POS_1;
  } else if (!io_expander.digitalRead(SELECTOR3)) {
    selectorPosition = POS_2;
  } else if (!io_expander.digitalRead(SELECTOR4)) {
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
  int reading = !io_expander.digitalRead(TRIGGER);
  if (reading != lastTriggerState) {
    lastTriggerDebounce = millis();
    lastTriggerState = reading;
  }
  if ((millis() - lastTriggerDebounce) > triggerDebounceTime) {
    currentTriggerState = reading;
  }
}

void bidirection_driver_move(driver_direction action, int speed) {
  if (action == COAST){
    digitalWrite(DRIVER_BI_IN1, LOW);
    digitalWrite(DRIVER_BI_IN2, LOW);
  } else if (action == FORWARD) {
    digitalWrite(DRIVER_BI_IN2, LOW);  // do 2 low first toavoid accadently going into brake
    analogWrite(DRIVER_BI_IN1, speed);
  } else if (action == BACKWARDS) {
    digitalWrite(DRIVER_BI_IN1, LOW);
    analogWrite(DRIVER_BI_IN2, speed);
  } else if (action == BRAKE) {
    digitalWrite(DRIVER_BI_IN1, HIGH);
    digitalWrite(DRIVER_BI_IN2, HIGH);
  }
}

void high_current_driver_move(driver_direction action, int speed) {
  if (action == COAST || action == BRAKE){
    digitalWrite(DRIVER_HIGH_IN, LOW);
  } else if (action == FORWARD) {
    analogWrite(DRIVER_HIGH_IN, speed);
  }
}

void error_tone(int times_to_play) {
  for (int i = 0; i < times_to_play; i++) {
    tone(BUZZER_Pin, 440, 200);
    delay(100);
    noTone(BUZZER_Pin);
    delay(100);
  }
}

// power_on_tone
// setup_tone
// reset_tone


void print_states() {
  Serial.print(switch_1_reading);
  Serial.print(": ");
  Serial.print(switch_2_reading);
  Serial.print(": ");
  Serial.print(switch_3_reading);
  Serial.print(": ");
  Serial.print(": ");
  Serial.print(!io_expander.digitalRead(SELECTOR1));
  Serial.print(": ");
  Serial.print(!io_expander.digitalRead(SELECTOR2));
  Serial.print(": ");
  Serial.print(!io_expander.digitalRead(SELECTOR3));
  Serial.print(": ");
  Serial.print(!io_expander.digitalRead(SELECTOR4));
  Serial.print(": ");
  Serial.println(selectorPosition);
}

#endif