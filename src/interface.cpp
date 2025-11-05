#include "interface.h"
#include <SPI.h>

UI ui;

void UI::drawInterface() {
    Serial.begin(SERIAL_BAUD);
    Serial.println("Init display...");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    tft.initR(DISPLAY_INITR_TAB);
    tft.setRotation(DISPLAY_ROTATION);
    tft.fillScreen(ST77XX_BLACK);

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);

    // Header
    tft.setCursor(35, 15);
    tft.print("My Player");

    // Song cover
    tft.drawRect(10, 35, 40, 40, ST77XX_WHITE);

    // Song title
    tft.setCursor(58, 38);
    tft.print("Song Name");

    // Artist
    tft.setCursor(58, 50);
    tft.print("Artist");

    // Progress bar
    tft.drawRect(58, 63, 60, 5, ST77XX_WHITE);
    tft.fillRect(58, 63, 25, 5, ST77XX_WHITE);

    // Timers
    tft.setCursor(58, 72);
    tft.print("0:24");
    tft.setCursor(100, 72);
    tft.print("3:45");

    // Menu buttons (2x2)
    const int btnW = 55;
    const int btnH = 18;
    
    const int x1 = 8,  y1 = 90;
    const int x2 = 68, y2 = 90;
    const int x3 = 8,  y3 = 112;
    const int x4 = 68, y4 = 112;

    // Button frames
    tft.drawRect(x1, y1, btnW, btnH, ST77XX_WHITE);
    tft.drawRect(x2, y2, btnW, btnH, ST77XX_WHITE);
    tft.drawRect(x3, y3, btnW, btnH, ST77XX_WHITE);
    tft.drawRect(x4, y4, btnW, btnH, ST77XX_WHITE);

    // Text on buttons
    tft.setTextSize(1);
    tft.setCursor(x1 + 11, y1 + 6);
    tft.print("Files");
    tft.setCursor(x2 + 4, y2 + 6);
    tft.print("Playlist");
    tft.setCursor(x3 + 12, y3 + 6);
    tft.print("Radio");
    tft.setCursor(x4 + 8, y4 + 6);
    tft.print("Setting");

    // Bottom control buttons row
    tft.setCursor(20, 138);
    tft.print("<<");
    tft.setCursor(60, 138);
    tft.print(">");
    tft.setCursor(100, 138);
    tft.print(">>");
}
