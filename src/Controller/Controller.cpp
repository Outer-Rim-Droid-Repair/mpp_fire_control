#include "Arduino.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <RotaryEncoder.h>

#include "Controller.h"
#include "MEDIC_Comms/MEDIC_Comms.h"
// #include "MEDIC_Comms/MEDIC_Screens.h"
#include "FireControl/FireControlStructsEnums.h"
#include "FireControl/FireControlStructsEnums.cpp"


const char version[6] = "V0.1";

// Screen_ILI9341 display;
Adafruit_ILI9341 screen_obj = Adafruit_ILI9341(TFT_CS, TFT_DC);
RotaryEncoder encoder(PIN_ENCODER_A, PIN_ENCODER_B, RotaryEncoder::LatchMode::TWO03);

MEDIC_CONNTROLLER communicator;

unsigned long lastupdate = 0;
const int updateSpeed = 100;
bool buttonUpdate = false;
bool editMode = false;
bool redrawBackground = true;
int width, height;
blasterTypes blaster_type;

int last_rotary = 0;


void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  /*pinMode(COMA, OUTPUT);
  digitalWrite(COMA, LOW);
  pinMode(COMB, OUTPUT);
  digitalWrite(COMB, LOW);*/

  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_IN, INPUT_PULLUP);

  pinMode(A5, OUTPUT);  // Needed to get an extra gnd for comm testing
  digitalWrite(A5, LOW);

  delay(500);

  communicator = MEDIC_CONNTROLLER();
  communicator.begin(); 
  screen_obj.begin();
  delay(500);
  screen_obj.setRotation(1);
  width = screen_obj.width();
  height = screen_obj.height();

  clear_screen();
  drawTestPattern(40, ILI9341_WHITE, ILI9341_BLACK, ILI9341_RED);
  delay(500);
  updateConnectedDevices();
  updateVersionScreen();
}

