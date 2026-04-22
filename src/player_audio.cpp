#include "player_audio.h"

#include <Audio.h>
#include <SD.h>

#include "config.h"

AudioPlayer audioPlayer;
static Audio decoder;

void AudioPlayer::begin() {
    Serial.println("Init MP3 decoder...");
    decoder.setPinout(I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN);
    decoder.setVolume(16);
    Serial.println("MP3 decoder ready");
}

void AudioPlayer::playFile(const String& filename) {
    String normalized = filename;
    if (!normalized.startsWith("/")) {
        normalized = "/" + normalized;
    }

    currentFile = normalized;
    const bool started = decoder.connecttoFS(SD, normalized.c_str());
    playing = started;

    Serial.print("Play request: ");
    Serial.print(normalized);
    Serial.print(" -> ");
    Serial.println(started ? "OK" : "FAILED");
}

void AudioPlayer::stop() {
    decoder.stopSong();
    playing = false;
    currentFile = "";
}

void AudioPlayer::pause() {
    if (playing) {
        decoder.pauseResume();
        playing = false;
    }
}

void AudioPlayer::resume() {
    if (!playing) {
        decoder.pauseResume();
        playing = true;
    }
}

bool AudioPlayer::isPlaying() const {
    return playing;
}

const String& AudioPlayer::getCurrentFile() const {
    return currentFile;
}

void audioUpdate() {
    decoder.loop();
}
