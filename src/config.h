#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#ifndef CONFIG_H
#define CONFIG_H

// Serial
#define SERIAL_BAUD 115200

// Display (Adafruit ST7735)
#define TFT_CS   5
#define TFT_RST  4
#define TFT_DC   2
#define LED_PIN  15

// SD card
#define SD_CS    16

// Encoder
const int ENCODER_PIN_A = 27;
const int ENCODER_PIN_B = 26;
const int BUTTON_PIN_SW = 25;

// Display rotation 0..3
#define DISPLAY_ROTATION 0

// ST7735 tab type for initR()
#define DISPLAY_INITR_TAB INITR_BLACKTAB

#endif // CONFIG_H
