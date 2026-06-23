#ifndef GAME_LOGIC_HPP
#define GAME_LOGIC_HPP

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <set>
#include <utility>
#include <vector>

// -------------------------------------------------------------
// Constants (matching breakout-lifeofgame.rkt)
// -------------------------------------------------------------
constexpr int WIDTH = 40;
constexpr int HEIGHT = 30;
constexpr int CELL_SIZE = 24;
constexpr int SCREEN_WIDTH = WIDTH * CELL_SIZE;   // 960
constexpr int SCREEN_HEIGHT = HEIGHT * CELL_SIZE; // 720

constexpr int BALL_SIZE = 16;
constexpr int PADDLE_WIDTH = 150;
constexpr int PADDLE_HEIGHT = 15;
constexpr int PADDLE_Y = SCREEN_HEIGHT - PADDLE_HEIGHT - 50; // 655
constexpr int PADDLE_SPEED = 11;

using Cell = std::pair<int, int>;
using Cells = std::vector<Cell>;

// -------------------------------------------------------------
// Game of Life core logic (pure, no I/O)
// -------------------------------------------------------------

inline bool alive(const Cells& cells, const Cell& cell) {
    return std::find(cells.begin(), cells.end(), cell) != cells.end();
}

inline std::vector<Cell> cellNeighbors(const Cell& cell) {
    int x = cell.first;
    int y = cell.second;
    return {
        {x - 1, y - 1}, {x, y - 1}, {x + 1, y - 1},
        {x - 1, y},
        {x + 1, y},
        {x - 1, y + 1}, {x, y + 1}, {x + 1, y + 1}
    };
}

inline int countAliveNeighbors(const Cells& cells, const Cell& cell) {
    int count = 0;
    for (const auto& n : cellNeighbors(cell)) {
        if (alive(cells, n)) ++count;
    }
    return count;
}

inline Cells nextGeneration(const Cells& cells) {
    std::set<Cell> candidateSet;
    for (const auto& cell : cells) {
        candidateSet.insert(cell);
        for (const auto& n : cellNeighbors(cell)) {
            candidateSet.insert(n);
        }
    }

    Cells result;
    for (const auto& cell : candidateSet) {
        bool currentlyAlive = alive(cells, cell);
        int neighbors = countAliveNeighbors(cells, cell);
        if (currentlyAlive) {
            if (neighbors == 2 || neighbors == 3) result.push_back(cell);
        } else {
            if (neighbors == 3) result.push_back(cell);
        }
    }
    return result;
}

inline Cells initCells() {
    Cells cells;
    // Glider 1 (top-left)
    cells.push_back({5, 4}); cells.push_back({6, 5});
    cells.push_back({4, 6}); cells.push_back({5, 6}); cells.push_back({6, 6});
    // Glider 2 (center-left)
    cells.push_back({15, 5}); cells.push_back({16, 6});
    cells.push_back({14, 7}); cells.push_back({15, 7}); cells.push_back({16, 7});
    // Glider 3 (center-right)
    cells.push_back({25, 4}); cells.push_back({26, 5});
    cells.push_back({24, 6}); cells.push_back({25, 6}); cells.push_back({26, 6});
    // Glider 4 (lower-center)
    cells.push_back({10, 12}); cells.push_back({11, 13});
    cells.push_back({9, 14}); cells.push_back({10, 14}); cells.push_back({11, 14});
    return cells;
}

// -------------------------------------------------------------
// Physics and collision (pure helpers)
// -------------------------------------------------------------

inline bool rectOverlap(float x1, float y1, float w1, float h1,
                        float x2, float y2, float w2, float h2) {
    return x1 < x2 + w2 && x1 + w1 > x2 &&
           y1 < y2 + h2 && y1 + h1 > y2;
}

struct BallState {
    float x, y, vx, vy;
    bool active;
};

struct PaddleState {
    float x, vx;
};

struct GameState {
    Cells cells;
    BallState ball;
    PaddleState paddle;
    int lives;
    int score;
    bool gameOver;
    int frameCounter;
};

inline void resetBall(GameState& state) {
    state.ball.active = false;
    state.ball.x = state.paddle.x + PADDLE_WIDTH / 2.0f - BALL_SIZE / 2.0f;
    state.ball.y = PADDLE_Y - BALL_SIZE - 2.0f;
    state.ball.vx = 0;
    state.ball.vy = 0;
}

inline void initGame(GameState& state) {
    state.cells = initCells();
    state.paddle.x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2.0f;
    state.paddle.vx = 0;
    state.lives = 3;
    state.score = 0;
    state.gameOver = false;
    state.frameCounter = 0;
    resetBall(state);
}

