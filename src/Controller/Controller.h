#ifndef Controller_h
#define Controller_h

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// keypad
#define BUTTON_UP 5
#define BUTTON_LEFT 6
#define BUTTON_DOWN 9
#define BUTTON_RIGHT 10
#define BUTTON_IN 11

enum POSSITIONS {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  IN,
  ROTARY,
  NONE
};
POSSITIONS lastPressed = NONE;

enum SCREEN_STATE {
    VERSION,
    CHRONO_STATUS,
    FIRE_MODE_STATUS,
    POWER_STATUS,
    OTHER
};
const SCREEN_STATE screenOrder[] = {VERSION, CHRONO_STATUS, FIRE_MODE_STATUS, POWER_STATUS};
SCREEN_STATE selectedScreenState = VERSION;
SCREEN_STATE currentScreenState = OTHER;

enum EDIT_MODES {
    INIT,
    MODIFY,
    CONFIRM
};
EDIT_MODES lastEditMode = INIT;


bool powerBoardPresent;
bool fireControlPresent;
bool chronoPresent;

void readKeypad(void);

void updateConnectedDevices(void);

void updateVersionScreen(void);
void editVersionScreen(void);

void updateChronoStatusScreen(void);
void editChronoStatusScreen(void);

void updateFireModeScreen(void);
void editFireModeScreen(void);

void findNextValidScreen(bool countUp);

#endif