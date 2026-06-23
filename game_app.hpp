#ifndef GAME_APP_HPP
#define GAME_APP_HPP

#include "game_logic.hpp"
#include "audio_helper.hpp"
#include "raylib.h"

inline void drawGame(const GameState& state) {
    ClearBackground(WHITE);

    for (const auto& cell : state.cells) {
        int x = cell.first;
        int y = cell.second;
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);
        }
    }

    DrawRectangle(static_cast<int>(state.paddle.x), PADDLE_Y,
                  PADDLE_WIDTH, PADDLE_HEIGHT, ORANGE);

    DrawRectangle(static_cast<int>(state.ball.x), static_cast<int>(state.ball.y),
                  BALL_SIZE, BALL_SIZE, RED);

    DrawText(TextFormat("SCORE: %d", state.score), 15, 15, 20, BLACK);
    DrawText(TextFormat("LIVES: %d", state.lives), 150, 15, 20, BLACK);

    if (state.gameOver) {
        const char* msg = "GAME OVER - Press SPACE to Restart";
        int tw = MeasureText(msg, 20);
        DrawText(msg, SCREEN_WIDTH / 2 - tw / 2, SCREEN_HEIGHT / 2 - 10, 20, RED);
    }
}

inline void handleInput(GameState& state) {
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

// Returns which sound event fired this tick (for testing and audio routing).
enum class TickSound { None, Bounce, LifeLost, PaddleHit, CellDestroy };

inline TickSound gameTick(GameState& state, const GameSounds& sounds) {
    if (state.gameOver) return TickSound::None;

    updatePaddle(state);
    followBallToPaddle(state);

    TickSound fired = TickSound::None;

    BallEvent ev = updateBallPhysics(state);
    if (ev == BallEvent::WallBounce) {
        playSoundIfReady(sounds, sounds.bounce);
        fired = TickSound::Bounce;
    } else if (ev == BallEvent::BottomMiss) {
        playSoundIfReady(sounds, sounds.lifeLost);
        fired = TickSound::LifeLost;
    }

    if (checkPaddleCollision(state)) {
        playSoundIfReady(sounds, sounds.paddleHit);
        fired = TickSound::PaddleHit;
    }
    if (checkCellCollisions(state)) {
        playSoundIfReady(sounds, sounds.cellDestroy);
        fired = TickSound::CellDestroy;
    }

    tickLifeGeneration(state);
    checkStageClear(state);
    return fired;
}

#endif // GAME_APP_HPP