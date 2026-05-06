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
    pinMode(BTN_PREV_PIN, INPUT_PULLUP);
    pinMode(BTN_NEXT_PIN, INPUT_PULLUP);

    sdReady = SD.begin(SD_CS);
    if (!sdReady) {
        Serial.println("SD init failed");
    } else {
        Serial.println("SD init OK");
    }

    tft.initR(DISPLAY_INITR_TAB);
    tft.setRotation(DISPLAY_ROTATION);

    returnToHomeScreen();
}

void UI::update() {
    const unsigned long now = millis();
    handleTrackButtons(now);

    if (currentScreen == Screen::Home) {
        handleHomeInput(now);

        // Update progress bar every 500ms when playing
        if (isAudioPlaying && now - lastProgressUpdateMs >= 500) {
            lastProgressUpdateMs = now;
            updateProgressBar();
        }

        // Update scrolling title every 300ms
        if (now - lastTitleScrollMs >= 300) {
            lastTitleScrollMs = now;
            updateTitleScroll();
        }
    } else {
        handleFilesInput(now);
    }
}
