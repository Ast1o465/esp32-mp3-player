#include "interface.h"
#include <SPI.h>

UI ui;

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

    sdReady = SD.begin(SD_CS);
    if (!sdReady) {
        Serial.println("SD init failed");
    } else {
        Serial.println("SD init OK");
    }

    drawInterface();
    drawMenuButtons();
}

void UI::update() {
    const unsigned long now = millis();

    if (currentScreen == Screen::Home) {
        handleHomeInput(now);
    } else {
        handleFilesInput(now);
    }
}