void loop() {
  
  if ((millis() - lastupdate >= updateSpeed) and (!editMode)){
    if (selectedScreenState == VERSION) {
      // Serial.println("scanning");
      // updateConnectedDevices();
      // updateVersionScreen();
    } else if (selectedScreenState == FIRE_MODE_STATUS) {
      updateFireModeScreen();
    } else if (selectedScreenState == CHRONO_STATUS) {
      //updateChronoStatusScreen();
    } else if (selectedScreenState == POWER_STATUS) {
      clear_screen();  // clear screen
    } 
    // Version_Screen_Control.invertSection(20, 20 , 60, 60);
    lastupdate = millis();
  }
  lastPressed = readKeypad();
  if (buttonUpdate) {
    if (lastPressed == IN and !editMode) {
      // editMode = true;
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
    Serial.println(state);
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

POSSITIONS readKeypad(void) {
  encoder.tick();
  int curr_rotary = encoder.getPosition();
  RotaryEncoder::Direction direction = encoder.getDirection();
  if (curr_rotary != last_rotary) {
    Serial.print("Encoder value: ");
    Serial.print(curr_rotary);
    Serial.print(" direction: ");
    Serial.println((int)direction);
    last_rotary = curr_rotary;
    if (direction > RotaryEncoder::Direction::CLOCKWISE) {
      return ROTARY_CLOCKWISE;
    } else {
      return ROTARY_COUNTERCLOCKWISE;
    }
  }
  last_rotary = curr_rotary;

  buttonUpdate = true;
  if (! digitalRead(BUTTON_UP)) {
    Serial.println("UP");
    return UP;
  }
  if (! digitalRead(BUTTON_LEFT)) {
    Serial.println("LEFT");
    return LEFT;
  }
  if (! digitalRead(BUTTON_DOWN)) {
    Serial.println("DOWN");
    return DOWN;
  }
  if (! digitalRead(BUTTON_RIGHT)) {
    Serial.println("RIGHT");
    return RIGHT;
  }
  if (! digitalRead(BUTTON_IN)) {
    Serial.println("IN");
    return IN;
  }
  buttonUpdate = false;
  return NONE;
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
      drawVersionBackgrond();
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
    blaster_type = (blasterTypes) communicator.identifyStatus.blaster_type;
  }
  if (chronoPresent) {
    communicator.requestIdentifyStatus(CHRONO_BOARD_ADDRESS);
    strcpy(chronoVersion, communicator.identifyStatus.version);
  }
  drawVersionInfo(controllerVersion, powerBoardVersion, fireControlVersion, chronoVersion);
}

void editVersionScreen(void) {
  editMode = false;
}

// Chrono
void updateChronoStatusScreen(void) {
  if ((currentScreenState != CHRONO_STATUS) or redrawBackground) {
    drawChronoBackgrond();
    currentScreenState = CHRONO_STATUS;
    redrawBackground = false;
  }
  communicator.requestChronoStatus();

  float lastFPS = communicator.chronoStatus.lastFPS;
  float maxFPS = communicator.chronoStatus.maxFPS;
  float minFPS = communicator.chronoStatus.minFPS;
  float lastDPS = communicator.chronoStatus.lastDPS;
  float maxDPS = communicator.chronoStatus.maxDPS;
  drawChronoInfo(lastFPS, maxFPS, minFPS, lastDPS, maxDPS);
}

void editChronoStatusScreen(void) {
  if (lastPressed == IN) {
    drawQuestionBox("Reset Data?");
    redrawBackground = true;       
  } else if (lastPressed == UP) {
    // reset chrono
    communicator.resetUnit(CHRONO_BOARD_ADDRESS);
    editMode = false;
  } else if (lastPressed == DOWN) {
    editMode = false;
  }
}

// Fire Mode Screens
void updateFireModeScreen(void) {
  if ((currentScreenState != FIRE_MODE_STATUS) or redrawBackground) {
    drawFireControlBackgrond();
    currentScreenState = FIRE_MODE_STATUS;
    redrawBackground = false;
  }
  
  communicator.requestFireControlSettings();

  int *selectableFireModes = communicator.fireControlSettings.selectableFireModes;
  int *selectableBurstAmounts = communicator.fireControlSettings.selectableBurstAmounts;
  int *selectablemaxFireRates = communicator.fireControlSettings.selectablemaxFireRates;
  bool *selectableUseIdle = communicator.fireControlSettings.selectableUseIdle;
  int idlePossitionLevel = communicator.fireControlSettings.idlePossitionLevel;

  drawFireControlInfo(selectableFireModes, selectableBurstAmounts, selectablemaxFireRates, selectableUseIdle, idlePossitionLevel);
}

void editFireModeScreen(void) {
  /*static unsigned int xLoaction = 0;
  static unsigned int yLocation = 0;
  if (lastPressed == IN) {
    if (lastEditMode == INIT) {
      lastEditMode = MODIFY;
      xLoaction = 0;
      yLocation = 0;
    } else if (lastEditMode == MODIFY) {
      display.drawQuestionBox("Confirm Changes?");
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
      display.addOutline(xLoaction, yLocation, true);
    }
    return;
  }


  display.addOutline(xLoaction, yLocation, false);
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
  Fire_Control_Screen_Control.forceScreenDraw();*/
}

void drawFireControlBackgrond() {
  screen_obj.fillScreen(ILI9341_BLACK);  // clear screen
  screen_obj.drawRect(0, 0, width, height, ILI9341_WHITE);  // boarder rect
  screen_obj.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // init text settings
  screen_obj.setTextSize(1);

  screen_obj.drawFastVLine(12, 0, 62, ILI9341_WHITE);
  screen_obj.drawFastVLine(60, 0, 62, ILI9341_WHITE);
  screen_obj.drawFastVLine(114, 0, 62, ILI9341_WHITE);
  screen_obj.drawFastVLine(173, 0, 62, ILI9341_WHITE);
  screen_obj.drawFastHLine(0, 16, width, ILI9341_WHITE);
  screen_obj.drawFastHLine(0, 32, width, ILI9341_WHITE);
  screen_obj.drawFastHLine(0, 47, width, ILI9341_WHITE);
  screen_obj.drawFastHLine(0, 62, width, ILI9341_WHITE);

  screen_obj.setCursor(15, 5);
  screen_obj.print("Mode");
  screen_obj.setCursor(63, 5);
  screen_obj.print("Setting");
  screen_obj.setCursor(117, 5);
  screen_obj.print("Fire Rate");
  screen_obj.setCursor(176, 5);
  screen_obj.print("Idle");

  screen_obj.setCursor(3, 20);
  screen_obj.print("1");
  screen_obj.setCursor(3, 36);
  screen_obj.print("2");
  screen_obj.setCursor(3, 52);
  screen_obj.print("3");
}

void drawFireControlInfo(int selectableFireModes[3], int selectableBurstAmounts[3], int selectablemaxFireRates[3], bool selectableUseIdle[3], int idlePossitionLevel) {
  screen_obj.setCursor(15, 20);
  screen_obj.print(fireModeStr[selectableFireModes[0]]);
  screen_obj.setCursor(15, 36);
  screen_obj.print(fireModeStr[selectableFireModes[1]]);
  screen_obj.setCursor(15, 52);
  screen_obj.print(fireModeStr[selectableFireModes[2]]);

  screen_obj.setCursor(63, 20);
  if (selectableBurstAmounts[0] <= 0){
    screen_obj.print("N/A");
  } else {
    screen_obj.print(selectableBurstAmounts[0], 1);
  }

  screen_obj.setCursor(63, 36);
  if (selectableBurstAmounts[1] <= 0){
    screen_obj.print("N/A");
  } else {
    screen_obj.print(selectableBurstAmounts[1], 1);
  }    

  screen_obj.setCursor(63, 52);
  if (selectableBurstAmounts[2] <= 0){
    screen_obj.print("N/A");
  } else {
    screen_obj.print(selectableBurstAmounts[2], 1);
  }

  screen_obj.setCursor(117, 20);
  screen_obj.print(selectablemaxFireRates[0]);
  screen_obj.setCursor(117, 36);
  screen_obj.print(selectablemaxFireRates[0]);
  screen_obj.setCursor(117, 52);
  screen_obj.print(selectablemaxFireRates[0]);
}

// screen functions 
void clear_screen() {
  screen_obj.fillScreen(ILI9341_BLACK);
}


void drawTestPattern(int radius, int background_color, int filled_color, int line_color) {
  int x, y, reducer;
  int r2 = radius * 2;
  int w = screen_obj.width()  + radius;
  int h = screen_obj.height() + radius;
  static int line_thickness = 5;

  screen_obj.fillScreen(background_color);
  for(x=radius; x<w; x+=r2) {
    for(y=radius; y<h; y+=r2) {
      screen_obj.fillCircle(x, y, radius, filled_color);
    }
  }

  for(x=0; x<w; x+=r2) {
    for(y=0; y<h; y+=r2) {
      for (reducer=0; reducer<line_thickness; reducer+=1){
        screen_obj.drawCircle(x, y, radius-reducer, line_color);
      }
    }
  }
  screen_obj.setTextColor(ILI9341_GREEN);  
  screen_obj.setTextSize(5);

  set_for_centered_text(9, 5);
  screen_obj.println("M.E.D.I.C");
}

void set_for_centered_text(int length, int text_size) {
  static int center_x = screen_obj.width()  / 2 - 1;
  static int center_y = screen_obj.height()  / 2 - 1;

  int total_lenght = ((length + 1) * TEXT_CHARACTER_WIDTH * text_size);
  int total_height = (TEXT_CHARACTER_HEIGHT * text_size);

  int start_location_x = center_x - (total_lenght / 2);
  int start_location_y = center_y - (total_height / 2);

  screen_obj.setCursor(start_location_x, start_location_y);
}

void invertSection(int x1, int y1, int x2, int y2) {
    if ((x1 >= x2) or (y1 >= y2)) {
        return;
    }
    // TODO make work on ILI9341
    /*for (int i = x1; i < x2; i++) {
        for (int j = y1; j < y2; j++) {
            if (_screen_obj.gitPixel(i, j)) {
                _screen_obj.drawPixel(i, j, ILI9341_BLACK);
            } else {
                _screen_obj.drawPixel(i, j, ILI9341_WHITE);
            }
        }
    }*/
}

void drawQuestionBox(char *question){
    screen_obj.setTextColor(ILI9341_BLACK, ILI9341_WHITE);  // init text settings
    screen_obj.setTextSize(1);
    int16_t x1;
    int16_t y1;
    uint16_t length;
    uint16_t height;
    screen_obj.getTextBounds(question, 0, 0, &x1, &y1, &length, &height);
    if (length < 72) {
        length = 72;
    }
    x1 = (width - length) / 2;
    y1 = (height - height) / 2;
    screen_obj.fillRect(x1 - 2, y1 - 2, length + 4, 2*height + 6, ILI9341_WHITE);
    screen_obj.setCursor(x1, y1);
    screen_obj.print(question);

    y1 += (height + 2);
    screen_obj.setCursor(x1, y1);
    screen_obj.print("YES:");
    screen_obj.write(0x18);
    screen_obj.print(" NO:");
    screen_obj.write(0x19);
}

void drawVersionBackgrond() {
  screen_obj.fillScreen(ILI9341_BLACK);  // clear screen
  screen_obj.drawRect(0, 0, width, height, ILI9341_WHITE);  // boarder rect
  screen_obj.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // init text settings
  screen_obj.setTextSize(VERSION_TEXT_SIZE);
  int start_location = 3;

  screen_obj.setCursor(82, start_location);
  screen_obj.print("Version");
  start_location += (VERSION_TEXT_SIZE * TEXT_CHARACTER_HEIGHT) + 3;
  screen_obj.setCursor(3, start_location);
  screen_obj.print("Controller  :");
  start_location += (VERSION_TEXT_SIZE * TEXT_CHARACTER_HEIGHT) + 3;
  screen_obj.setCursor(3, start_location);
  screen_obj.print("Power Board :");
  start_location += (VERSION_TEXT_SIZE * TEXT_CHARACTER_HEIGHT) + 3;
  screen_obj.setCursor(3, start_location);
  screen_obj.print("Fire Control:");
  start_location += (VERSION_TEXT_SIZE * TEXT_CHARACTER_HEIGHT) + 3;
  screen_obj.setCursor(3, start_location);
  screen_obj.print("Chrono      :");
}

void drawVersionInfo(char *ControllerVersion, char *powerBoardVersion, char *FireControlVersion, char *ChronoVersion) {
    screen_obj.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // init text settings
    screen_obj.setTextSize(VERSION_TEXT_SIZE);
    int start_location = 3;

    start_location += (VERSION_TEXT_SIZE * TEXT_CHARACTER_HEIGHT) + 3;
    screen_obj.setCursor(240, start_location);
    screen_obj.print(ControllerVersion);
    start_location += (VERSION_TEXT_SIZE * TEXT_CHARACTER_HEIGHT) + 3;
    screen_obj.setCursor(240, start_location);    
    screen_obj.print(powerBoardVersion);
    start_location += (VERSION_TEXT_SIZE * TEXT_CHARACTER_HEIGHT) + 3;
    screen_obj.setCursor(240, start_location);    
    screen_obj.print(FireControlVersion);
    start_location += (VERSION_TEXT_SIZE * TEXT_CHARACTER_HEIGHT) + 3;
    screen_obj.setCursor(240, start_location);
    screen_obj.print(ChronoVersion);
}

void drawChronoBackgrond() {
    screen_obj.fillScreen(ILI9341_BLACK);  // clear screen
    screen_obj.drawRect(0, 0, width, height, ILI9341_WHITE);  // boarder rect
    screen_obj.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // init text settings
    screen_obj.setTextSize(1);

    // draw lines
    screen_obj.drawFastHLine(0, 32, width, ILI9341_WHITE);
    screen_obj.drawFastHLine(0, 48, width, ILI9341_WHITE);
    screen_obj.drawFastVLine(32, 64, 32, ILI9341_WHITE);

    screen_obj.setCursor(20, 6);
    screen_obj.print("Last FPS:");

    // FPS
    screen_obj.setCursor(20, 22);
    screen_obj.print("FPS");
    screen_obj.setCursor(2, 35);
    screen_obj.print("Max:");
    screen_obj.setCursor(2, 51);
    screen_obj.print("Min:");

    // DPS
    screen_obj.setCursor(90, 22);
    screen_obj.print("DPS");
    screen_obj.setCursor(66, 35);
    screen_obj.print("Last:");
    screen_obj.setCursor(66, 51);
    screen_obj.print("Max:");
}

void drawChronoInfo(float lastFPS, float maxFPS, float minFPS, float lastDPS, float maxDPS) {
    screen_obj.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // init text settings
    screen_obj.setTextSize(1);

    screen_obj.setCursor(75, 6);
    screen_obj.print(lastFPS, 1);
    Serial.println(lastFPS);

    screen_obj.setCursor(27, 35);
    screen_obj.print(maxFPS, 1);
    screen_obj.setCursor(27, 51);
    screen_obj.print(minFPS, 1);

    screen_obj.setCursor(96, 35);
    screen_obj.print(lastDPS, 1);
    screen_obj.setCursor(91, 51);
    screen_obj.print(maxDPS, 1);
}



void addOutline(int x, int y, bool isWhite) { // TODO Fix
    /*if (isWhite){
        _screen_obj.drawRect(x1[x], y1[y], outlineWidth[x], outlineHeight, ILI9341_WHITE);
    } else {
        _screen_obj.drawRect(x1[x], y1[y], outlineWidth[x], outlineHeight, ILI9341_BLACK);
    }*/
}
