#include "interface.h"
#include "player_audio.h"

namespace {
const int kBtnW = 55;
const int kBtnH = 18;
const int kBtnX[4] = {8, 68, 8, 68};
const int kBtnY[4] = {90, 90, 112, 112};
}

const char* UI::menuName(uint8_t index) const {
    static const char* kBtnText[4] = {"Files", "Playlist", "Radio", "Setting"};
    return kBtnText[index];
}

void UI::drawInterface() {
    tft.initR(DISPLAY_INITR_TAB);
    tft.setRotation(DISPLAY_ROTATION);
    tft.fillScreen(ST77XX_BLACK);

    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);

    tft.setCursor(35, 15);
    tft.print("My Player");

    // Song cover
    tft.drawRect(10, 35, 40, 40, ST77XX_WHITE);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(14, 55);
    tft.print("AUDIO");

    // Song title (show playing file)
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(58, 38);
    String displayTitle = currentPlayingFile;
    if (displayTitle.length() > 18) {
        displayTitle = displayTitle.substring(0, 15) + "...";
    }
    if (displayTitle.length() == 0) {
        displayTitle = "No Song";
    }
    tft.print(displayTitle);

    // Status
    tft.setCursor(58, 50);
    if (isAudioPlaying) {
        tft.setTextColor(ST77XX_GREEN);
        tft.print("Playing");
    } else {
        tft.setTextColor(ST77XX_YELLOW);
        tft.print("Stopped");
    }

    // Progress bar
    tft.setTextColor(ST77XX_WHITE);
    tft.drawRect(58, 63, 60, 5, ST77XX_WHITE);
    if (isAudioPlaying) {
        tft.fillRect(58, 63, 30, 5, ST77XX_WHITE);
    }

    // Timers
    tft.setCursor(58, 72);
    tft.print("0:24");
    tft.setCursor(100, 72);
    tft.print("3:45");

    // Bottom control buttons
    tft.setCursor(15, 138);
    tft.print("Prev");
    tft.setCursor(55, 138);
    isAudioPlaying ? tft.print("Pause") : tft.print("Play");
    tft.setCursor(100, 138);
    tft.print("Next");
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
    tft.print(menuName(index));
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

void UI::handleHomeInput(unsigned long now) {
    if (now - lastMoveMs > JOY_DEBOUNCE_MS) {
        if (digitalRead(JOY_PIN_R) == LOW) {
            moveSelection(-1, 0);
            lastMoveMs = now;
        } else if (digitalRead(JOY_PIN_F) == LOW) {
            moveSelection(0, -1);
            lastMoveMs = now;
        } else if (digitalRead(JOY_PIN_B) == LOW) {
            // Next song
            if (mediaFileCount > 0) {
                selectedFileIndex = (selectedFileIndex + 1) % mediaFileCount;
                currentPlayingFile = mediaFiles[selectedFileIndex];
                audioPlayer.playFile(currentPlayingFile);
                isAudioPlaying = true;
                drawInterface();
                drawMenuButtons();
            }
            lastMoveMs = now;
        } else if (digitalRead(JOY_PIN_L) == LOW) {
            // Previous song
            if (mediaFileCount > 0) {
                selectedFileIndex = (selectedFileIndex > 0) ? (selectedFileIndex - 1) : (mediaFileCount - 1);
                currentPlayingFile = mediaFiles[selectedFileIndex];
                audioPlayer.playFile(currentPlayingFile);
                isAudioPlaying = true;
                drawInterface();
                drawMenuButtons();
            }
            lastMoveMs = now;
        }
    }

    if (now - lastSelectMs > JOY_SELECT_DEBOUNCE_MS && digitalRead(JOY_PIN_M) == LOW) {
        lastSelectMs = now;

        if (selectedMenuIndex == 0) {
            openFilesScreen();
            return;
        }

        Serial.print("Selected menu: ");
        Serial.println(menuName(selectedMenuIndex));
    }
}

void UI::returnToHomeScreen() {
    currentScreen = Screen::Home;
    
    // Load media files if not already loaded
    if (mediaFileCount == 0) {
        loadMediaFilesFromSd();
    }
    
    // Pick random song if available
    if (mediaFileCount > 0 && currentPlayingFile.length() == 0) {
        selectedFileIndex = random(mediaFileCount);
        currentPlayingFile = mediaFiles[selectedFileIndex];
        audioPlayer.playFile(currentPlayingFile);
        isAudioPlaying = true;
    }
    
    drawInterface();
    drawMenuButtons();
}
