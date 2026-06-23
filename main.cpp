#include "game_app.hpp"
#include "bgm_helper.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

// ウィンドウなしの自己診断（CI・動作確認用）
static void runSelfTest() {
    GameState state;
    initGame(state);

    printf("SELFTEST init: cells=%zu lives=%d score=%d ball_active=%d\n",
           state.cells.size(), state.lives, state.score, state.ball.active ? 1 : 0);

    shootBall(state, true);
    printf("SELFTEST shoot: vx=%.1f vy=%.1f active=%d\n",
           state.ball.vx, state.ball.vy, state.ball.active ? 1 : 0);

    GameSounds silent = {};
    silent.ready = false;

    for (int i = 0; i < 25; ++i) {
        TickSound snd = gameTick(state, silent);
        if (snd != TickSound::None) {
            printf("SELFTEST tick %d: sound_event=%d score=%d lives=%d\n",
                   i, static_cast<int>(snd), state.score, state.lives);
        }
    }
    printf("SELFTEST after 25 ticks: frame_counter=%d cells=%zu score=%d\n",
           state.frameCounter, state.cells.size(), state.score);
    printf("SELFTEST OK\n");
}

int main(int argc, char* argv[]) {
    // ヘッドレス自己テストモード
    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == '-') {
        if (strcmp(argv[1], "--self-test") == 0) {
            runSelfTest();
            return 0;
        }
    }

    // raylib ウィンドウ・音声の初期化
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout Game of Life");
    SetTargetFPS(60);

    InitAudioDevice();
    GameSounds sounds = initGameSounds();
    GameBgm bgm = initGameBgm();
    playBgmIfReady(bgm);

    GameState state;
    initGame(state);
    bool wasGameOver = false;

    // メインループ（60 FPS）
    while (!WindowShouldClose()) {
        handleInput(state);
        gameTick(state, sounds);

        // ゲームオーバー時は BGM を一時停止、リスタートで再開
        if (state.gameOver && !wasGameOver) {
            pauseBgmIfReady(bgm);
        } else if (!state.gameOver && wasGameOver) {
            resumeBgmIfReady(bgm);
        }
        wasGameOver = state.gameOver;
        updateGameBgm(bgm);

        BeginDrawing();
        drawGame(state);
        EndDrawing();
    }

    unloadGameBgm(bgm);
    unloadGameSounds(sounds);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}