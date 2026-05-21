#ifndef MEDIC_Comms_cpp
#define MEDIC_Comms_cpp

#include "Arduino.h"
#include "MEDIC_Comms.h"
#include "Wire.h"


MEDIC* MEDIC::globalLib = 0;

/* ---------------- MEDIC ---------------- */

MEDIC::MEDIC(int address) {
  globalLib = this;
  _address = address;
}



/*
Start i2c connections
*/
void MEDIC::begin() {
	Wire.begin(_address);  // open i22
}

void MEDIC::onReceiveHandler(int numBytesReceived) {
}

// Handles what function to call on a requestbased on current mode
void MEDIC::onRequestHandler() {
}

/* ---------------- MEDIC_CONNTROLLER ---------------- */

MEDIC_CONNTROLLER::MEDIC_CONNTROLLER(): MEDIC(CONTROLLER_ADDRESS) {}

// set target device to a response mode
void MEDIC_CONNTROLLER::SetUnitToMode(int targetAddress, mode selectedMode) {
	static SendMessageStruct modeBlock;
	modeBlock.targetMode = selectedMode;
	
	Wire.beginTransmission(targetAddress);
	Wire.write((byte*) &modeBlock, sizeof(modeBlock));
	Wire.endTransmission();
}

// get IdentifyStatusStruct from target device
void MEDIC_CONNTROLLER::requestIdentifyStatus(byte address) {
	identifyStatus = IdentifyStatusStruct(); // clear identifyStatus struct
	byte numBytes = sizeof(identifyStatus);  // get size
	// request the data be send. returns false if not a valid sender
	_request(address, numBytes, IDENTIFY);
	Wire.readBytes( (byte*) &identifyStatus, numBytes); // read identifyStatus
}

// get status from the power board device
void MEDIC_CONNTROLLER::requestPowerStatus() {	
	powerBoardStatus = PowerBoardStatusStruct(); // clear powerBoardStatus struct
	byte numBytes = sizeof(powerBoardStatus); // get size
	// request the data be send. returns false if not a valid sender
	_request(POWER_DISTRO_BOARD_ADDRESS, numBytes, STATUS);
	Wire.readBytes( (byte*) &powerBoardStatus, numBytes); // read powerBoardStatus
}

// get status from the fire control device
void MEDIC_CONNTROLLER::requestFireControlStatus() {	
	fireControlStatus = fireControlStatusStruct(); // clear fireControlStatus struct
	byte numBytes = sizeof(fireControlStatus); // get size
	// request the data be send. returns false if not a valid sender
	_request(FIRE_CONTROL_BOARD_ADDRESS, numBytes, STATUS);
	Wire.readBytes( (byte*) &fireControlStatus, numBytes); // read fireControlStatus
}

// get status from the chrono device
void MEDIC_CONNTROLLER::requestChronoStatus() {	
	chronoStatus = chronoStatusStruct(); // clear chronoStatus struct
	byte numBytes = sizeof(chronoStatus); // get size
	// request the data be send. returns false if not a valid sender
	_request(CHRONO_BOARD_ADDRESS, numBytes, STATUS);
	Wire.readBytes( (byte*) &chronoStatus, numBytes); // read chrono Status
	//Serial.println(chronoStatus.lastFPS);
}

void MEDIC_CONNTROLLER::requestPowerSettings() {
	powerBoardSettings = PowerBoardSettingsStruct(); // clear powerBoardSettings struct
	byte numBytes = sizeof(powerBoardSettings); // get size
	// request the data be send. returns false if not a valid sender
	_request(POWER_DISTRO_BOARD_ADDRESS, numBytes, GET_SETTINGS);
	Wire.readBytes( (byte*) &powerBoardSettings, numBytes); // read powerBoardSettings
}

void MEDIC_CONNTROLLER::requestFireControlSettings() {
	fireControlSettings = fireControlSettingsStruct(); // clear fireControlSettings struct
	byte numBytes = sizeof(fireControlSettings); // get size
	// request the data be send. returns false if not a valid sender
	_request(FIRE_CONTROL_BOARD_ADDRESS, numBytes, GET_SETTINGS);
	Wire.readBytes( (byte*) &fireControlSettings, numBytes); // read fireControlSettings
}

