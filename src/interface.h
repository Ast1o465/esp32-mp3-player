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
	enum class Screen : uint8_t {
		Home,
		Files
	};

	Screen currentScreen = Screen::Home;
	uint8_t selectedMenuIndex = 0;
	bool sdReady = false;
	
	String currentPlayingFile;
	bool isAudioPlaying = false;

	static const uint8_t kMaxMediaFiles = 48;
	String mediaFiles[kMaxMediaFiles];
	uint8_t mediaFileCount = 0;
	uint8_t selectedFileIndex = 0;
	uint8_t filesScrollOffset = 0;

	unsigned long lastMoveMs = 0;
	unsigned long lastSelectMs = 0;

	void drawInterface();
	void drawMenuButtons();
	void drawMenuButton(uint8_t index, bool selected);
	void moveSelection(int8_t dRow, int8_t dCol);
	const char* menuName(uint8_t index) const;

	void handleHomeInput(unsigned long now);
	void handleFilesInput(unsigned long now);
	void openFilesScreen();
	void returnToHomeScreen();

	void loadMediaFilesFromSd();
	void scanDirectory(File dir, const String& prefix);
	bool isMediaFile(const String& fileName) const;

	void drawFilesScreen();
	void drawFilesList();
	String shortenFileName(const String& fileName, uint8_t maxChars) const;
};

extern UI ui;

#endif // INTERFACE_H

