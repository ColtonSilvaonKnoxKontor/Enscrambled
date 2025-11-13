// Happy birthday audio integration for Enscrambled
#include "happy_birthday.hpp"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

namespace {
// Define the notes (frequencies in Hz)
constexpr int C4 = 262;
constexpr int D4 = 294;
constexpr int E4 = 330;
constexpr int F4 = 349;
constexpr int G4 = 392;
constexpr int A4 = 440;
constexpr int AS4 = 466; // A-Sharp / B-Flat
constexpr int C5 = 523;  // High C

// Durations (milliseconds)
constexpr int EIGHTH = 250;
constexpr int QUARTER = 500;
constexpr int HALF = 1000;

constexpr int AMPLITUDE = 10000;   // Volume
constexpr int SAMPLE_RATE = 44100; // CD-quality audio

bool g_verbose = false;
SDL_AudioDeviceID g_audio_device = 0;

bool initializeAudioSubsystem(bool verbose) {
    auto logInfo = [&](const std::string& msg) {
        if (verbose) {
            std::cout << "[birthday] " << msg << std::endl;
        }
    };
    auto logError = [&](const std::string& msg) {
        if (verbose) {
            std::cerr << "[birthday][error] " << msg << std::endl;
        }
    };

    if (SDL_Init(SDL_INIT_AUDIO) == 0) {
        logInfo("SDL audio initialized (default driver: " +
                std::string(SDL_GetCurrentAudioDriver() ? SDL_GetCurrentAudioDriver() : "unknown") + ")");
        return true;
    }

    logError(std::string("Default audio driver failed: ") + SDL_GetError());
    SDL_Quit();

    logInfo("Attempting to find a working audio driver...");
    for (int i = 0; i < SDL_GetNumAudioDrivers(); ++i) {
        const char* driver_name = SDL_GetAudioDriver(i);
        logInfo("Trying driver: " + std::string(driver_name));
        if (SDL_AudioInit(driver_name) == 0) {
            logInfo("Successfully initialized audio driver: " + std::string(driver_name));
            return true;
        }
        logError("Could not initialize audio driver: " + std::string(driver_name) + " - " + SDL_GetError());
        SDL_AudioQuit();
    }

    logError("No working audio driver found. Sound will be disabled.");
    // As a last resort, try the dummy driver (no actual audio output)
    if (SDL_AudioInit("dummy") == 0) {
        logInfo("SDL audio initialized with dummy driver (no sound output).");
        return true;
    }

    logError(std::string("Failed to initialize even the dummy audio driver: ") + SDL_GetError());
    return false;
}

void play_note(int frequency, int duration_ms) {
    if (g_verbose) {
        std::cout << "  Playing: " << frequency << " Hz for " << duration_ms << " ms" << std::endl;
    }

    const int total_samples = (SAMPLE_RATE * duration_ms) / 1000;
    std::vector<Sint16> buffer(static_cast<size_t>(total_samples));
    const double phase_increment = 2.0 * M_PI * frequency / SAMPLE_RATE;
    double current_phase = 0.0;

    for (int i = 0; i < total_samples; ++i) {
        buffer[static_cast<size_t>(i)] = static_cast<Sint16>(AMPLITUDE * sin(current_phase));
        current_phase += phase_increment;
    }

    if (SDL_QueueAudio(g_audio_device, buffer.data(), static_cast<Uint32>(buffer.size() * sizeof(Sint16))) != 0) {
        std::cerr << "Error: Could not queue audio: " << SDL_GetError() << std::endl;
    }

    SDL_Delay(duration_ms);
}

void play_song_loop() {
    while (true) {
        if (g_verbose) {
            std::cout << "--- Starting full song ---" << std::endl;
        }

        play_note(C4, EIGHTH);
        play_note(C4, EIGHTH);
        play_note(D4, QUARTER);
        play_note(C4, QUARTER);
        play_note(F4, QUARTER);
        play_note(E4, HALF);

        SDL_Delay(EIGHTH);

        play_note(C4, EIGHTH);
        play_note(C4, EIGHTH);
        play_note(D4, QUARTER);
        play_note(C4, QUARTER);
        play_note(G4, QUARTER);
        play_note(F4, HALF);

        SDL_Delay(EIGHTH);

        play_note(C4, EIGHTH);
        play_note(C4, EIGHTH);
        play_note(C5, QUARTER);
        play_note(A4, QUARTER);
        play_note(F4, QUARTER);
        play_note(E4, QUARTER);
        play_note(D4, HALF);

        SDL_Delay(EIGHTH);

        play_note(AS4, EIGHTH);
        play_note(AS4, EIGHTH);
        play_note(A4, QUARTER);
        play_note(F4, QUARTER);
        play_note(G4, QUARTER);
        play_note(F4, HALF);

        if (g_verbose) {
            std::cout << "--- Song finished, restarting in 1 second... ---" << std::endl;
        }

        SDL_Delay(1000);
    }
}

} // namespace

void runHappyBirthdaySong(bool verbose) {
    g_verbose = verbose;

    if (!initializeAudioSubsystem(g_verbose)) {
        std::cerr << "Error: Could not initialize any SDL audio driver." << std::endl;
        return;
    }

    SDL_AudioSpec desired_spec;
    SDL_zero(desired_spec);
    desired_spec.freq = SAMPLE_RATE;
    desired_spec.format = AUDIO_S16SYS;
    desired_spec.channels = 1;
    desired_spec.samples = 4096;
    desired_spec.callback = nullptr;

    g_audio_device = SDL_OpenAudioDevice(nullptr, 0, &desired_spec, nullptr, 0);

    if (g_audio_device == 0) {
        std::cerr << "Error: Could not open audio device: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    if (g_verbose) {
        std::cout << "SDL Audio Initialized." << std::endl;
        std::cout << "Playing the *full* song in an infinite loop..." << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;
    }

    SDL_PauseAudioDevice(g_audio_device, 0);

    play_song_loop();

    SDL_CloseAudioDevice(g_audio_device);
    SDL_Quit();
}

