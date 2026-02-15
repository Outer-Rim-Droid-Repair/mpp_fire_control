#ifndef MEDIC_Screens_h
#define MEDIC_Screens_h

#include "Arduino.h"
#include <Adafruit_SSD1306.h>
#include "MEDIC_Comms.h"


class Screen {
    public:
        Screen() {};
        Screen(Adafruit_SSD1306 screen_obj);
        virtual void drawBackgrond() {}
        void drawInfo() {}

        void drawTestPattern(void);
        void invertSection(int x1, int y1, int x2, int y2);
        void drawQuestionBox(char *question);
        void forceScreenDraw();

    protected:
        Adafruit_SSD1306 _screen_obj;
        int _width;
        int _height;

    private:

};

class Version_Screen: public Screen {
    public:
        Version_Screen(): Screen() {}
        Version_Screen(Adafruit_SSD1306 screen_obj): Screen(screen_obj) {}
        void drawBackgrond() override;
        void drawInfo(char *ControllerVersion, char *powerBoardVersion, char *FireControlVersion, char *ChronoVersion);

    };

class Chrono_Screen: public Screen {
    public: 
        Chrono_Screen(): Screen() {}
        Chrono_Screen(Adafruit_SSD1306 screen_obj): Screen(screen_obj) {}
        void drawBackgrond() override;
        void drawInfo(float lastFPS, float maxFPS, float minFPS, float lastDPS, float maxDPS);

};

class Fire_Control_Screen: public Screen {
    public:
        Fire_Control_Screen(): Screen() {}
        Fire_Control_Screen(Adafruit_SSD1306 screen_obj): Screen(screen_obj) {}
        void drawBackgrond() override;
        void drawInfo(int selectableFireModes[3], int selectableBurstAmounts[3]);
        void addOutline(int x, int y, bool isWhite);
        unsigned x1[2] = {14, 62};
        //int x2[2] = {58, 125};
        unsigned int outlineWidth[2] = {45, 64};
        unsigned int outlineHeight = 12;
        unsigned int y1[3] = {19, 34, 49};
        //int y2[3] = {37, 53, 61};
        

};

#endif