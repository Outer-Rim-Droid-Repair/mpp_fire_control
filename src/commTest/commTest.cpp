#include <Arduino.h>
#include "FireControl/FireControlStructsEnums.h"
#include "MEDIC_Comms/MEDIC_Comms.h"
#include "commTest.h"


int flywheel_speed = 255;
int flywheel_idle_speed = 175;  

//int maxFireRate = 15;

fireMode selectableFireModes[3] = {SINGLE_FIRE, BURST_FIRE, AUTO_FIRE};
int selectableBurstAmounts[3]    = {1,           3,          -1};
int selectablemaxFireRates[3]   = {15,          15,         15};
bool selectableUseIdle[3]       = {false,       false,      false}; 

//enums
enum selector_positions {
    SAFE = -1,
    POS_1 = 0,
    POS_2 = 1,
    POS_3 = 2,
    INVALID = -2
};

const char version[6] = "V0.1";

MEDIC_FIRE_CONTROL_RECEIVER communicator = MEDIC_FIRE_CONTROL_RECEIVER();

int selectorPosition = SAFE;
blasterTypes blaster_type = FLYWHEELER;

// quick accesses settings
#define DEBUG_MODE true

void setup() {
  Serial.begin(9600); // initialize serial communication:

  // pinMode(LED_BUILTIN, OUTPUT);

  //communicator = MEDIC_FIRE_CONTROL_RECEIVER();
  communicator.connectOnRequestIdentifyFunction(fillIdentifier);
  communicator.connectOnRequestSettingsFunction(fillSettings);
  communicator.connectSetSettingFunction(setSettings);
  communicator.connectOnRequestStatusFunction(fillStatus);
  communicator.begin();
}

void loop() {
  // digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  //Serial.println("HIGH");
  delay(1000);                      // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  //Serial.println("LOW");
  delay(1000);                      // wait for a second
}

void fillStatus() {
  communicator.statusStruct.FireMode = selectableFireModes[selectorPosition];
  communicator.statusStruct.BurstAmount = selectableBurstAmounts[selectorPosition];
  communicator.statusStruct.safteyState = 0;
  communicator.statusStruct.triggerState = 0;
}

void fillIdentifier() {
  strcpy(communicator.identifyStruct.version, version);
  communicator.identifyStruct.blaster_type = (int) blaster_type;
  Serial.print("Type: ");
  Serial.println(blaster_type);
}

void setSettings() {
  memcpy(&selectableFireModes, &communicator.settingStruct.selectableFireModes[0], sizeof(selectableFireModes));
  memcpy(&selectableBurstAmounts, &communicator.settingStruct.selectableBurstAmounts[0], sizeof(selectableBurstAmounts));
  memcpy(&selectablemaxFireRates, &communicator.settingStruct.selectablemaxFireRates[0], sizeof(selectablemaxFireRates));
  // maxDPS = communicator.settingStruct.maxFireRate;
  // idlePossition = (idleMode) communicator.settingStruct.idlePossition;
}

void fillSettings() {
  // convert firemode to int this should get changed back. see TODO in MEDIC_Comms
  unsigned int modes[3];
  for (unsigned int i = 0; i < 3; i++){
    modes[i] = (unsigned int) selectableFireModes[i];
  }
  memcpy(&communicator.settingStruct.selectableFireModes, &modes[0], sizeof(communicator.settingStruct.selectableFireModes));
  memcpy(&communicator.settingStruct.selectableBurstAmounts, &selectableBurstAmounts[0], sizeof(communicator.settingStruct.selectableBurstAmounts));
  memcpy(&communicator.settingStruct.selectablemaxFireRates, &selectablemaxFireRates[0], sizeof(communicator.settingStruct.selectablemaxFireRates));
  memcpy(&communicator.settingStruct.selectableUseIdle, &selectableUseIdle[0], sizeof(communicator.settingStruct.selectableUseIdle));
  communicator.settingStruct.idlePossitionLevel = flywheel_idle_speed;
}
