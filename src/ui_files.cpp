#include "interface.h"

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
    if (now - lastMoveMs > JOY_DEBOUNCE_MS) {
        if (mediaFileCount > 0 && digitalRead(JOY_PIN_R) == LOW) {
            if (selectedFileIndex > 0) {
                selectedFileIndex--;
                if (selectedFileIndex < filesScrollOffset) {
                    filesScrollOffset = selectedFileIndex;
                }
                drawFilesList();
            }
            lastMoveMs = now;
        } else if (mediaFileCount > 0 && digitalRead(JOY_PIN_L) == LOW) {
            if (selectedFileIndex + 1 < mediaFileCount) {
                selectedFileIndex++;
                if (selectedFileIndex >= filesScrollOffset + kVisibleFileRows) {
                    filesScrollOffset = selectedFileIndex - kVisibleFileRows + 1;
                }
                drawFilesList();
            }
            lastMoveMs = now;
        } else if (mediaFileCount > 0 && digitalRead(JOY_PIN_F) == LOW) {
            if (selectedFileIndex > 0) {
                if (selectedFileIndex > kVisibleFileRows) {
                    selectedFileIndex -= kVisibleFileRows;
                } else {
                    selectedFileIndex = 0;
                }
                if (selectedFileIndex < filesScrollOffset) {
                    filesScrollOffset = selectedFileIndex;
                }
                drawFilesList();
            }
            lastMoveMs = now;
        } else if (mediaFileCount > 0 && digitalRead(JOY_PIN_B) == LOW) {
            if (selectedFileIndex + 1 < mediaFileCount) {
                uint8_t newIndex = selectedFileIndex + kVisibleFileRows;
                if (newIndex >= mediaFileCount) {
                    newIndex = mediaFileCount - 1;
                }
                selectedFileIndex = newIndex;
                if (selectedFileIndex >= filesScrollOffset + kVisibleFileRows) {
                    filesScrollOffset = selectedFileIndex - kVisibleFileRows + 1;
                }
                drawFilesList();
            }
            lastMoveMs = now;
        }
    }

    if (now - lastSelectMs > JOY_SELECT_DEBOUNCE_MS && digitalRead(JOY_PIN_M) == LOW) {
        lastSelectMs = now;
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
    String lowerName = fileName;
    lowerName.toLowerCase();
    return lowerName.endsWith(".mp3") || lowerName.endsWith(".mp4") || lowerName.endsWith(".wav");
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
        if (fileIndex >= mediaFileCount) {
            break;
        }

        const bool selected = fileIndex == selectedFileIndex;
        const int y = kFilesListStartY + row * kFileRowHeight;
        if (selected) {
            tft.fillRect(2, y, 124, kFileRowHeight - 1, ST77XX_YELLOW);
            tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
        } else {
            tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
        }

        tft.setCursor(6, y + 4);
        tft.print(shortenFileName(mediaFiles[fileIndex], 20));
    }
}

String UI::shortenFileName(const String& fileName, uint8_t maxChars) const {
    if (fileName.length() <= maxChars) {
        return fileName;
    }

    if (maxChars < 4) {
        return fileName.substring(0, maxChars);
    }

    return fileName.substring(0, maxChars - 3) + "...";
}
