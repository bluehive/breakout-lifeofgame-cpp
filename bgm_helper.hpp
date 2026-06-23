#ifndef BGM_HELPER_HPP
#define BGM_HELPER_HPP

#include "raylib.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

// BGM 管理（raylib Music ストリームでループ再生）
struct GameBgm {
    Music music;
    bool ready;
};

// MIDI ノート番号 → 周波数 (Hz)
inline float midiToFreq(int midi) {
    return 440.0f * std::pow(2.0f, (midi - 69) / 12.0f);
}

// チップ調スクエア波（テトリス Type A / コロブチカ風）
inline float chipSquare(float phase, float duty) {
    return (phase < duty) ? 0.35f : -0.35f;
}

// 1音をレンダリング（短い ADSR エンベロープ付き）
inline void renderNote(std::vector<float>& buffer, int sampleRate,
                       int midi, float startSec, float durationSec, float volume) {
    if (midi < 0 || durationSec <= 0.0f) return;

    int startSample = static_cast<int>(startSec * sampleRate);
    int noteSamples = static_cast<int>(durationSec * sampleRate);
    if (startSample < 0) return;

    int needed = startSample + noteSamples;
    if (static_cast<int>(buffer.size()) < needed) {
        buffer.resize(needed, 0.0f);
    }

    float freq = midiToFreq(midi);
    float attack = std::min(0.01f, durationSec * 0.15f);
    float release = std::min(0.04f, durationSec * 0.25f);
    int attackSamples = static_cast<int>(attack * sampleRate);
    int releaseSamples = static_cast<int>(release * sampleRate);

    for (int i = 0; i < noteSamples; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float phase = std::fmod(freq * t, 1.0f);
        float sample = chipSquare(phase, 0.5f) * volume;

        float env = 1.0f;
        if (i < attackSamples && attackSamples > 0) {
            env = static_cast<float>(i) / attackSamples;
        } else if (i > noteSamples - releaseSamples && releaseSamples > 0) {
            env = static_cast<float>(noteSamples - i) / releaseSamples;
        }
        buffer[startSample + i] += sample * env;
    }
}

// テンポ早めの「ミュージックエアポート」風メロディ（コロブチカ / Tetris Type A 簡略版）
// BPM 180（オリジナルより速い）
inline void renderAirportStyleMelody(std::vector<float>& buffer, int sampleRate, int bpm) {
    const float beatSec = 60.0f / static_cast<float>(bpm);
    const float eighth = beatSec * 0.5f;
    const float quarter = beatSec;

    struct Note { int midi; float dur; };
    const Note melody[] = {
        // フレーズ1
        {76, eighth}, {71, eighth}, {72, eighth}, {74, eighth},
        {72, eighth}, {71, eighth}, {69, quarter},
        // フレーズ2
        {69, eighth}, {69, eighth}, {72, eighth}, {76, eighth},
        {74, eighth}, {72, eighth}, {71, quarter},
        // フレーズ3
        {71, eighth}, {72, eighth}, {74, eighth}, {76, eighth},
        {72, eighth}, {69, quarter},
        {-1, eighth}, // 休符
        // フレーズ4（後半）
        {74, eighth}, {77, eighth}, {81, eighth}, {79, eighth},
        {77, eighth}, {76, eighth}, {72, quarter},
        {72, eighth}, {76, eighth}, {81, eighth}, {79, eighth},
        {77, eighth}, {76, eighth}, {74, quarter},
        {71, eighth}, {74, eighth}, {77, eighth}, {76, eighth},
        {74, eighth}, {72, eighth}, {71, quarter},
        {71, eighth}, {72, eighth}, {74, eighth}, {76, eighth},
        {72, eighth}, {69, quarter},
    };

    float t = 0.0f;
    for (const auto& n : melody) {
        renderNote(buffer, sampleRate, n.midi, t, n.dur, 0.55f);
        t += n.dur;
    }

    // 低音伴奏（オクターブ下、シンプルなルート）
    const int bassRoots[] = {64, 64, 64, 69, 69, 69, 67, 67, 72, 72, 71, 71, 64};
    float bassStart = 0.0f;
    float bassDur = quarter * 2.0f;
    for (int root : bassRoots) {
        renderNote(buffer, sampleRate, root - 12, bassStart, bassDur, 0.22f);
        bassStart += bassDur;
        if (bassStart >= t) break;
    }
}

