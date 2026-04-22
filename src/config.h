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

// 5-way joystick (digital outputs, active LOW)
// Custom mapping: R = up, F = left, B = right, L = down, M = select
#define JOY_PIN_F 27
#define JOY_PIN_B 26
#define JOY_PIN_L 25
#define JOY_PIN_R 33
#define JOY_PIN_M 32

// Input timing
#define JOY_DEBOUNCE_MS 140
#define JOY_SELECT_DEBOUNCE_MS 220

// Audio I2S (PCM5102A)
#define I2S_BCLK_PIN 22
#define I2S_LRCK_PIN 21
#define I2S_DOUT_PIN 17

// Display rotation 0..3
#define DISPLAY_ROTATION 0

// ST7735 tab type for initR()
#define DISPLAY_INITR_TAB INITR_BLACKTAB

#endif // CONFIG_H
