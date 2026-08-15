#ifndef Controller_h
#define Controller_h

#define STMPE_CS 6
#define TFT_CS   9
#define TFT_DC   10
#define SD_CS    5

#define TEXT_CHARACTER_HEIGHT 7
#define TEXT_CHARACTER_WIDTH 5 

#define VERSION_TEXT_SIZE 3


// keypad
#define ENCA 12
#define ENCB 11
//#define COMA 11 // grounded by design
#define SW1 A0
#define SW2 A1
#define SW3 A2
#define SW4 A3
#define SW5 A4
//#define COMB A5

#define PIN_ENCODER_A ENCA
#define PIN_ENCODER_B ENCB
#define BUTTON_IN SW1


#define HEADER_DIRECTION 0  // 0 Bottom, 1 Left, 2 Top, 3 Right
#if HEADER_DIRECTION == 0
  #define BUTTON_UP SW2
  #define BUTTON_LEFT SW3
  #define BUTTON_DOWN SW4
  #define BUTTON_RIGHT SW5
#elif HEADER_DIRECTION == 1
  #define BUTTON_UP SW5
  #define BUTTON_LEFT SW2
  #define BUTTON_DOWN SW3
  #define BUTTON_RIGHT SW4
#elif HEADER_DIRECTION == 2
  #define BUTTON_UP SW4
  #define BUTTON_LEFT SW5
  #define BUTTON_DOWN SW2
  #define BUTTON_RIGHT SW3
#elif HEADER_DIRECTION == 3
  #define BUTTON_UP SW3
  #define BUTTON_LEFT SW4
  #define BUTTON_DOWN SW5
  #define BUTTON_RIGHT SW2
#endif


enum POSSITIONS {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  IN,
  ROTARY_CLOCKWISE,
  ROTARY_COUNTERCLOCKWISE,
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

POSSITIONS readKeypad(void);

void updateConnectedDevices(void);

void updateVersionScreen(void);
void editVersionScreen(void);

void updateChronoStatusScreen(void);
void editChronoStatusScreen(void);

void updateFireModeScreen(void);
void editFireModeScreen(void);

void findNextValidScreen(bool countUp);


// screen
void clear_screen();
void drawTestPattern(int radius, int background_color, int filled_color, int line_color);
void set_for_centered_text(int length, int text_size);

void updateVersionScreen(void);
void editVersionScreen(void);
void drawVersionBackgrond();
void drawVersionInfo(char *ControllerVersion, char *powerBoardVersion, char *FireControlVersion, char *ChronoVersion);

void updateChronoStatusScreen(void);
void editChronoStatusScreen(void);
void drawChronoBackgrond();
void drawChronoInfo(float lastFPS, float maxFPS, float minFPS, float lastDPS, float maxDPS);

void updateFireModeScreen(void);
void editFireModeScreen(void);
void drawFireControlBackgrond();
void drawFireControlInfo(int selectableFireModes[3], int selectableBurstAmounts[3], int selectablemaxFireRates[3], 
                         bool selectableUseIdle[3], int idlePossitionLevel);

void invertSection(int x1, int y1, int x2, int y2);
void drawQuestionBox(char *question);

void addOutline(int x, int y, bool isWhite);


#endif