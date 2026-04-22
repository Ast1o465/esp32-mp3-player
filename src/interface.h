#ifndef INTERFACE_H
#define INTERFACE_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "config.h"

extern Adafruit_ST7735 tft;

class UI {
public:
	void begin();
	void update();

private:
	uint8_t selectedMenuIndex = 0;
	unsigned long lastMoveMs = 0;
	unsigned long lastSelectMs = 0;

	void drawInterface();
	void drawMenuButtons();
	void drawMenuButton(uint8_t index, bool selected);
	void moveSelection(int8_t dRow, int8_t dCol);
};

extern UI ui;

#endif // INTERFACE_H

