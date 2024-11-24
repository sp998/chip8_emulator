#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <SDL2/SDL.h>

class Audio {
public:
    Audio();           // Constructor to initialize audio
    ~Audio();          // Destructor to clean up audio
    void play_tit_sound(); // Function to play the simple "tit" sound
    bool is_initialized() const { return initialized; } // Check if audio is initialized

private:
    SDL_AudioSpec desiredSpec;
    static void audio_callback(void* userdata, Uint8* stream, int len); // Static callback function to generate audio
    double phase; // Phase of the sine wave
    bool initialized; // Flag to check if initialization was successful
};

#endif // AUDIO_HPP
