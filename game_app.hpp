#ifndef GAME_APP_HPP
#define GAME_APP_HPP

#include "game_logic.hpp"
#include "audio_helper.hpp"
#include "raylib.h"

// ゲーム画面の描画
inline void drawGame(const GameState& state) {
    ClearBackground(WHITE);

    // 1. 生存セル（ブロック）の描画
    for (const auto& cell : state.cells) {
        int x = cell.first;
        int y = cell.second;
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);
        }
    }

    // 2. パドル
    DrawRectangle(static_cast<int>(state.paddle.x), PADDLE_Y,
                  PADDLE_WIDTH, PADDLE_HEIGHT, ORANGE);

    // 3. ボール
    DrawRectangle(static_cast<int>(state.ball.x), static_cast<int>(state.ball.y),
                  BALL_SIZE, BALL_SIZE, RED);

    // 4. スコア・ライフ表示
    DrawText(TextFormat("SCORE: %d", state.score), 15, 15, 20, BLACK);
    DrawText(TextFormat("LIVES: %d", state.lives), 150, 15, 20, BLACK);

    // ゲームオーバー表示
    if (state.gameOver) {
        const char* msg = "GAME OVER - Press SPACE to Restart";
        int tw = MeasureText(msg, 20);
        DrawText(msg, SCREEN_WIDTH / 2 - tw / 2, SCREEN_HEIGHT / 2 - 10, 20, RED);
    }
}

// キーボード入力の処理
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
            initGame(state);  // リスタート
        } else {
            bool randomRight = GetRandomValue(0, 1) == 1;
            shootBall(state, randomRight);
        }
    }
}

// この tick で鳴った効果音の種類（テスト・音声ルーティング用）
enum class TickSound { None, Bounce, LifeLost, PaddleHit, CellDestroy };

// 1フレーム分のゲーム状態更新（参照版 game-tick と同じ順序）
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