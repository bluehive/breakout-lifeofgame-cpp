// gameTick と音声の統合テスト（本番コードを直接呼び出す）
// Integration tests for shipped gameTick() and audio helpers
#include "game_app.hpp"
#include <cassert>
#include <cstdio>

static int testsRun = 0;
static int testsFailed = 0;

#define TEST(name) \
    do { ++testsRun; printf("  %s ... ", name); } while (0)
#define PASS() printf("PASS\n")
#define FAIL(msg) do { printf("FAIL: %s\n", msg); ++testsFailed; } while (0)

// 本番 gameTick() を短いセッションで駆動する
static void test_game_tick_session() {
    TEST("gameTick mini-session (shoot, evolve, score)");
    GameState state;
    initGame(state);
    GameSounds silent = {};
    silent.ready = false;

    size_t cellsAtStart = state.cells.size();
    if (cellsAtStart != 20) { FAIL("init cells"); return; }

    shootBall(state, true);
    if (!state.ball.active) { FAIL("ball not active after shoot"); return; }

    float ballXStart = state.ball.x;
    int prevCounter = state.frameCounter;
    bool sawLifeTick = false;

    for (int i = 0; i < 30; ++i) {
        gameTick(state, silent);
        if (prevCounter == 19 && state.frameCounter == 0) sawLifeTick = true;
        prevCounter = state.frameCounter;
    }

    if (!sawLifeTick) { FAIL("life generation did not fire by frame 20"); return; }
    if (state.ball.x == ballXStart && state.ball.y == PADDLE_Y - BALL_SIZE - 2.0f) {
        FAIL("ball should have moved after shoot"); return;
    }
    PASS();
}

static void test_game_tick_cell_destroy_sound_event() {
    TEST("gameTick fires CellDestroy on block hit (+ stage clear respawn)");
    GameState state;
    initGame(state);
    state.cells = {{1, 1}};
    state.ball.active = true;
    state.ball.x = 1 * CELL_SIZE - 4;
    state.ball.y = 1 * CELL_SIZE + 10;
    state.ball.vx = 5;
    state.ball.vy = 1;

    GameSounds silent = {};
    TickSound snd = gameTick(state, silent);
    if (snd != TickSound::CellDestroy) { FAIL("expected CellDestroy event"); return; }
    if (state.score != 10) { FAIL("score not incremented"); return; }
    // Last cell destroyed triggers checkStageClear -> initCells() respawn (reference behavior)
    if (state.cells.size() != 20) { FAIL("stage clear should respawn 20 glider cells"); return; }
    if (state.ball.active) { FAIL("resetBall after stage clear"); return; }
    PASS();
}

static void test_game_tick_paddle_hit_sound_event() {
    TEST("gameTick fires PaddleHit on paddle collision");
    GameState state;
    initGame(state);
    state.paddle.x = 400;
    state.ball.active = true;
    state.ball.x = state.paddle.x + PADDLE_WIDTH / 2.0f - BALL_SIZE / 2.0f;
    state.ball.y = PADDLE_Y - BALL_SIZE + 1;
    state.ball.vx = 2;
    state.ball.vy = 5;

    GameSounds silent = {};
    TickSound snd = gameTick(state, silent);
    if (snd != TickSound::PaddleHit) { FAIL("expected PaddleHit event"); return; }
    if (state.ball.vy >= 0) { FAIL("ball should reflect upward"); return; }
    PASS();
}

static void test_play_sound_if_ready_gating() {
    TEST("playSoundIfReady gates on sounds.ready");
    GameSounds off = {};
    off.ready = false;
    Sound dummy = {};
    // Must not crash when ready=false (PlaySound must not be called)
    playSoundIfReady(off, dummy);
    PASS();
}

static void test_audio_init_and_play() {
    TEST("initGameSounds + playSoundIfReady with real audio device");
    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        CloseAudioDevice();
        printf("SKIP (no audio device)\n");
        return;
    }

    GameSounds sounds = initGameSounds();
    if (!sounds.ready) {
        CloseAudioDevice();
        FAIL("sounds.ready should be true"); return;
    }

    playSoundIfReady(sounds, sounds.bounce);
    // Give miniaudio a moment to start playback
    for (int i = 0; i < 5; ++i) {
        if (IsSoundPlaying(sounds.bounce)) break;
    }
    bool playing = IsSoundPlaying(sounds.bounce);
    unloadGameSounds(sounds);
    CloseAudioDevice();

    if (!playing) { FAIL("bounce sound did not start playing"); return; }
    PASS();
}

int main() {
    printf("Running integration tests (shipped gameTick + audio)...\n");
    test_game_tick_session();
    test_game_tick_cell_destroy_sound_event();
    test_game_tick_paddle_hit_sound_event();
    test_play_sound_if_ready_gating();
    test_audio_init_and_play();

    printf("\n%d tests run, %d failed\n", testsRun, testsFailed);
    return testsFailed == 0 ? 0 : 1;
}