inline void shootBall(GameState& state, bool randomRight) {
    if (!state.ball.active) {
        state.ball.active = true;
        state.ball.x = state.paddle.x + PADDLE_WIDTH / 2.0f - BALL_SIZE / 2.0f;
        state.ball.y = PADDLE_Y - BALL_SIZE - 2.0f;
        state.ball.vx = randomRight ? 5.0f : -5.0f;
        state.ball.vy = -7.0f;
    }
}

enum class BallEvent { None, WallBounce, BottomMiss, PaddleHit, CellHit };

inline BallEvent updateBallPhysics(GameState& state) {
    if (!state.ball.active) return BallEvent::None;

    state.ball.x += state.ball.vx;
    state.ball.y += state.ball.vy;

    BallEvent event = BallEvent::None;

    if (state.ball.x <= 0) {
        state.ball.x = 0;
        state.ball.vx = -state.ball.vx;
        event = BallEvent::WallBounce;
    }
    if (state.ball.x + BALL_SIZE >= SCREEN_WIDTH) {
        state.ball.x = SCREEN_WIDTH - BALL_SIZE;
        state.ball.vx = -state.ball.vx;
        event = BallEvent::WallBounce;
    }
    if (state.ball.y <= 0) {
        state.ball.y = 0;
        state.ball.vy = -state.ball.vy;
        event = BallEvent::WallBounce;
    }
    if (state.ball.y >= SCREEN_HEIGHT) {
        state.lives -= 1;
        if (state.lives <= 0) {
            state.gameOver = true;
        } else {
            resetBall(state);
        }
        return BallEvent::BottomMiss;
    }
    return event;
}

inline bool checkPaddleCollision(GameState& state) {
    if (!rectOverlap(state.ball.x, state.ball.y, BALL_SIZE, BALL_SIZE,
                     state.paddle.x, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT)) {
        return false;
    }
    state.ball.vy = -std::abs(state.ball.vy);
    float hitPos = (state.ball.x + BALL_SIZE / 2.0f) - state.paddle.x;
    float relativeHit = hitPos / PADDLE_WIDTH;
    float angleFactor = relativeHit * 2.0f - 1.0f;
    state.ball.vx = angleFactor * 7.0f;
    return true;
}

inline bool checkCellCollisions(GameState& state) {
    Cells remaining;
    bool hit = false;
    for (size_t i = 0; i < state.cells.size(); ++i) {
        const auto& cell = state.cells[i];
        int cx = cell.first;
        int cy = cell.second;
        float bx = cx * CELL_SIZE;
        float by = cy * CELL_SIZE;

        if (rectOverlap(state.ball.x, state.ball.y, BALL_SIZE, BALL_SIZE,
                        bx, by, CELL_SIZE, CELL_SIZE)) {
            state.score += 10;
            float ballCenterX = state.ball.x + BALL_SIZE / 2.0f;
            float ballCenterY = state.ball.y + BALL_SIZE / 2.0f;
            float blockCenterX = bx + CELL_SIZE / 2.0f;
            float blockCenterY = by + CELL_SIZE / 2.0f;
            float dx = ballCenterX - blockCenterX;
            float dy = ballCenterY - blockCenterY;
            if (std::abs(dx) > std::abs(dy)) {
                state.ball.vx = -state.ball.vx;
            } else {
                state.ball.vy = -state.ball.vy;
            }
            // Remove hit cell and all remaining cells after it (matching Racket loop)
            for (size_t j = i + 1; j < state.cells.size(); ++j) {
                remaining.push_back(state.cells[j]);
            }
            state.cells = remaining;
            return true;
        }
        remaining.push_back(cell);
    }
    state.cells = remaining;
    return hit;
}

inline void updatePaddle(GameState& state) {
    state.paddle.x += state.paddle.vx;
    if (state.paddle.x < 0) state.paddle.x = 0;
    if (state.paddle.x > SCREEN_WIDTH - PADDLE_WIDTH) {
        state.paddle.x = SCREEN_WIDTH - PADDLE_WIDTH;
    }
}

inline void followBallToPaddle(GameState& state) {
    if (!state.ball.active) {
        state.ball.x = state.paddle.x + PADDLE_WIDTH / 2.0f - BALL_SIZE / 2.0f;
        state.ball.y = PADDLE_Y - BALL_SIZE - 2.0f;
    }
}

inline void tickLifeGeneration(GameState& state) {
    state.frameCounter += 1;
    if (state.frameCounter >= 20) {
        state.frameCounter = 0;
        state.cells = nextGeneration(state.cells);
    }
}

inline void checkStageClear(GameState& state) {
    if (state.cells.empty()) {
        state.cells = initCells();
        resetBall(state);
    }
}

#endif // GAME_LOGIC_HPP