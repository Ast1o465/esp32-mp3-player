#ifndef PLAYER_AUDIO_H
#define PLAYER_AUDIO_H

#include <Arduino.h>

class AudioPlayer {
public:
    void begin();
    void playFile(const String& filename);
    void stop();
    void pause();
    void resume();
    bool isPlaying() const;
    const String& getCurrentFile() const;
    uint32_t getCurrentTime() const;
    uint32_t getDuration() const;
    
private:
    String currentFile;
    bool playing = false;
};

extern AudioPlayer audioPlayer;
void audioUpdate();

#endif // PLAYER_AUDIO_H
