#ifndef MEDIC_Screens_cpp
#define MEDIC_Screens_cpp

#include "MEDIC_Screens.h"
#include "Arduino.h"
#include <Adafruit_SSD1306.h>
#include "FireControl/FireControlStructsEnums.h"


Screen::Screen(Adafruit_SSD1306 screen_obj) {
    _screen_obj = screen_obj;
    _width = _screen_obj.width();
    _height = _screen_obj.height();
}

void Screen::drawTestPattern(void) {
  _screen_obj.clearDisplay();

  for(int16_t i=max(_screen_obj.width(), _screen_obj.height())/2; i>0; i-=3) {
    // The INVERSE color is used so circles alternate white/black
    _screen_obj.fillCircle(_screen_obj.width() / 2, _screen_obj.height() / 2, i, SSD1306_INVERSE);
    _screen_obj.display(); // Update screen with each newly-drawn circle
    delay(1);
  }
}

void Screen::invertSection(int x1, int y1, int x2, int y2) {
    if ((x1 >= x2) or (y1 >= y2)) {
        return;
    }
    for (int i = x1; i < x2; i++) {
        for (int j = y1; j < y2; j++) {
            if (_screen_obj.getPixel(i, j)) {
                _screen_obj.drawPixel(i, j, SSD1306_BLACK);
            } else {
                _screen_obj.drawPixel(i, j, SSD1306_WHITE);
            }
        }
    }
    _screen_obj.display();
}

void Screen::drawQuestionBox(char *question){
    _screen_obj.setTextColor(SSD1306_BLACK, SSD1306_WHITE);  // init text settings
    _screen_obj.setTextSize(1);
    int16_t x1;
    int16_t y1;
    uint16_t length;
    uint16_t height;
    _screen_obj.getTextBounds(question, 0, 0, &x1, &y1, &length, &height);
    if (length < 72) {
        length = 72;
    }
    x1 = (_width - length) / 2;
    y1 = (_height - height) / 2;
    _screen_obj.fillRect(x1 - 2, y1 - 2, length + 4, 2*height + 6, SSD1306_WHITE);
    _screen_obj.setCursor(x1, y1);
    _screen_obj.print(question);

    y1 += (height + 2);
    _screen_obj.setCursor(x1, y1);
    _screen_obj.print("YES:");
    _screen_obj.write(0x18);
    _screen_obj.print(" NO:");
    _screen_obj.write(0x19);

    _screen_obj.display();
}

void Screen::forceScreenDraw() {
    _screen_obj.display();
}


void Version_Screen::drawBackgrond() {
    _screen_obj.fillScreen(SSD1306_BLACK);  // clear screen
    _screen_obj.drawRect(0, 0, _width, _height, SSD1306_WHITE);  // boarder rect
    _screen_obj.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // init text settings
    _screen_obj.setTextSize(1);

    _screen_obj.setCursor(82, 3);
    _screen_obj.print("Version");

    _screen_obj.setCursor(3, 13);
    _screen_obj.print("Controller  :");
    _screen_obj.setCursor(3, 23);
    _screen_obj.print("Power Board :");
    _screen_obj.setCursor(3, 33);
    _screen_obj.print("Fire Control:");
    _screen_obj.setCursor(3, 43);
    _screen_obj.print("Chrono      :");
    _screen_obj.display();
}

void Version_Screen::drawInfo(char *ControllerVersion, char *powerBoardVersion, char *FireControlVersion, char *ChronoVersion) {
    _screen_obj.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // init text settings
    _screen_obj.setTextSize(1);
    _screen_obj.setCursor(82, 13);
    _screen_obj.print(ControllerVersion);
    _screen_obj.setCursor(82, 23);
    _screen_obj.print(powerBoardVersion);
    _screen_obj.setCursor(82, 33);
    _screen_obj.print(FireControlVersion);
    _screen_obj.setCursor(82, 43);
    _screen_obj.print(ChronoVersion);

    _screen_obj.display();
}