void MEDIC_CONNTROLLER::requestChronoSettings() {
	chronoSettings = chronoSettingsStruct(); // clear chronoSettings struct
	byte numBytes = sizeof(chronoSettings); // get size
	// request the data be send. returns false if not a valid sender
	_request(CHRONO_BOARD_ADDRESS, numBytes, GET_SETTINGS);
	Wire.readBytes( (byte*) &chronoSettings, numBytes); // read chronoSettings
}

void MEDIC_CONNTROLLER::setPowerSettings() {
	SetUnitToMode(POWER_DISTRO_BOARD_ADDRESS, SET_SETTINGS); // set to the given type
	Wire.beginTransmission(POWER_DISTRO_BOARD_ADDRESS);
	Wire.write((byte*) &powerBoardSettings, sizeof(powerBoardSettings));
	Wire.endTransmission();
}

void MEDIC_CONNTROLLER::setFireControlSettings() {
	SetUnitToMode(FIRE_CONTROL_BOARD_ADDRESS, SET_SETTINGS); // set to the given type
	Wire.beginTransmission(FIRE_CONTROL_BOARD_ADDRESS);
	Wire.write((byte*) &fireControlSettings, sizeof(fireControlSettings));
	Wire.endTransmission();
}

void MEDIC_CONNTROLLER::setChronoSettings() {
	SetUnitToMode(CHRONO_BOARD_ADDRESS, SET_SETTINGS); // set to the given type
	Wire.beginTransmission(CHRONO_BOARD_ADDRESS);
	Wire.write((byte*) &chronoSettings, sizeof(chronoSettings));
	Wire.endTransmission();
}

void MEDIC_CONNTROLLER::resetUnit(int targetAddress) {
	SetUnitToMode(targetAddress, RESET);
}

// attempts to connect to a given device
bool MEDIC_CONNTROLLER::checkDeviceInSystem(byte address) {
	requestIdentifyStatus(address);  // get device status
	if (strlen(identifyStatus.version) != 0){  // see if a version was placed
		return true;
	}
	return false;
}

// Handles requesting data froma device
bool MEDIC_CONNTROLLER::_request(byte address, byte numBytes, mode type) {
	SetUnitToMode(address, type); // set to the given type
	byte stop = true;
	Wire.requestFrom(address, numBytes, stop);  // request data
	return true;
}


/* ---------------- MEDIC_RECEIVER ---------------- */

MEDIC_RECEIVER::MEDIC_RECEIVER(int address): MEDIC(address) {

}

void MEDIC_RECEIVER::begin() {
	Wire.begin(_address);  // open i2c
	Serial.println("setup");
	Wire.onReceive(staticOnReceiveHandler); 
	Wire.onRequest(staticOnRequestHandler);
}


void MEDIC_RECEIVER::connectSetSettingFunction(void (*funct)()) {
	_onSetSettingsFunction = funct;
}


// add a function to call if a status request is made
void MEDIC_RECEIVER::connectOnRequestStatusFunction(void (*funct)()) {	
	_onRequestStatusFunction = funct;
}

// add a function to call if a identify request is made
void MEDIC_RECEIVER::connectOnRequestIdentifyFunction(void (*funct)()) {	
	_onRequestIdentifyFunction = funct;
}

// add a function to call if a settings request is made
void MEDIC_RECEIVER::connectOnRequestSettingsFunction(void (*funct)()) {	
	_onRequestSettingsFunction = funct;
}

// add a function to call if a settings request is made
void MEDIC_RECEIVER::connectOnResetFunction(void (*funct)()) {	
	_onResetFunction = funct;
}

void MEDIC_RECEIVER::onReceiveHandler(int numBytesReceived) {
}

// Handles what function to call on a requestbased on current mode
void MEDIC_RECEIVER::onRequestHandler() {

}

// --------------------------------
void MEDIC_POWER_BOARD_RECEIVER::onReceiveHandler(int numBytesReceived) {
	if (_currentMode != SET_SETTINGS) {
		Wire.readBytes( (byte*) &recivedData, numBytesReceived);
		_currentMode = recivedData.targetMode;
	} else {  // in SET_SETTINGS
		_currentMode = IDENTIFY;  // set to a different mode to leave set mode
		Wire.readBytes( (byte*) &settingStruct, numBytesReceived);
		_onSetSettingsFunction();
	}
}

