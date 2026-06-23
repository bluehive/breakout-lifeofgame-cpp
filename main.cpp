#include "game_logic.hpp"
#include "audio_helper.hpp"
#include "raylib.h"
#include <cstdlib>
#include <string>

static void drawGame(const GameState& state) {
    ClearBackground(WHITE);

    // 1. Draw living cells (blocks)
    for (const auto& cell : state.cells) {
        int x = cell.first;
        int y = cell.second;
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);
        }
    }

    // 2. Draw paddle
    DrawRectangle(static_cast<int>(state.paddle.x), PADDLE_Y,
                  PADDLE_WIDTH, PADDLE_HEIGHT, ORANGE);

    // 3. Draw ball
    DrawRectangle(static_cast<int>(state.ball.x), static_cast<int>(state.ball.y),
                  BALL_SIZE, BALL_SIZE, RED);

    // 4. Score and lives
    DrawText(TextFormat("SCORE: %d", state.score), 15, 15, 20, BLACK);
    DrawText(TextFormat("LIVES: %d", state.lives), 150, 15, 20, BLACK);

    if (state.gameOver) {
        const char* msg = "GAME OVER - Press SPACE to Restart";
        int tw = MeasureText(msg, 20);
        DrawText(msg, SCREEN_WIDTH / 2 - tw / 2, SCREEN_HEIGHT / 2 - 10, 20, RED);
    }
}

static void handleInput(GameState& state) {
    if (IsKeyDown(KEY_LEFT)) {
        state.paddle.vx = -PADDLE_SPEED;
    } else if (IsKeyDown(KEY_RIGHT)) {
        state.paddle.vx = PADDLE_SPEED;
    } else {
        state.paddle.vx = 0;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        if (state.gameOver) {
            initGame(state);
        } else {
            bool randomRight = GetRandomValue(0, 1) == 1;
            shootBall(state, randomRight);
        }
    }
}

static void gameTick(GameState& state, const GameSounds& sounds) {
    if (state.gameOver) return;

    updatePaddle(state);
    followBallToPaddle(state);

    BallEvent ev = updateBallPhysics(state);
    if (ev == BallEvent::WallBounce) {
        playSoundIfReady(sounds, sounds.bounce);
    } else if (ev == BallEvent::BottomMiss) {
        playSoundIfReady(sounds, sounds.lifeLost);
    }

    if (checkPaddleCollision(state)) {
        playSoundIfReady(sounds, sounds.paddleHit);
    }
    if (checkCellCollisions(state)) {
        playSoundIfReady(sounds, sounds.cellDestroy);
    }

    tickLifeGeneration(state);
    checkStageClear(state);
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout Game of Life");
    SetTargetFPS(60);

    InitAudioDevice();
    GameSounds sounds = initGameSounds();

    GameState state;
    initGame(state);

    while (!WindowShouldClose()) {
        handleInput(state);
        gameTick(state, sounds);

        BeginDrawing();
        drawGame(state);
        EndDrawing();
    }

    unloadGameSounds(sounds);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}