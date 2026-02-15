#include <Arduino.h>

#include "Chrono.h"
#include "MEDIC_Comms/MEDIC_Comms.h"

const char version[6] = "V0.1";

MEDIC_CHRONO_RECEIVER communicator = MEDIC_CHRONO_RECEIVER();

// TODO add shot counter and add single beam mode to make smaller modular for just shot count/DPS
// TODO add stand alone with screen mode

void setup() {
  //start serial connection
  Serial.begin(9600);

  pinMode(BACK_SENSOR_PIN, INPUT);
  pinMode(FRONT_SENSOR_PIN, INPUT);
  digitalWrite(BACK_SENSOR_PIN, HIGH); // turn on the pullup
  digitalWrite(FRONT_SENSOR_PIN, HIGH); // turn on the pullup

  pinMode(13, OUTPUT);

  communicator.connectOnRequestIdentifyFunction(fillIdentifier);
  communicator.connectOnRequestSettingsFunction(fillSettings);
  communicator.connectSetSettingFunction(setSettings);
  communicator.connectOnRequestStatusFunction(fillStatus);
  communicator.connectOnResetFunction(resetrecords);
  communicator.begin();
}

void loop() {
  static int reading = 0; 

  switch (current_state) {
    case WAITING_FOR_INPUT:
      reading = digitalRead(BACK_SENSOR_PIN);
      // Serial.println(reading);
      if (reading == LOW) {  // beam broken
        enterTime_us = micros();
        fireTime_ms = millis();
        current_state = WAITING_TO_EXIT;
      }
      break;
    case WAITING_TO_EXIT:
      reading = digitalRead(FRONT_SENSOR_PIN);
      if (reading == LOW) {
        exitTime_us = micros();
        current_state = CALCULATING;
      } else if (millis() - fireTime_ms > 250) { //time out after 0.25 seconds
        current_state = WAITING_FOR_INPUT;
      }
      break;
    case CALCULATING:
      float airTime_us = (exitTime_us-enterTime_us);
      float airTime_s = airTime_us / 1000000.;
      lastMPS = distance_m / airTime_s;

      if (lastMPS > maxMPS or maxMPS < 0) {
        maxMPS = lastMPS;
      }
      if (lastMPS < minMPS or minMPS < 0) {
        minMPS = lastMPS;
      }

      if (currentPreviousMPSIndex >= currentRollingLength) {
        currentPreviousMPSIndex = 0;
      }
      previousMPS[currentPreviousMPSIndex] = lastMPS;
      currentPreviousMPSIndex++;

      if (lastFireTime_ms != 0) {
        if ((fireTime_ms - lastFireTime_ms) > timeoutDPS_ms) {  // if too long between shots reset
          calculateDPS();
          resetDPSReconrds();
        } else {
          previousTimeBetweenShots[PreviousTimeBetweenShotsIndex] = fireTime_ms - lastFireTime_ms;
          PreviousTimeBetweenShotsIndex++;

          if (PreviousTimeBetweenShotsIndex >= DPSAverageLength) {
            calculateDPS();
            PreviousTimeBetweenShotsIndex = 0;
          }
        }
      }
      lastFireTime_ms = fireTime_ms;

      printStats();

      current_state = WAITING_FOR_INPUT;
      break;
  } 

}

float calculateAverageMPS() {
  float total = 0.;
  int i;
  for (i=0; i<currentRollingLength; i++) {
    if (previousMPS[i] == 0) {
      break;
    }
    total += previousMPS[i];
  }
  return total / (float) i; 

}

float calculateDPS() {
  float total = 0.;
  int i;
  for (i=0; i<DPSAverageLength; i++) {
    if (previousTimeBetweenShots[i] == 0) {
      break;
    }
    total += previousTimeBetweenShots[i];
  }
  if (i == 0) {
    return -1;
  }
  float average =  total / (float) i;  // average time between shots
  float dps = 1000. / average;  // 1 second / average time between shots

  if (dps > maxDPS or maxDPS < 0) {
    maxDPS = dps;
  } 
  lastDPS = dps;
  return dps;
}

void setUnits(bool isFPS) {
  useFPS = isFPS;
}

void setUpRollingAverage(int rollingAmount) {
  currentRollingLength = rollingAmount;
}

void resetDPSReconrds() {
  Serial.println("reset");
  fireTime_ms = 0;
  lastFireTime_ms = 0;
  memset(previousTimeBetweenShots, 0, sizeof(previousTimeBetweenShots));
  PreviousTimeBetweenShotsIndex = 0;
}

void resetrecords() {
  Serial.print("reset 2");
  maxMPS = -1;
  minMPS = -1;
  memset(previousMPS, 0, sizeof(previousMPS));
  currentPreviousMPSIndex = 0;
  maxDPS = -1.;
  resetDPSReconrds();
}

void printStats() {
  float fps = lastMPS * MPS2FSP;
  float average = calculateAverageMPS();
  average = average * MPS2FSP;
  Serial.print("fps: ");
  Serial.print(fps);
  Serial.print(" min: ");
  Serial.print(minMPS*MPS2FSP);
  Serial.print(" max: ");
  Serial.print(maxMPS*MPS2FSP);
  Serial.print(" average: ");
  Serial.print(average);
  Serial.print(" max DPS: ");
  Serial.print(maxDPS);

  Serial.println("");
}

void fillStatus() {
  float unitAdjuster;
  if (useFPS) {
    unitAdjuster = MPS2FSP;
  } else {
    unitAdjuster = 1;
  }
  communicator.statusStruct.lastFPS = lastMPS * unitAdjuster;
  communicator.statusStruct.maxFPS = maxMPS * unitAdjuster;
  communicator.statusStruct.minFPS = minMPS * unitAdjuster;
  communicator.statusStruct.maxDPS = maxDPS;
  communicator.statusStruct.lastDPS = lastDPS;
}

void fillIdentifier() {
  strcpy(communicator.identifyStruct.version, version);
}

void setSettings() {
  DPSAverageLength = communicator.settingStruct.DPSAverageLength;
  currentRollingLength = communicator.settingStruct.MPSRollingLength;
  timeoutDPS_ms = communicator.settingStruct.timeoutDPS_ms;
  useFPS = communicator.settingStruct.useFPS;
}

void fillSettings() {
  communicator.settingStruct.DPSAverageLength = DPSAverageLength;
  communicator.settingStruct.MPSRollingLength = currentRollingLength;
  communicator.settingStruct.timeoutDPS_ms = timeoutDPS_ms;
  communicator.settingStruct.useFPS = useFPS;
}