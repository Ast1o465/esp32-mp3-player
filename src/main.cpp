#include "config.h"
#include "interface.h"

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  ui.drawInterface();
}

void loop() {

}
