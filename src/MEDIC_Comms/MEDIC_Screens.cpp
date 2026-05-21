#ifndef MEDIC_Screens_cpp
#define MEDIC_Screens_cpp

#include "MEDIC_Screens.h"
#include "Arduino.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "FireControl/FireControlStructsEnums.h"



Adafruit_ILI9341 _screen_obj = Adafruit_ILI9341(TFT_CS, TFT_DC);

Screen_ILI9341::Screen_ILI9341() {
    _screen_obj.setRotation(1);
    _width = _screen_obj.width();
    _height = _screen_obj.height();
}

void Screen_ILI9341::clear_screen() {
      _screen_obj.fillScreen(ILI9341_BLACK);
}

void Screen_ILI9341::drawTestPattern(int radius, int background_color, int filled_color, int line_color) {
  int x, y, reducer;
  int r2 = radius * 2;
  int w = _screen_obj.width()  + radius;
  int h = _screen_obj.height() + radius;
  static int line_thickness = 5;

  _screen_obj.fillScreen(background_color);
  for(x=radius; x<w; x+=r2) {
    for(y=radius; y<h; y+=r2) {
      _screen_obj.fillCircle(x, y, radius, filled_color);
    }
  }

  for(x=0; x<w; x+=r2) {
    for(y=0; y<h; y+=r2) {
      for (reducer=0; reducer<line_thickness; reducer+=1){
        _screen_obj.drawCircle(x, y, radius-reducer, line_color);
      }
    }
  }
  _screen_obj.setTextColor(ILI9341_GREEN);  
  _screen_obj.setTextSize(5);

  set_for_centered_text(9, 5);
  _screen_obj.println("M.E.D.I.C");
}

void Screen_ILI9341::set_for_centered_text(int length, int text_size) {
  static int center_x = _screen_obj.width()  / 2 - 1;
  static int center_y = _screen_obj.height()  / 2 - 1;

  int total_lenght = ((length + 1) * TEXT_CHARACTER_WIDTH * text_size);
  int total_height = (TEXT_CHARACTER_HEIGHT * text_size);

  int start_location_x = center_x - (total_lenght / 2);
  int start_location_y = center_y - (total_height / 2);

  _screen_obj.setCursor(start_location_x, start_location_y);
}

void Screen_ILI9341::invertSection(int x1, int y1, int x2, int y2) {
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

void Screen_ILI9341::drawQuestionBox(char *question){
    _screen_obj.setTextColor(ILI9341_BLACK, ILI9341_WHITE);  // init text settings
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
    _screen_obj.fillRect(x1 - 2, y1 - 2, length + 4, 2*height + 6, ILI9341_WHITE);
    _screen_obj.setCursor(x1, y1);
    _screen_obj.print(question);

    y1 += (height + 2);
    _screen_obj.setCursor(x1, y1);
    _screen_obj.print("YES:");
    _screen_obj.write(0x18);
    _screen_obj.print(" NO:");
    _screen_obj.write(0x19);
}

void Screen_ILI9341::drawVersionBackgrond() {
    _screen_obj.fillScreen(ILI9341_BLACK);  // clear screen
    _screen_obj.drawRect(0, 0, _width, _height, ILI9341_WHITE);  // boarder rect
    _screen_obj.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // init text settings
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
}

void Screen_ILI9341::drawVersionInfo(char *ControllerVersion, char *powerBoardVersion, char *FireControlVersion, char *ChronoVersion) {
    _screen_obj.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // init text settings
    _screen_obj.setTextSize(1);
    _screen_obj.setCursor(82, 13);
    _screen_obj.print(ControllerVersion);
    _screen_obj.setCursor(82, 23);
    _screen_obj.print(powerBoardVersion);
    _screen_obj.setCursor(82, 33);
    _screen_obj.print(FireControlVersion);
    _screen_obj.setCursor(82, 43);
    _screen_obj.print(ChronoVersion);
}


void Screen_ILI9341::drawChronoBackgrond() {
    _screen_obj.fillScreen(ILI9341_BLACK);  // clear screen
    _screen_obj.drawRect(0, 0, _width, _height, ILI9341_WHITE);  // boarder rect
    _screen_obj.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // init text settings
    _screen_obj.setTextSize(1);

    // draw lines
    _screen_obj.drawFastHLine(0, 32, _width, ILI9341_WHITE);
    _screen_obj.drawFastHLine(0, 48, _width, ILI9341_WHITE);
    _screen_obj.drawFastVLine(32, 64, 32, ILI9341_WHITE);

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
}

void Screen_ILI9341::drawChronoInfo(float lastFPS, float maxFPS, float minFPS, float lastDPS, float maxDPS) {
    _screen_obj.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // init text settings
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
}

void Screen_ILI9341::drawFireControlBackgrond() {
    _screen_obj.fillScreen(ILI9341_BLACK);  // clear screen
    _screen_obj.drawRect(0, 0, _width, _height, ILI9341_WHITE);  // boarder rect
    _screen_obj.setTextColor(ILI9341_WHITE, ILI9341_BLACK);  // init text settings
    _screen_obj.setTextSize(1);

    _screen_obj.drawFastVLine(12, 0, _height, ILI9341_WHITE);
    _screen_obj.drawFastVLine(60, 0, _height, ILI9341_WHITE);
    _screen_obj.drawFastHLine(0, 16, _width, ILI9341_WHITE);
    _screen_obj.drawFastHLine(0, 32, _width, ILI9341_WHITE);
    _screen_obj.drawFastHLine(0, 47, _width, ILI9341_WHITE);

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
}

void Screen_ILI9341::drawFireControlInfo(int selectableFireModes[3], int selectableBurstAmounts[3]) {
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
}

void Screen_ILI9341::addOutline(int x, int y, bool isWhite) {
    /*if (isWhite){
        _screen_obj.drawRect(x1[x], y1[y], outlineWidth[x], outlineHeight, ILI9341_WHITE);
    } else {
        _screen_obj.drawRect(x1[x], y1[y], outlineWidth[x], outlineHeight, ILI9341_BLACK);
    }*/
}


#endif