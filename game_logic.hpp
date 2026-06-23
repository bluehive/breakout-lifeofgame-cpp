#ifndef GAME_LOGIC_HPP
#define GAME_LOGIC_HPP

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <set>
#include <utility>
#include <vector>

// -------------------------------------------------------------
// 定数と画面設定（breakout-lifeofgame.rkt と同一）
// Constants (matching breakout-lifeofgame.rkt)
// -------------------------------------------------------------
constexpr int WIDTH = 40;          // グリッド列数 / grid columns
constexpr int HEIGHT = 30;         // グリッド行数 / grid rows
constexpr int CELL_SIZE = 24;      // 1セルのピクセルサイズ / pixels per cell
constexpr int SCREEN_WIDTH = WIDTH * CELL_SIZE;   // 960
constexpr int SCREEN_HEIGHT = HEIGHT * CELL_SIZE; // 720

constexpr int BALL_SIZE = 16;
constexpr int PADDLE_WIDTH = 150;
constexpr int PADDLE_HEIGHT = 15;
constexpr int PADDLE_Y = SCREEN_HEIGHT - PADDLE_HEIGHT - 50; // パドルY座標 / paddle Y
constexpr int PADDLE_SPEED = 11;

using Cell = std::pair<int, int>;  // (x, y) グリッド座標
using Cells = std::vector<Cell>;

// -------------------------------------------------------------
// ライフゲームのコアロジック（純粋関数・I/O なし）
// Game of Life core logic (pure, no I/O)
// -------------------------------------------------------------

// セルが生存リストに含まれるか判定
inline bool alive(const Cells& cells, const Cell& cell) {
    return std::find(cells.begin(), cells.end(), cell) != cells.end();
}

// 周囲8方向の隣接セル座標を返す
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

// 隣接する生存セルの個数をカウント
inline int countAliveNeighbors(const Cells& cells, const Cell& cell) {
    int count = 0;
    for (const auto& n : cellNeighbors(cell)) {
        if (alive(cells, n)) ++count;
    }
    return count;
}

// 次世代の生存セルリストを計算（B3/S23 ルール）
inline Cells nextGeneration(const Cells& cells) {
    // 候補は「現在の生存セル」とその隣接セルのみ
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
            // 生存: 隣接2個または3個
            if (neighbors == 2 || neighbors == 3) result.push_back(cell);
        } else {
            // 誕生: 隣接ちょうど3個
            if (neighbors == 3) result.push_back(cell);
        }
    }
    return result;
}

// 初期状態: 画面上部に4つのグライダーを配置
inline Cells initCells() {
    Cells cells;
    // グライダー1（左上）
    cells.push_back({5, 4}); cells.push_back({6, 5});
    cells.push_back({4, 6}); cells.push_back({5, 6}); cells.push_back({6, 6});
    // グライダー2（中央左）
    cells.push_back({15, 5}); cells.push_back({16, 6});
    cells.push_back({14, 7}); cells.push_back({15, 7}); cells.push_back({16, 7});
    // グライダー3（中央右）
    cells.push_back({25, 4}); cells.push_back({26, 5});
    cells.push_back({24, 6}); cells.push_back({25, 6}); cells.push_back({26, 6});
    // グライダー4（中央下側）
    cells.push_back({10, 12}); cells.push_back({11, 13});
    cells.push_back({9, 14}); cells.push_back({10, 14}); cells.push_back({11, 14});
    return cells;
}

// -------------------------------------------------------------
// 物理演算と衝突判定（純粋ヘルパー）
// Physics and collision (pure helpers)
// -------------------------------------------------------------

// 2つの矩形が重なっているか（AABB 衝突判定）
inline bool rectOverlap(float x1, float y1, float w1, float h1,
                        float x2, float y2, float w2, float h2) {
    return x1 < x2 + w2 && x1 + w1 > x2 &&
           y1 < y2 + h2 && y1 + h1 > y2;
}

struct BallState {
    float x, y, vx, vy;  // 位置と速度（ピクセル/フレーム）
    bool active;         // 発射済みかどうか
};

struct PaddleState {
    float x, vx;  // 左端X座標と移動速度
};

struct GameState {
    Cells cells;
    BallState ball;
    PaddleState paddle;
    int lives;
    int score;
    bool gameOver;
    int frameCounter;  // ライフゲーム世代交代用（20フレームごと）
};

// ボールをパドル上の待機位置にリセット
inline void resetBall(GameState& state) {
    state.ball.active = false;
    state.ball.x = state.paddle.x + PADDLE_WIDTH / 2.0f - BALL_SIZE / 2.0f;
    state.ball.y = PADDLE_Y - BALL_SIZE - 2.0f;
    state.ball.vx = 0;
    state.ball.vy = 0;
}

// ゲーム全体を初期化（開始時・リスタート時）
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

// ボールを発射（スペースキー）
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

// ボール移動と壁・床との反射処理
inline BallEvent updateBallPhysics(GameState& state) {
    if (!state.ball.active) return BallEvent::None;

    state.ball.x += state.ball.vx;
    state.ball.y += state.ball.vy;

    BallEvent event = BallEvent::None;

    // 左右の壁で反射
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
    // 天井で反射
    if (state.ball.y <= 0) {
        state.ball.y = 0;
        state.ball.vy = -state.ball.vy;
        event = BallEvent::WallBounce;
    }
    // 画面底でミス（ライフ減少）
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

// パドル衝突: 当たり位置で反射角を変化
inline bool checkPaddleCollision(GameState& state) {
    if (!rectOverlap(state.ball.x, state.ball.y, BALL_SIZE, BALL_SIZE,
                     state.paddle.x, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT)) {
        return false;
    }
    // Y方向は常に上向きに反転
    state.ball.vy = -std::abs(state.ball.vy);
    float hitPos = (state.ball.x + BALL_SIZE / 2.0f) - state.paddle.x;
    float relativeHit = hitPos / PADDLE_WIDTH;       // 0.0〜1.0
    float angleFactor = relativeHit * 2.0f - 1.0f;   // -1.0〜1.0
    state.ball.vx = angleFactor * 7.0f;
    return true;
}

// セル（ブロック）衝突: 破壊・スコア加算・反射
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
            // 衝突面（上下か左右か）で反射方向を決定
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
            // 当たったセルを除外（貫通防止のため1フレーム1セル）
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

// パドル移動と画面端クリッピング
inline void updatePaddle(GameState& state) {
    state.paddle.x += state.paddle.vx;
    if (state.paddle.x < 0) state.paddle.x = 0;
    if (state.paddle.x > SCREEN_WIDTH - PADDLE_WIDTH) {
        state.paddle.x = SCREEN_WIDTH - PADDLE_WIDTH;
    }
}

// 未発射時はボールをパドル上に追従させる
inline void followBallToPaddle(GameState& state) {
    if (!state.ball.active) {
        state.ball.x = state.paddle.x + PADDLE_WIDTH / 2.0f - BALL_SIZE / 2.0f;
        state.ball.y = PADDLE_Y - BALL_SIZE - 2.0f;
    }
}

// 20フレームごとにライフゲームの世代交代
inline void tickLifeGeneration(GameState& state) {
    state.frameCounter += 1;
    if (state.frameCounter >= 20) {
        state.frameCounter = 0;
        state.cells = nextGeneration(state.cells);
    }
}

// 全セル破壊でステージクリア → グライダー再配置
inline void checkStageClear(GameState& state) {
    if (state.cells.empty()) {
        state.cells = initCells();
        resetBall(state);
    }
}

#endif // GAME_LOGIC_HPP