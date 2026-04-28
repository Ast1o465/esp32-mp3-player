#include "player_audio.h"

#include <Audio.h>
#include <SD.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "config.h"

AudioPlayer audioPlayer;
static Audio decoder;
static SemaphoreHandle_t commandMutex = nullptr;

enum class AudioCommand : uint8_t {
    None,
    Play,
    Stop,
    Pause,
    Resume
};

static AudioCommand pendingCommand = AudioCommand::None;
static String pendingFile;

static const uint8_t kTargetVolume = 16;
static uint8_t currentVolume = 0;
static bool volumeRampActive = false;
static unsigned long lastVolumeRampMs = 0;

static bool lockCommand(TickType_t timeoutTicks = pdMS_TO_TICKS(20)) {
    return commandMutex != nullptr && xSemaphoreTake(commandMutex, timeoutTicks) == pdTRUE;
}

static void unlockCommand() {
    if (commandMutex != nullptr) {
        xSemaphoreGive(commandMutex);
    }
}

void AudioPlayer::begin() {
    Serial.println("Init MP3 decoder...");
    commandMutex = xSemaphoreCreateMutex();
    if (commandMutex == nullptr) {
        Serial.println("Audio command mutex create failed");
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

    if (lockCommand()) {
        pendingFile = normalized;
        pendingCommand = AudioCommand::Play;
        unlockCommand();
    }

    playing = true;
    volumeRampActive = true;
    lastVolumeRampMs = millis();
}

void AudioPlayer::stop() {
    if (lockCommand()) {
        pendingCommand = AudioCommand::Stop;
        unlockCommand();
    }
    playing = false;
    volumeRampActive = false;
    currentVolume = 0;
    currentFile = "";
}

void AudioPlayer::pause() {
    if (playing) {
        if (lockCommand()) {
            pendingCommand = AudioCommand::Pause;
            unlockCommand();
        }
        playing = false;
        volumeRampActive = false;
    }
}

void AudioPlayer::resume() {
    if (!playing) {
        if (lockCommand()) {
            pendingCommand = AudioCommand::Resume;
            unlockCommand();
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

uint32_t AudioPlayer::getCurrentTime() const {
    return decoder.getAudioCurrentTime();
}

uint32_t AudioPlayer::getDuration() const {
    return decoder.getAudioFileDuration();
}

void audioUpdate() {
    AudioCommand cmd = AudioCommand::None;
    String cmdFile;

    if (lockCommand(pdMS_TO_TICKS(1))) {
        cmd = pendingCommand;
        if (cmd == AudioCommand::Play) {
            cmdFile = pendingFile;
        }
        pendingCommand = AudioCommand::None;
        unlockCommand();
    }

    switch (cmd) {
        case AudioCommand::Play: {
            decoder.stopSong();
            decoder.setVolume(0);
            currentVolume = 0;
            const bool started = decoder.connecttoFS(SD, cmdFile.c_str());
            volumeRampActive = started;

            Serial.print("Play request: ");
            Serial.print(cmdFile);
            Serial.print(" -> ");
            Serial.println(started ? "OK" : "FAILED");
            break;
        }
        case AudioCommand::Stop:
            decoder.stopSong();
            decoder.setVolume(0);
            volumeRampActive = false;
            currentVolume = 0;
            break;
        case AudioCommand::Pause:
            decoder.pauseResume();
            volumeRampActive = false;
            break;
        case AudioCommand::Resume:
            decoder.pauseResume();
            volumeRampActive = true;
            lastVolumeRampMs = millis();
            break;
        case AudioCommand::None:
            break;
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
}