void Chrono_Screen::drawBackgrond() {
    _screen_obj.fillScreen(SSD1306_BLACK);  // clear screen
    _screen_obj.drawRect(0, 0, _width, _height, SSD1306_WHITE);  // boarder rect
    _screen_obj.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // init text settings
    _screen_obj.setTextSize(1);

    // draw lines
    _screen_obj.drawFastHLine(0, 32, _width, SSD1306_WHITE);
    _screen_obj.drawFastHLine(0, 48, _width, SSD1306_WHITE);
    _screen_obj.drawFastVLine(32, 64, 32, SSD1306_WHITE);

    _screen_obj.setCursor(20, 6);
    _screen_obj.print("Last FPS:");

    // FPS
    _screen_obj.setCursor(20, 22);
    _screen_obj.print("FPS");
    _screen_obj.setCursor(2, 35);
    _screen_obj.print("Max:");
    _screen_obj.setCursor(2, 51);
    _screen_obj.print("Min:");

    // DPS
    _screen_obj.setCursor(90, 22);
    _screen_obj.print("DPS");
    _screen_obj.setCursor(66, 35);
    _screen_obj.print("Last:");
    _screen_obj.setCursor(66, 51);
    _screen_obj.print("Max:");

    _screen_obj.display();
}

void Chrono_Screen::drawInfo(float lastFPS, float maxFPS, float minFPS, float lastDPS, float maxDPS) {
    _screen_obj.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // init text settings
    _screen_obj.setTextSize(1);

    _screen_obj.setCursor(75, 6);
    _screen_obj.print(lastFPS, 1);
    Serial.println(lastFPS);

    _screen_obj.setCursor(27, 35);
    _screen_obj.print(maxFPS, 1);
    _screen_obj.setCursor(27, 51);
    _screen_obj.print(minFPS, 1);

    _screen_obj.setCursor(96, 35);
    _screen_obj.print(lastDPS, 1);
    _screen_obj.setCursor(91, 51);
    _screen_obj.print(maxDPS, 1);

    _screen_obj.display();
}


void Fire_Control_Screen::drawBackgrond() {
    _screen_obj.fillScreen(SSD1306_BLACK);  // clear screen
    _screen_obj.drawRect(0, 0, _width, _height, SSD1306_WHITE);  // boarder rect
    _screen_obj.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  // init text settings
    _screen_obj.setTextSize(1);

    _screen_obj.drawFastVLine(12, 0, _height, SSD1306_WHITE);
    _screen_obj.drawFastVLine(60, 0, _height, SSD1306_WHITE);
    _screen_obj.drawFastHLine(0, 16, _width, SSD1306_WHITE);
    _screen_obj.drawFastHLine(0, 32, _width, SSD1306_WHITE);
    _screen_obj.drawFastHLine(0, 47, _width, SSD1306_WHITE);

    _screen_obj.setCursor(15, 5);
    _screen_obj.print("Mode");
    _screen_obj.setCursor(63, 5);
    _screen_obj.print("Setting");

    _screen_obj.setCursor(3, 20);
    _screen_obj.print("1");
    _screen_obj.setCursor(3, 36);
    _screen_obj.print("2");
    _screen_obj.setCursor(3, 52);
    _screen_obj.print("3");

    _screen_obj.display();
}

void Fire_Control_Screen::drawInfo(int selectableFireModes[3], int selectableBurstAmounts[3]) {
    _screen_obj.setCursor(15, 20);
    _screen_obj.print(fireModeStr[selectableFireModes[0]]);
    _screen_obj.setCursor(15, 36);
    _screen_obj.print(fireModeStr[selectableFireModes[1]]);
    _screen_obj.setCursor(15, 52);
    _screen_obj.print(fireModeStr[selectableFireModes[2]]);

    _screen_obj.setCursor(63, 20);
    _screen_obj.print(selectableBurstAmounts[0], 1);
    _screen_obj.setCursor(63, 36);
    _screen_obj.print(selectableBurstAmounts[1], 1);
    _screen_obj.setCursor(63, 52);
    _screen_obj.print(selectableBurstAmounts[2], 1);

    _screen_obj.display();
}

void Fire_Control_Screen::addOutline(int x, int y, bool isWhite) {
    if (isWhite){
        _screen_obj.drawRect(x1[x], y1[y], outlineWidth[x], outlineHeight, SSD1306_WHITE);
    } else {
        _screen_obj.drawRect(x1[x], y1[y], outlineWidth[x], outlineHeight, SSD1306_BLACK);
    }
}


#endif