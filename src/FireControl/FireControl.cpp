#ifndef FIRE_CONTROL_cpp
#define FIRE_CONTROL_cpp

#include <Arduino.h>
#include <Adafruit_MCP23X08.h>

#include "FireControl.h"

#include "FlyCore_Flywheeler.h"

const char version[6] = "V0.3";

// quick accesses settings
#define DEBUG_MODE true

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
  io_expander.pinMode(SWITCH5, INPUT_PULLUP);
  io_expander.pinMode(SELECTOR1, INPUT_PULLUP);
  io_expander.pinMode(SELECTOR2, INPUT_PULLUP);
  io_expander.pinMode(SELECTOR3, INPUT_PULLUP);

  // outputs
  pinMode(MOTOR1_IN1, OUTPUT);
  digitalWrite(MOTOR1_IN1, LOW);
  pinMode(MOTOR1_IN2, OUTPUT);
  digitalWrite(MOTOR1_IN2, LOW);

  pinMode(MOTOR2_IN1, OUTPUT);
  digitalWrite(MOTOR2_IN1, LOW);
  pinMode(MOTOR2_IN2, OUTPUT);
  digitalWrite(MOTOR2_IN2, LOW);

  Serial.begin(9600); // initialize serial communication:

  inital_setup();
  while (!Serial);
  Serial.println("Running");
}

void loop() {
  static bool blasterSetup = false;     // setup completion flag
  static bool triggerReleased = true;   // Has the trigger been released after the last shot. Used to force trigger release when needed
  static int burstCount = 0;            // How many shots have been fired 
  // static long lastDevMessage = 0;       // for timing debug messages

  read_selector();
  update_trigger_state();
  read_switchs();
  /*
  Serial.print(rev_trigger_reading);
  Serial.print(": ");
  Serial.print(switch_3_reading);
  Serial.print(": ");
  Serial.print(!switch3Pin);
  Serial.print(": ");
  Serial.println(selectorPosition);*/

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
  switch_1_reading = !io_expander.digitalRead(SWITCH1);
  switch_2_reading = !io_expander.digitalRead(SWITCH2);
  switch_3_reading = !io_expander.digitalRead(SWITCH3);
  switch_4_reading = !io_expander.digitalRead(SWITCH4);
  switch_5_reading = !io_expander.digitalRead(SWITCH5);
}

void read_selector() {  // TODO Needed updated
  if (!io_expander.digitalRead(SELECTOR1)) { 
    selectorPosition = POS_1;
  } else if (!io_expander.digitalRead(SELECTOR2)) {
    selectorPosition = POS_2;
  } else if (!io_expander.digitalRead(SELECTOR3)) {
    selectorPosition = POS_3;
  } else {
    selectorPosition = SAFE;
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

void driver_coast(int driver) {
  int in1, in2;
  if (driver == 1){
    in1 = MOTOR1_IN1;
    in2 = MOTOR1_IN2;
  } else if (driver == 2){
    in1 = MOTOR2_IN1;
    in2 = MOTOR2_IN2;
  } else {
    return;
  }
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}

void driver_reverse(int driver, int speed = 255) {
  int in1, in2;
  if (driver == 1){
    in1 = MOTOR1_IN1;
    in2 = MOTOR1_IN2;
  } else if (driver == 2){
    in1 = MOTOR2_IN1;
    in2 = MOTOR2_IN2;
  } else {
    return;
  }
  // Speed options not avalible on current board version
  // TODO fix that
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
}

void driver_forward(int driver, int speed = 255) {
  int in1, in2;
  if (driver == 1){
    in1 = MOTOR1_IN1;
    in2 = MOTOR1_IN2;
  } else if (driver == 2){
    in1 = MOTOR2_IN1;
    in2 = MOTOR2_IN2;
  } else {
    return;
  }
  analogWrite(in1, speed);
  digitalWrite(in2, LOW);
}

void driver_brake(int driver) {
  int in1, in2;
  if (driver == 1){
    in1 = MOTOR1_IN1;
    in2 = MOTOR1_IN2;
  } else if (driver == 2){
    in1 = MOTOR2_IN1;
    in2 = MOTOR2_IN2;
  } else {
    return;
  }
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
}


/*
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
}*/

#endif