void MEDIC_POWER_BOARD_RECEIVER::onRequestHandler() {
	if (_currentMode == STATUS){
		statusStruct = PowerBoardStatusStruct();
		statusStruct.heartbeat = true;
		_onRequestStatusFunction();
		Wire.write((byte*) &statusStruct, sizeof(statusStruct));  // send data
	} else if (_currentMode == IDENTIFY) {
		identifyStruct = IdentifyStatusStruct();
		identifyStruct.heartbeat = true;
		_onRequestIdentifyFunction();
		Wire.write((byte*) &identifyStruct, sizeof(identifyStruct));  // send data
	} else if (_currentMode == GET_SETTINGS) {
		settingStruct = PowerBoardSettingsStruct();
		settingStruct.heartbeat = true;
		_onRequestSettingsFunction();
		Wire.write((byte*) &settingStruct, sizeof(settingStruct));  // send data
	} else { // invalid _currentMode
		return;
	}
}

// --------------------------------
void MEDIC_FIRE_CONTROL_RECEIVER::onReceiveHandler(int numBytesReceived) {
	Serial.print("received message. Mode: ");
	Serial.println(_currentMode);
	if (_currentMode != SET_SETTINGS) {
		Wire.readBytes( (byte*) &recivedData, numBytesReceived);
		_currentMode = recivedData.targetMode;
	} else {  // in SET_SETTINGS
		_currentMode = IDENTIFY;  // set to a different mode to leave set mode
		Wire.readBytes( (byte*) &settingStruct, numBytesReceived);
		_onSetSettingsFunction();
	}
}

void MEDIC_FIRE_CONTROL_RECEIVER::onRequestHandler() {
	Serial.print("received request. Mode: ");
	Serial.println(_currentMode);
	if (_currentMode == STATUS){
		statusStruct = fireControlStatusStruct();
		statusStruct.heartbeat = true;
		_onRequestStatusFunction();
		Wire.write((byte*) &statusStruct, sizeof(statusStruct));  // send data
	} else if (_currentMode == IDENTIFY) {
		identifyStruct = IdentifyStatusStruct();
		identifyStruct.heartbeat = true;
		_onRequestIdentifyFunction();
		Wire.write((byte*) &identifyStruct, sizeof(identifyStruct));  // send data
	} else if (_currentMode == GET_SETTINGS) {
		settingStruct = fireControlSettingsStruct();
		settingStruct.heartbeat = true;
		_onRequestSettingsFunction();
		Wire.write((byte*) &settingStruct, sizeof(settingStruct));  // send data
	} else { // invalid _currentMode
		return;
	}
}

// --------------------------------

void MEDIC_CHRONO_RECEIVER::onReceiveHandler(int numBytesReceived) {
	Serial.println("received message");
	if (_currentMode != SET_SETTINGS) {
		Wire.readBytes( (byte*) &recivedData, numBytesReceived);
		if (recivedData.targetMode == RESET) {
			_onResetFunction();
		}
		_currentMode = recivedData.targetMode;
		//Serial.println(_currentMode);
	} else {  // in SET_SETTINGS
		_currentMode = IDENTIFY;  // set to a different mode to leave set mode
		Wire.readBytes( (byte*) &settingStruct, numBytesReceived);
		_onSetSettingsFunction();
	}
}

void MEDIC_CHRONO_RECEIVER::onRequestHandler() {
	Serial.println("received request");
	if (_currentMode == STATUS){
		statusStruct = chronoStatusStruct();
		statusStruct.heartbeat = true;
		_onRequestStatusFunction();
		Serial.println(statusStruct.lastFPS);
		Wire.write((byte*) &statusStruct, sizeof(statusStruct));  // send data
	} else if (_currentMode == IDENTIFY) {
		identifyStruct = IdentifyStatusStruct();
		identifyStruct.heartbeat = true;
		_onRequestIdentifyFunction();
		Wire.write((byte*) &identifyStruct, sizeof(identifyStruct));  // send data
	} else if (_currentMode == GET_SETTINGS) {
		settingStruct = chronoSettingsStruct();
		settingStruct.heartbeat = true;
		_onRequestSettingsFunction();
		Wire.write((byte*) &settingStruct, sizeof(settingStruct));  // send data
	} else { // invalid _currentMode
		Serial.print("mode: ");
		Serial.println(_currentMode);
		return;
	}
}

#endif