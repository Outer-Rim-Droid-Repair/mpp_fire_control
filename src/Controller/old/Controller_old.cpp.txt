#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RotaryEncoder.h>

#include "Controller.h"
#include "MEDIC_Comms/MEDIC_Comms.h"
#include "MEDIC_Comms/MEDIC_Screens.h"
#include "FireControl/FireControlStructsEnums.h"
#include "FireControl/FireControlStructsEnums.cpp"


const char version[6] = "V0.1";


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


MEDIC_CONNTROLLER communicator;

Version_Screen Version_Screen_Control;
Chrono_Screen Chrono_Screen_Control;
Fire_Control_Screen Fire_Control_Screen_Control;

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_IN, INPUT_PULLUP);

  // Wait for display
  delay(500);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;) {
      Serial.println(F("SSD1306 allocation failed"));
      delay(1000);
    } // Don't proceed, loop forever
    // TODO handle failure differently
  }
  display.display();
  display.clearDisplay();
  communicator = MEDIC_CONNTROLLER();
  communicator.begin();

  Version_Screen_Control = Version_Screen(display);
  Chrono_Screen_Control = Chrono_Screen(display);
  Fire_Control_Screen_Control = Fire_Control_Screen(display);

  Version_Screen_Control.drawTestPattern();
  delay(1000); // Pause for 1 seconds
  updateConnectedDevices();
  updateVersionScreen();
  delay(1000);
}


int i = 0;
unsigned long lastupdate = 0;
const int updateSpeed = 100;
bool buttonUpdate = false;
bool editMode = false;
bool redrawBackground = true;

void loop() {
  
  if ((millis() - lastupdate >= updateSpeed) and (!editMode)){
    if (selectedScreenState == VERSION) {
      updateVersionScreen();
    } else if (selectedScreenState == FIRE_MODE_STATUS) {
      updateFireModeScreen();
    } else if (selectedScreenState == CHRONO_STATUS) {
      updateChronoStatusScreen();
    } else if (selectedScreenState == POWER_STATUS) {
      display.fillScreen(SSD1306_BLACK);  // clear screen
    } 
    // Version_Screen_Control.invertSection(20, 20 , 60, 60);
    lastupdate = millis();
  }
  readKeypad();
  if (buttonUpdate) {
    if (lastPressed == IN and !editMode) {
      editMode = true;
      lastEditMode = INIT;
    }
    if (!editMode) {
      if (lastPressed == LEFT) {
        findNextValidScreen(false);
      } else if (lastPressed == RIGHT) {
        findNextValidScreen(true);
      }
    } else {
      if (currentScreenState == VERSION) {
        editVersionScreen();
      } else if (currentScreenState == CHRONO_STATUS) {
        editChronoStatusScreen();
      } else if (currentScreenState == FIRE_MODE_STATUS) {
        editFireModeScreen();
      }
    }
    buttonUpdate = false;
  }
  delay(20);
  // Serial.println("Encoder value: ");
}

void findNextValidScreen(bool countUp) {
  int direction = 1;
  if (!countUp) {
    direction = -1;
  }

  unsigned int index = selectedScreenState;
  SCREEN_STATE state;
  while (true) {
    if (index <= 0 and !countUp) {
      index = sizeof(screenOrder);
    }
    index = (index + direction) % sizeof(screenOrder);
    state = screenOrder[index];

    if (state == VERSION) {
      selectedScreenState = VERSION;
      break;
    } else if ((state == CHRONO_STATUS) and (chronoPresent)) {
      selectedScreenState = CHRONO_STATUS;
      break;
    } else if ((state == FIRE_MODE_STATUS) and (fireControlPresent)) {
      selectedScreenState = FIRE_MODE_STATUS;
      break;
    } else if ((state == POWER_STATUS) and (powerBoardPresent)) {
      selectedScreenState = POWER_STATUS;
      break;
    }

    if (index == selectedScreenState) {
      break;
    }
  }
  

}

void readKeypad(void) {
  int buttonState = LOW;
  static int lastButtonState = HIGH;

  if (! digitalRead(BUTTON_UP)) {
    lastPressed = UP;
  } else if (! digitalRead(BUTTON_LEFT)) {
    lastPressed = LEFT;
  } else if (! digitalRead(BUTTON_DOWN)) {
    lastPressed = DOWN;
  } else if (! digitalRead(BUTTON_RIGHT)) {
    lastPressed = RIGHT;
  } else if (! digitalRead(BUTTON_IN)) {
    lastPressed = IN;
  } else {
    lastPressed = NONE;
    buttonState = HIGH;
  }
  if (buttonState != lastButtonState && lastPressed != NONE) {
    buttonUpdate = true;
  }

  lastButtonState = buttonState;
}

void updateConnectedDevices(void) {
  powerBoardPresent = communicator.checkDeviceInSystem(POWER_DISTRO_BOARD_ADDRESS);
  fireControlPresent = communicator.checkDeviceInSystem(FIRE_CONTROL_BOARD_ADDRESS);
  chronoPresent = communicator.checkDeviceInSystem(CHRONO_BOARD_ADDRESS);
}