// float サンプル列を 16bit mono WAV にエンコード（LoadMusicStreamFromMemory 用）
inline std::vector<unsigned char> encodeWav16Mono(const std::vector<float>& samples, int sampleRate) {
    const int numSamples = static_cast<int>(samples.size());
    const int dataSize = numSamples * 2;
    const int fileSize = 44 + dataSize;

    std::vector<unsigned char> wav(fileSize, 0);
    auto write32 = [&](int offset, uint32_t v) {
        wav[offset] = static_cast<unsigned char>(v & 0xFF);
        wav[offset + 1] = static_cast<unsigned char>((v >> 8) & 0xFF);
        wav[offset + 2] = static_cast<unsigned char>((v >> 16) & 0xFF);
        wav[offset + 3] = static_cast<unsigned char>((v >> 24) & 0xFF);
    };
    auto write16 = [&](int offset, uint16_t v) {
        wav[offset] = static_cast<unsigned char>(v & 0xFF);
        wav[offset + 1] = static_cast<unsigned char>((v >> 8) & 0xFF);
    };

    std::memcpy(&wav[0], "RIFF", 4);
    write32(4, static_cast<uint32_t>(fileSize - 8));
    std::memcpy(&wav[8], "WAVE", 4);
    std::memcpy(&wav[12], "fmt ", 4);
    write32(16, 16);
    write16(20, 1); // PCM
    write16(22, 1); // mono
    write32(24, static_cast<uint32_t>(sampleRate));
    write32(28, static_cast<uint32_t>(sampleRate * 2));
    write16(32, 2);
    write16(34, 16);
    std::memcpy(&wav[36], "data", 4);
    write32(40, static_cast<uint32_t>(dataSize));

    for (int i = 0; i < numSamples; ++i) {
        float s = samples[i];
        if (s > 1.0f) s = 1.0f;
        if (s < -1.0f) s = -1.0f;
        int16_t pcm = static_cast<int16_t>(s * 32767.0f);
        int off = 44 + i * 2;
        wav[off] = static_cast<unsigned char>(pcm & 0xFF);
        wav[off + 1] = static_cast<unsigned char>((pcm >> 8) & 0xFF);
    }
    return wav;
}

// BGM を合成して Music ストリームとして初期化
inline GameBgm initGameBgm() {
    GameBgm bgm = {};
    bgm.ready = false;
    if (!IsAudioDeviceReady()) return bgm;

    const int sampleRate = 44100;
    const int bpm = 180; // テンポ早め

    std::vector<float> samples;
    renderAirportStyleMelody(samples, sampleRate, bpm);

    // 末尾に短いフェードアウト
    int fadeSamples = sampleRate / 20;
    int total = static_cast<int>(samples.size());
    for (int i = 0; i < fadeSamples && i < total; ++i) {
        float gain = static_cast<float>(fadeSamples - i) / fadeSamples;
        samples[total - 1 - i] *= gain;
    }

    std::vector<unsigned char> wav = encodeWav16Mono(samples, sampleRate);
    Music music = LoadMusicStreamFromMemory(".wav", wav.data(), static_cast<int>(wav.size()));
    if (!IsMusicValid(music)) return bgm;

    music.looping = true;
    SetMusicVolume(music, 0.30f);
    bgm.music = music;
    bgm.ready = true;
    return bgm;
}

inline void playBgmIfReady(const GameBgm& bgm) {
    if (bgm.ready && !IsMusicStreamPlaying(bgm.music)) {
        PlayMusicStream(bgm.music);
    }
}

inline void updateGameBgm(const GameBgm& bgm) {
    if (bgm.ready) UpdateMusicStream(bgm.music);
}

inline void pauseBgmIfReady(const GameBgm& bgm) {
    if (bgm.ready && IsMusicStreamPlaying(bgm.music)) {
        PauseMusicStream(bgm.music);
    }
}

inline void resumeBgmIfReady(const GameBgm& bgm) {
    if (bgm.ready && !IsMusicStreamPlaying(bgm.music)) {
        ResumeMusicStream(bgm.music);
    }
}

inline void unloadGameBgm(GameBgm& bgm) {
    if (!bgm.ready) return;
    StopMusicStream(bgm.music);
    UnloadMusicStream(bgm.music);
    bgm.ready = false;
}

#endif // BGM_HELPER_HPP