#include "interface.h"
#include <SPI.h>

UI ui;

namespace {
const int kBtnW = 55;
const int kBtnH = 18;

const int kBtnX[4] = {8, 68, 8, 68};
const int kBtnY[4] = {90, 90, 112, 112};
const char* kBtnText[4] = {"Files", "Playlist", "Radio", "Setting"};
}

void UI::begin() {
    Serial.begin(SERIAL_BAUD);
    Serial.println("Init display...");

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    pinMode(JOY_PIN_F, INPUT_PULLUP);
    pinMode(JOY_PIN_B, INPUT_PULLUP);
    pinMode(JOY_PIN_L, INPUT_PULLUP);
    pinMode(JOY_PIN_R, INPUT_PULLUP);
    pinMode(JOY_PIN_M, INPUT_PULLUP);

    drawInterface();
    drawMenuButtons();
}

void UI::drawInterface() {

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

    // Bottom control buttons row
    tft.setCursor(20, 138);
    tft.print("<<");
    tft.setCursor(60, 138);
    tft.print(">");
    tft.setCursor(100, 138);
    tft.print(">>");
}

void UI::drawMenuButton(uint8_t index, bool selected) {
    const uint16_t frameColor = selected ? ST77XX_YELLOW : ST77XX_WHITE;
    const int x = kBtnX[index];
    const int y = kBtnY[index];

    tft.drawRect(x, y, kBtnW, kBtnH, frameColor);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

    int textX = x + 4;
    if (index == 0) textX = x + 11;
    if (index == 2) textX = x + 12;
    if (index == 3) textX = x + 8;

    tft.setCursor(textX, y + 6);
    tft.print(kBtnText[index]);
}

void UI::drawMenuButtons() {
    for (uint8_t i = 0; i < 4; i++) {
        drawMenuButton(i, i == selectedMenuIndex);
    }
}

void UI::moveSelection(int8_t dRow, int8_t dCol) {
    int row = selectedMenuIndex / 2;
    int col = selectedMenuIndex % 2;

    row += dRow;
    col += dCol;

    if (row < 0) row = 0;
    if (row > 1) row = 1;
    if (col < 0) col = 0;
    if (col > 1) col = 1;

    const uint8_t newIndex = static_cast<uint8_t>(row * 2 + col);
    if (newIndex == selectedMenuIndex) {
        return;
    }

    const uint8_t oldIndex = selectedMenuIndex;
    selectedMenuIndex = newIndex;
    drawMenuButton(oldIndex, false);
    drawMenuButton(selectedMenuIndex, true);
}

void UI::update() {
    const unsigned long now = millis();

    if (now - lastMoveMs > JOY_DEBOUNCE_MS) {
        if (digitalRead(JOY_PIN_R) == LOW) {
            moveSelection(-1, 0);
            lastMoveMs = now;
        } else if (digitalRead(JOY_PIN_F) == LOW) {
            moveSelection(0, -1);
            lastMoveMs = now;
        } else if (digitalRead(JOY_PIN_B) == LOW) {
            moveSelection(0, 1);
            lastMoveMs = now;
        } else if (digitalRead(JOY_PIN_L) == LOW) {
            moveSelection(1, 0);
            lastMoveMs = now;
        }
    }

    if (now - lastSelectMs > JOY_SELECT_DEBOUNCE_MS && digitalRead(JOY_PIN_M) == LOW) {
        lastSelectMs = now;
        Serial.print("Selected menu: ");
        Serial.println(kBtnText[selectedMenuIndex]);
    }
}
