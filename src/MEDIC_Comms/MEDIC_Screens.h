#ifndef MEDIC_Screens_h
#define MEDIC_Screens_h

#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "MEDIC_Comms.h"

#define STMPE_CS 6
#define TFT_CS   9
#define TFT_DC   10
#define SD_CS    5

#define TEXT_CHARACTER_HEIGHT 7
#define TEXT_CHARACTER_WIDTH 5 

class Screen_ILI9341 {
    public:
        Screen_ILI9341(void);
        void clear_screen();

        void drawTestPattern(int radius, int background_color, int filled_color, int line_color);
        void set_for_centered_text(int length, int text_size);
        void invertSection(int x1, int y1, int x2, int y2);
        void drawQuestionBox(char *question);

        // Version
        void drawVersionBackgrond();
        void drawVersionInfo(char *ControllerVersion, char *powerBoardVersion, char *FireControlVersion, char *ChronoVersion);

        // Chrono
        void drawChronoBackgrond();
        void drawChronoInfo(float lastFPS, float maxFPS, float minFPS, float lastDPS, float maxDPS);

        // Fire Control
        void drawFireControlBackgrond();
        void drawFireControlInfo(int selectableFireModes[3], int selectableBurstAmounts[3]);
        void addOutline(int x, int y, bool isWhite);
        /*
        unsigned x1[2] = {14, 62};
        //int x2[2] = {58, 125};
        unsigned int outlineWidth[2] = {45, 64};
        unsigned int outlineHeight = 12;
        unsigned int y1[3] = {19, 34, 49};
        */

    protected:
        // Adafruit_ILI9341 _screen_obj;
        int _width;
        int _height;

    private:

};

#endif