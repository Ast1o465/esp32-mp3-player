#ifndef INTERFACE_H
#define INTERFACE_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "config.h"

extern Adafruit_ST7735 tft;

class UI {
public:
	void drawInterface();
};

extern UI ui;

#endif // INTERFACE_H

