#include "interface.h"
#include "player_audio.h"

namespace {
const uint8_t kVisibleFileRows = 6;
const int kFilesListStartY = 24;
const int kFileRowHeight = 16;
}

void UI::openFilesScreen() {
    currentScreen = Screen::Files;
    loadMediaFilesFromSd();
    drawFilesScreen();
}

void UI::handleFilesInput(unsigned long now) {
    if (mediaFileCount == 0) return;

    if (now - lastMoveMs > JOY_DEBOUNCE_MS) {
        int8_t delta = 0;
        bool pageMove = false;

        if (digitalRead(JOY_PIN_R) == LOW) {
            delta = -1;
        } else if (digitalRead(JOY_PIN_L) == LOW) {
            delta = 1;
        } else if (digitalRead(JOY_PIN_F) == LOW) {
            delta = -kVisibleFileRows;
            pageMove = true;
        } else if (digitalRead(JOY_PIN_B) == LOW) {
            delta = kVisibleFileRows;
            pageMove = true;
        }

        if (delta != 0) {
            int newIndex = selectedFileIndex + delta;
            if (newIndex < 0) newIndex = 0;
            if (newIndex >= mediaFileCount) newIndex = mediaFileCount - 1;

            if (newIndex != selectedFileIndex) {
                selectedFileIndex = newIndex;

                // Adjust scroll offset
                if (selectedFileIndex < filesScrollOffset) {
                    filesScrollOffset = selectedFileIndex;
                } else if (selectedFileIndex >= filesScrollOffset + kVisibleFileRows) {
                    filesScrollOffset = selectedFileIndex - kVisibleFileRows + 1;
                }

                drawFilesList();
            }
            lastMoveMs = now;
        }
    }

    if (now - lastSelectMs > JOY_SELECT_DEBOUNCE_MS && digitalRead(JOY_PIN_M) == LOW) {
        lastSelectMs = now;
        currentPlayingFile = mediaFiles[selectedFileIndex];
        audioPlayer.playFile(currentPlayingFile);
        isAudioPlaying = true;
        returnToHomeScreen();
    }
}

void UI::loadMediaFilesFromSd() {
    mediaFileCount = 0;
    selectedFileIndex = 0;
    filesScrollOffset = 0;

    if (!sdReady) {
        return;
    }

    File root = SD.open("/");
    if (!root || !root.isDirectory()) {
        Serial.println("Cannot open SD root");
        return;
    }

    scanDirectory(root, "");
    root.close();

    Serial.print("Media files found: ");
    Serial.println(mediaFileCount);
}

void UI::scanDirectory(File dir, const String& prefix) {
    while (mediaFileCount < kMaxMediaFiles) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }

        String entryName = String(entry.name());
        if (entry.isDirectory()) {
            scanDirectory(entry, prefix + entryName + "/");
        } else if (isMediaFile(entryName)) {
            mediaFiles[mediaFileCount] = prefix + entryName;
            mediaFileCount++;
        }

        entry.close();
    }
}

bool UI::isMediaFile(const String& fileName) const {
    const int len = fileName.length();
    if (len < 4) return false;

    const char* name = fileName.c_str();
    const char* ext = name + len - 4;

    return (strcasecmp(ext, ".mp3") == 0 ||
            strcasecmp(ext, ".mp4") == 0 ||
            strcasecmp(ext, ".wav") == 0);
}

void UI::drawFilesScreen() {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(6, 6);
    tft.print("Files (.mp3/.mp4/.wav)");

    if (!sdReady) {
        tft.setTextColor(ST77XX_RED);
        tft.setCursor(6, 28);
        tft.print("SD card init failed");
    } else if (mediaFileCount == 0) {
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(6, 28);
        tft.print("No MP3/MP4/WAV");
    } else {
        drawFilesList();
    }

    tft.drawFastHLine(0, 132, 128, ST77XX_WHITE);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(6, 138);
    tft.print("R/L nav F/B page");
    tft.setCursor(6, 148);
    tft.print("M back");
}

void UI::drawFilesList() {
    tft.fillRect(0, kFilesListStartY, 128, kVisibleFileRows * kFileRowHeight, ST77XX_BLACK);

    for (uint8_t row = 0; row < kVisibleFileRows; row++) {
        const uint8_t fileIndex = filesScrollOffset + row;
        if (fileIndex >= mediaFileCount) break;

        const bool selected = (fileIndex == selectedFileIndex);
        const int y = kFilesListStartY + row * kFileRowHeight;

        if (selected) {
            tft.fillRect(2, y, 124, kFileRowHeight - 1, ST77XX_YELLOW);
            tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
        } else {
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        }

        tft.setCursor(6, y + 4);
        const String& fileName = mediaFiles[fileIndex];
        tft.print(shortenFileName(fileName, 20));
    }
}

String UI::shortenFileName(const String& fileName, uint8_t maxChars) const {
    if (fileName.length() <= maxChars) {
        return fileName;
    }
    return fileName.substring(0, maxChars - 3) + "...";
}