// --------------------- screens ---------------------
// Version
void updateVersionScreen(void) {
  if ((currentScreenState != VERSION) or redrawBackground) {
      Version_Screen_Control.drawBackgrond();
      currentScreenState = VERSION;
      redrawBackground = false;
  }
  char controllerVersion[6] = "";
  char powerBoardVersion[6] = "N/A";  // char[6] 
  char fireControlVersion[6] = "N/A";  // char[6] 
  char chronoVersion[6] = "N/A";  // char[6] 

  strcpy(controllerVersion, version);
  if (powerBoardPresent) {
    communicator.requestIdentifyStatus(POWER_DISTRO_BOARD_ADDRESS);
    strcpy(powerBoardVersion, communicator.identifyStatus.version);
  }
  if (fireControlPresent) {
    communicator.requestIdentifyStatus(FIRE_CONTROL_BOARD_ADDRESS);
    strcpy(fireControlVersion, communicator.identifyStatus.version);
  }
  if (chronoPresent) {
    communicator.requestIdentifyStatus(CHRONO_BOARD_ADDRESS);
    strcpy(chronoVersion, communicator.identifyStatus.version);
  }
  Version_Screen_Control.drawInfo(controllerVersion, powerBoardVersion, fireControlVersion, chronoVersion);
}

void editVersionScreen(void) {
  editMode = false;
}

// Chrono
void updateChronoStatusScreen(void) {
  if ((currentScreenState != CHRONO_STATUS) or redrawBackground) {
    Chrono_Screen_Control.drawBackgrond();
    currentScreenState = CHRONO_STATUS;
    redrawBackground = false;
  }
  communicator.requestChronoStatus();

  float lastFPS = communicator.chronoStatus.lastFPS;
  float maxFPS = communicator.chronoStatus.maxFPS;
  float minFPS = communicator.chronoStatus.minFPS;
  float lastDPS = communicator.chronoStatus.lastDPS;
  float maxDPS = communicator.chronoStatus.maxDPS;
  Chrono_Screen_Control.drawInfo(lastFPS, maxFPS, minFPS, lastDPS, maxDPS);
}

void editChronoStatusScreen(void) {
  if (lastPressed == IN) {
    Chrono_Screen_Control.drawQuestionBox("Reset Data?");
    redrawBackground = true;       
  } else if (lastPressed == UP) {
    // reset chrono
    communicator.resetUnit(CHRONO_BOARD_ADDRESS);
    editMode = false;
  } else if (lastPressed == DOWN) {
    editMode = false;
  }
}

// Fire Mode
void updateFireModeScreen(void) {
  if ((currentScreenState != FIRE_MODE_STATUS) or redrawBackground) {
    Fire_Control_Screen_Control.drawBackgrond();
    currentScreenState = FIRE_MODE_STATUS;
    redrawBackground = false;
  }
  communicator.requestFireControlStatus();

  int *selectableFireModes = communicator.fireControlSettings.selectableFireModes;
  int *selectableBurstAmounts = communicator.fireControlSettings.selectableBurstAmounts;
  Fire_Control_Screen_Control.drawInfo(selectableFireModes, selectableBurstAmounts);
}

void editFireModeScreen(void) {
  static unsigned int xLoaction = 0;
  static unsigned int yLocation = 0;
  if (lastPressed == IN) {
    if (lastEditMode == INIT) {
      lastEditMode = MODIFY;
      xLoaction = 0;
      yLocation = 0;
    } else if (lastEditMode == MODIFY) {
      Chrono_Screen_Control.drawQuestionBox("Confirm Changes?");
      redrawBackground = true;  
      lastEditMode = CONFIRM;
      return;
    }
  }

  if (lastEditMode == CONFIRM) {
    if (lastPressed == UP) {
      // send changes
      editMode = false;
    } else if (lastPressed == DOWN) {
      lastEditMode = MODIFY;
      updateFireModeScreen();
      Fire_Control_Screen_Control.addOutline(xLoaction, yLocation, true);
      Fire_Control_Screen_Control.forceScreenDraw();
    }
    return;
  }


  Fire_Control_Screen_Control.addOutline(xLoaction, yLocation, false);
  if (lastPressed == DOWN) {
    if (yLocation < ((sizeof(Fire_Control_Screen_Control.y1) / sizeof(*Fire_Control_Screen_Control.y1)) - 1)) {
      yLocation += 1;
    }
  } else if (lastPressed == UP) {
    if (yLocation > 0) {
      yLocation -= 1;
    }
  } else if (lastPressed == RIGHT) {
    if (xLoaction < ((sizeof(Fire_Control_Screen_Control.x1) / sizeof(*Fire_Control_Screen_Control.x1)) - 1)) {
      xLoaction += 1;
    }
  } else if (lastPressed == LEFT) {
    if (xLoaction > 0) {
      xLoaction -= 1;
    }
  } else if (lastPressed == ROTARY) {
    
  }


  Fire_Control_Screen_Control.addOutline(xLoaction, yLocation, true);
  Fire_Control_Screen_Control.forceScreenDraw();
}
