#include "player_audio.h"

#include <Audio.h>
#include <SD.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "config.h"

AudioPlayer audioPlayer;
static Audio decoder;
static SemaphoreHandle_t decoderMutex = nullptr;

static const uint8_t kTargetVolume = 16;
static uint8_t currentVolume = 0;
static bool volumeRampActive = false;
static unsigned long lastVolumeRampMs = 0;

static bool lockDecoder(TickType_t timeoutTicks = pdMS_TO_TICKS(20)) {
    return decoderMutex != nullptr && xSemaphoreTake(decoderMutex, timeoutTicks) == pdTRUE;
}

static void unlockDecoder() {
    if (decoderMutex != nullptr) {
        xSemaphoreGive(decoderMutex);
    }
}

void AudioPlayer::begin() {
    Serial.println("Init MP3 decoder...");
    decoderMutex = xSemaphoreCreateMutex();
    if (decoderMutex == nullptr) {
        Serial.println("Decoder mutex create failed");
    }

    decoder.setPinout(I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN);
    decoder.setVolume(0);
    currentVolume = 0;
    volumeRampActive = false;
    Serial.println("MP3 decoder ready");
}

void AudioPlayer::playFile(const String& filename) {
    String normalized = filename;
    if (!normalized.startsWith("/")) {
        normalized = "/" + normalized;
    }

    currentFile = normalized;

    bool started = false;
    if (lockDecoder()) {
        // Stop current stream before opening a new one to avoid buffer artifacts.
        decoder.stopSong();
        decoder.setVolume(0);
        currentVolume = 0;
        started = decoder.connecttoFS(SD, normalized.c_str());
        unlockDecoder();
    }

    playing = started;
    volumeRampActive = started;
    lastVolumeRampMs = millis();

    Serial.print("Play request: ");
    Serial.print(normalized);
    Serial.print(" -> ");
    Serial.println(started ? "OK" : "FAILED");
}

void AudioPlayer::stop() {
    if (lockDecoder()) {
        decoder.stopSong();
        decoder.setVolume(0);
        unlockDecoder();
    }
    playing = false;
    volumeRampActive = false;
    currentVolume = 0;
    currentFile = "";
}

void AudioPlayer::pause() {
    if (playing) {
        if (lockDecoder()) {
            decoder.pauseResume();
            unlockDecoder();
        }
        playing = false;
        volumeRampActive = false;
    }
}

void AudioPlayer::resume() {
    if (!playing) {
        if (lockDecoder()) {
            decoder.pauseResume();
            unlockDecoder();
        }
        playing = true;
        volumeRampActive = true;
        lastVolumeRampMs = millis();
    }
}

bool AudioPlayer::isPlaying() const {
    return playing;
}

const String& AudioPlayer::getCurrentFile() const {
    return currentFile;
}

void audioUpdate() {
    if (!lockDecoder(pdMS_TO_TICKS(2))) {
        return;
    }

    decoder.loop();

    if (volumeRampActive && audioPlayer.isPlaying()) {
        const unsigned long now = millis();
        if (now - lastVolumeRampMs >= 18) {
            lastVolumeRampMs = now;
            if (currentVolume < kTargetVolume) {
                currentVolume++;
                decoder.setVolume(currentVolume);
            } else {
                volumeRampActive = false;
            }
        }
    }

    unlockDecoder();
}
