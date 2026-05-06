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

    redrawHomePlaybackInfo();
}

void UI::redrawHomePlaybackInfo() {
    tft.fillRect(58, 38, 68, 40, ST77XX_BLACK);
    tft.fillRect(0, 136, 128, 12, ST77XX_BLACK);

    // Status
    tft.setCursor(58, 50);
    if (isAudioPlaying) {
        tft.setTextColor(ST77XX_GREEN);
        tft.print("Playing");
    } else {
        tft.setTextColor(ST77XX_YELLOW);
        tft.print("Stopped");
    }

    // Progress bar frame
    tft.setTextColor(ST77XX_WHITE);
    tft.drawRect(58, 63, 60, 5, ST77XX_WHITE);

    // Bottom control buttons
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(15, 138);
    tft.print("Prev");
    tft.setCursor(55, 138);
    isAudioPlaying ? tft.print("Pause") : tft.print("Play");
    tft.setCursor(100, 138);
    tft.print("Next");

    lastDisplayedTime = 0;
    titleScrollOffset = 0;
    updateTitleScroll();
    updateProgressBar();
}

void UI::updateTitleScroll() {
    const char* displayTitle = currentPlayingFile.c_str();
    int titleLen = currentPlayingFile.length();

    if (titleLen == 0) {
        displayTitle = "No Song";
        titleLen = 7;
    }

    // Clear title area
    tft.fillRect(58, 38, 68, 8, ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(58, 38);

    // If title fits, just display it
    if (titleLen <= 11) {
        tft.print(displayTitle);
        titleScrollOffset = 0;
        return;
    }

    // Scrolling effect for long titles
    const int scrollLen = titleLen + 3;
    for (int i = 0; i < 11 && i < scrollLen; i++) {
        int charIndex = (titleScrollOffset + i) % scrollLen;
        if (charIndex < titleLen) {
            tft.write(displayTitle[charIndex]);
        } else {
            tft.write(' ');
        }
    }

    titleScrollOffset = (titleScrollOffset + 1) % scrollLen;
}

void UI::updateProgressBar() {
    const uint32_t currentTime = audioPlayer.getCurrentTime();
    const uint32_t duration = audioPlayer.getDuration();

    // Only update if time changed
    if (currentTime == lastDisplayedTime && duration > 0) {
        return;
    }
    lastDisplayedTime = currentTime;

    // Clear progress bar area (inside the frame)
    tft.fillRect(59, 64, 58, 3, ST77XX_BLACK);

    // Draw progress
    if (duration > 0) {
        const int progressWidth = (currentTime * 58) / duration;
        if (progressWidth > 0 && progressWidth <= 58) {
            tft.fillRect(59, 64, progressWidth, 3, ST77XX_WHITE);
        }
    }

    // Clear and update time display
    tft.fillRect(58, 72, 68, 8, ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);

    // Current time
    tft.setCursor(58, 72);
    const int currentMin = currentTime / 60;
    const int currentSec = currentTime % 60;
    tft.print(currentMin);
    tft.print(':');
    if (currentSec < 10) tft.print('0');
    tft.print(currentSec);

    // Duration
    tft.setCursor(100, 72);
    const int durationMin = duration / 60;
    const int durationSec = duration % 60;
    tft.print(durationMin);
    tft.print(':');
    if (durationSec < 10) tft.print('0');
    tft.print(durationSec);
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
            moveSelection(0, 1);
            lastMoveMs = now;
        } else if (digitalRead(JOY_PIN_L) == LOW) {
            moveSelection(1, 0);
            lastMoveMs = now;
        }
    }

    const bool joyMPressed = digitalRead(JOY_PIN_M) == LOW;
    if (joyMPressed && !joyMWasPressed && now - lastSelectMs > JOY_SELECT_DEBOUNCE_MS) {
        if (pendingHomeSelect && (now - lastJoyMClickMs <= JOY_DOUBLECLICK_MS)) {
            pendingHomeSelect = false;
            togglePlayPause();
        } else {
            pendingHomeSelect = true;
            lastJoyMClickMs = now;
        }
        lastSelectMs = now;
    }
    joyMWasPressed = joyMPressed;

    if (pendingHomeSelect && (now - lastJoyMClickMs > JOY_DOUBLECLICK_MS)) {
        pendingHomeSelect = false;
        if (selectedMenuIndex == 0) {
            openFilesScreen();
            return;
        }

        Serial.print("Selected menu: ");
        Serial.println(menuName(selectedMenuIndex));
    }
}

void UI::handleTrackButtons(unsigned long now) {
    const bool prevPressed = digitalRead(BTN_PREV_PIN) == LOW;
    if (prevPressed && !prevBtnWasPressed && now - lastPrevBtnMs > BTN_DEBOUNCE_MS) {
        playPreviousTrack();
        lastPrevBtnMs = now;
    }
    prevBtnWasPressed = prevPressed;

    const bool nextPressed = digitalRead(BTN_NEXT_PIN) == LOW;
    if (nextPressed && !nextBtnWasPressed && now - lastNextBtnMs > BTN_DEBOUNCE_MS) {
        playNextTrack();
        lastNextBtnMs = now;
    }
    nextBtnWasPressed = nextPressed;
}

void UI::playNextTrack() {
    if (mediaFileCount == 0) {
        loadMediaFilesFromSd();
        if (mediaFileCount == 0) return;
    }

    selectedFileIndex = (selectedFileIndex + 1) % mediaFileCount;
    currentPlayingFile = mediaFiles[selectedFileIndex];
    audioPlayer.playFile(currentPlayingFile);
    isAudioPlaying = true;

    if (currentScreen == Screen::Home) {
        redrawHomePlaybackInfo();
    }
}

void UI::playPreviousTrack() {
    if (mediaFileCount == 0) {
        loadMediaFilesFromSd();
        if (mediaFileCount == 0) return;
    }

    selectedFileIndex = (selectedFileIndex > 0) ? (selectedFileIndex - 1) : (mediaFileCount - 1);
    currentPlayingFile = mediaFiles[selectedFileIndex];
    audioPlayer.playFile(currentPlayingFile);
    isAudioPlaying = true;

    if (currentScreen == Screen::Home) {
        redrawHomePlaybackInfo();
    }
}

void UI::togglePlayPause() {
    if (isAudioPlaying) {
        audioPlayer.pause();
        isAudioPlaying = false;
    } else {
        if (currentPlayingFile.length() == 0) {
            if (mediaFileCount == 0) {
                loadMediaFilesFromSd();
            }
            if (mediaFileCount == 0) {
                return;
            }

            selectedFileIndex = selectedFileIndex % mediaFileCount;
            currentPlayingFile = mediaFiles[selectedFileIndex];
            audioPlayer.playFile(currentPlayingFile);
        } else {
            audioPlayer.resume();
        }
        isAudioPlaying = true;
    }

    if (currentScreen == Screen::Home) {
        redrawHomePlaybackInfo();
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
