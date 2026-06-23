#ifndef AUDIO_HELPER_HPP
#define AUDIO_HELPER_HPP

#include "raylib.h"
#include <cmath>
#include <vector>

struct GameSounds {
    Sound bounce;
    Sound paddleHit;
    Sound cellDestroy;
    Sound lifeLost;
    bool ready;
};

inline Sound makeToneSound(int sampleRate, int frequency, float durationSec, float volume = 0.3f) {
    int sampleCount = static_cast<int>(sampleRate * durationSec);
    std::vector<float> samples(sampleCount);
    for (int i = 0; i < sampleCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float envelope = 1.0f - static_cast<float>(i) / sampleCount;
        samples[i] = volume * envelope * std::sin(2.0f * 3.14159265f * frequency * t);
    }

    Wave wave = {};
    wave.frameCount = sampleCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 32;
    wave.channels = 1;
    wave.data = samples.data();

    Sound sound = LoadSoundFromWave(wave);
    return sound;
}

inline GameSounds initGameSounds() {
    GameSounds sounds = {};
    sounds.ready = false;
    if (!IsAudioDeviceReady()) return sounds;

    int rate = 44100;
    sounds.bounce = makeToneSound(rate, 440, 0.08f, 0.25f);
    sounds.paddleHit = makeToneSound(rate, 660, 0.10f, 0.30f);
    sounds.cellDestroy = makeToneSound(rate, 880, 0.12f, 0.35f);
    sounds.lifeLost = makeToneSound(rate, 220, 0.25f, 0.40f);
    sounds.ready = true;
    return sounds;
}

inline void unloadGameSounds(GameSounds& sounds) {
    if (!sounds.ready) return;
    UnloadSound(sounds.bounce);
    UnloadSound(sounds.paddleHit);
    UnloadSound(sounds.cellDestroy);
    UnloadSound(sounds.lifeLost);
    sounds.ready = false;
}

inline void playSoundIfReady(const GameSounds& sounds, Sound sound) {
    if (sounds.ready) PlaySound(sound);
}

#endif // AUDIO_HELPER_HPP