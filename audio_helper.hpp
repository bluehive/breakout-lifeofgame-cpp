#ifndef AUDIO_HELPER_HPP
#define AUDIO_HELPER_HPP

#include "raylib.h"
#include <cmath>
#include <cstdlib>
#include <vector>

// ゲーム内で使用する効果音セット
struct GameSounds {
    Sound bounce;       // 壁バウンド
    Sound paddleHit;    // パドルヒット
    Sound cellDestroy;  // セル破壊
    Sound lifeLost;     // ライフ減少
    bool ready;         // 音声デバイス初期化済みか
};

// 正弦波＋減衰エンベロープで短いトーンを合成
inline Sound makeToneSound(int sampleRate, int frequency, float durationSec, float volume = 0.3f) {
    int sampleCount = static_cast<int>(sampleRate * durationSec);
    if (sampleCount <= 0) {
        return Sound{};
    }

    // LoadSoundFromWave まで有効なヒープバッファを確保
    float* data = static_cast<float*>(MemAlloc(static_cast<unsigned int>(sampleCount) * sizeof(float)));
    if (!data) return Sound{};

    for (int i = 0; i < sampleCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float envelope = 1.0f - static_cast<float>(i) / sampleCount;
        data[i] = volume * envelope * std::sin(2.0f * 3.14159265f * frequency * t);
    }

    Wave wave = {};
    wave.frameCount = sampleCount;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 32;
    wave.channels = 1;
    wave.data = data;

    Sound sound = LoadSoundFromWave(wave);
    MemFree(data);
    return sound;
}

// 各イベント用の効果音を生成
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

// 音声が利用可能なときだけ再生
inline void playSoundIfReady(const GameSounds& sounds, Sound sound) {
    if (sounds.ready) PlaySound(sound);
}

#endif // AUDIO_HELPER_HPP