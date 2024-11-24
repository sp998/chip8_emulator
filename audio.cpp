#include "audio.hpp"
#include <cmath>
#include <iostream>

// Audio parameters
const int SAMPLE_RATE = 44100;  // 44.1 kHz sample rate
const int AMPLITUDE = 30000;    // Amplitude (volume)
const int CHANNELS = 1;         // Mono sound
const int SAMPLE_SIZE = 2;      // 16-bit samples

const double PI = 3.14159265358979323846;
const double FREQUENCY = 550.0; // Frequency in Hz (A5 note, higher pitch)

// Sound duration
const int SOUND_DURATION_MS = 20; // Duration of the "tit" sound in milliseconds

Audio::Audio() : phase(0.0), initialized(false) {
    // Initialize SDL Audio subsystem
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }

    // Set up the audio spec
    desiredSpec.freq = SAMPLE_RATE;
    desiredSpec.format = AUDIO_S16SYS;  // 16-bit signed samples
    desiredSpec.channels = CHANNELS;
    desiredSpec.samples = 1024;  // Size of the audio buffer
    desiredSpec.callback = audio_callback;  // Set the audio callback function
    desiredSpec.userdata = this;  // Pass the current instance to the callback

    // Open the audio device
    if (SDL_OpenAudio(&desiredSpec, nullptr) < 0) {
        std::cerr << "SDL_OpenAudio failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    initialized = true; // Mark as successfully initialized
}

Audio::~Audio() {
    if (initialized) {
        SDL_CloseAudio(); // Clean up the audio device
        SDL_Quit();        // Quit SDL
    }
}

void Audio::play_tit_sound() {
    if (initialized) {
        SDL_PauseAudio(0);  // Start playing audio immediately

        // Play the sound for the duration defined in SOUND_DURATION_MS
        SDL_Delay(SOUND_DURATION_MS); // Wait for the sound to finish
        SDL_PauseAudio(1);  // Pause the audio after the sound finishes
    } else {
        std::cerr << "Audio not initialized successfully!" << std::endl;
    }
}

void Audio::audio_callback(void* userdata, Uint8* stream, int len) {
    Audio* audio = static_cast<Audio*>(userdata);
    Sint16* buffer = (Sint16*)stream;

    int num_samples = len / SAMPLE_SIZE;  // Number of samples in the buffer
    double increment = FREQUENCY * 2.0 * PI / SAMPLE_RATE;

    for (int i = 0; i < num_samples; i++) {
        // Generate the sine wave sample (tit sound)
        buffer[i] = (Sint16)(AMPLITUDE * sin(audio->phase));
        audio->phase += increment;

        // Keep the phase in the range of 0 to 2*PI
        if (audio->phase >= 2.0 * PI) {
            audio->phase -= 2.0 * PI;
        }
    }
}
