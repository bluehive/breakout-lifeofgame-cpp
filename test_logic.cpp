// コアロジックのユニットテスト（raylib 不要）
// Unit tests for core game logic (no raylib required)
#include "game_logic.hpp"
#include <cassert>
#include <cstdio>
#include <string>

static int testsRun = 0;
static int testsFailed = 0;

#define TEST(name) \
    do { \
        ++testsRun; \
        printf("  %s ... ", name); \
    } while (0)

#define PASS() do { printf("PASS\n"); } while (0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); ++testsFailed; } while (0)

static bool cellsEqual(const Cells& a, const Cells& b) {
    if (a.size() != b.size()) return false;
    for (const auto& c : a) {
        if (!alive(b, c)) return false;
    }
    return true;
}

static void test_init_cells() {
    TEST("initCells has 20 cells (4 gliders x 5)");
    Cells cells = initCells();
    if (cells.size() != 20) { FAIL("expected 20 cells"); return; }
    if (!alive(cells, {5, 4})) { FAIL("missing glider1 cell"); return; }
    if (!alive(cells, {11, 14})) { FAIL("missing glider4 cell"); return; }
    PASS();
}

static void test_alive_and_neighbors() {
    TEST("alive? and countAliveNeighbors");
    Cells cells = {{1, 1}, {2, 1}, {3, 1}};
    if (!alive(cells, {2, 1})) { FAIL("alive check failed"); return; }
    if (alive(cells, {0, 0})) { FAIL("should not be alive"); return; }
    if (countAliveNeighbors(cells, {2, 2}) != 3) { FAIL("neighbor count"); return; }
    PASS();
}

static void test_next_generation_blinker() {
    TEST("nextGeneration blinker oscillation");
    Cells h = {{1, 0}, {2, 0}, {3, 0}};
    Cells v = nextGeneration(h);
    Cells expected = {{2, -1}, {2, 0}, {2, 1}};
    if (!cellsEqual(v, expected)) { FAIL("vertical blinker"); return; }
    Cells h2 = nextGeneration(v);
    if (!cellsEqual(h2, h)) { FAIL("back to horizontal"); return; }
    PASS();
}

static void test_next_generation_glider_step() {
    TEST("nextGeneration single glider step");
    Cells glider = {{1, 0}, {2, 1}, {0, 2}, {1, 2}, {2, 2}};
    Cells next = nextGeneration(glider);
    Cells expected = {{0, 1}, {1, 2}, {1, 3}, {2, 1}, {2, 2}};
    if (!cellsEqual(next, expected)) { FAIL("glider step mismatch"); return; }
    PASS();
}

static void test_rect_overlap() {
    TEST("rectOverlap AABB");
    if (!rectOverlap(0, 0, 10, 10, 5, 5, 10, 10)) { FAIL("should overlap"); return; }
    if (rectOverlap(0, 0, 10, 10, 20, 20, 10, 10)) { FAIL("should not overlap"); return; }
    if (!rectOverlap(20, 20, 16, 16, 24, 24, 24, 24)) { FAIL("ball-cell overlap"); return; }
    PASS();
}

static void test_reset_ball_and_init_game() {
    TEST("resetBall and initGame positions");
    GameState state;
    initGame(state);
    if (state.lives != 3) { FAIL("lives"); return; }
    if (state.score != 0) { FAIL("score"); return; }
    if (state.gameOver) { FAIL("gameOver"); return; }
    if (state.cells.size() != 20) { FAIL("cells count"); return; }
    float expectedPaddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2.0f;
    if (state.paddle.x != expectedPaddleX) { FAIL("paddle x"); return; }
    if (state.ball.active) { FAIL("ball should be inactive"); return; }
    float expectedBallX = state.paddle.x + PADDLE_WIDTH / 2.0f - BALL_SIZE / 2.0f;
    if (state.ball.x != expectedBallX) { FAIL("ball x"); return; }
    if (state.ball.y != PADDLE_Y - BALL_SIZE - 2.0f) { FAIL("ball y"); return; }
    PASS();
}

static void test_shoot_ball() {
    TEST("shootBall sets velocity");
    GameState state;
    initGame(state);
    shootBall(state, true);
    if (!state.ball.active) { FAIL("not active"); return; }
    if (state.ball.vx != 5.0f) { FAIL("vx right"); return; }
    if (state.ball.vy != -7.0f) { FAIL("vy up"); return; }
    shootBall(state, false);
    if (state.ball.vx != 5.0f) { FAIL("should not re-shoot"); return; }
    PASS();
}

static void test_wall_bounce() {
    TEST("updateBallPhysics wall bounce");
    GameState state;
    initGame(state);
    state.ball.active = true;
    state.ball.x = 1;
    state.ball.y = 100;
    state.ball.vx = -5;
    state.ball.vy = 3;
    BallEvent ev = updateBallPhysics(state);
    if (state.ball.x != 0) { FAIL("clamped to 0"); return; }
    if (state.ball.vx != 5) { FAIL("vx reversed"); return; }
    if (ev != BallEvent::WallBounce) { FAIL("event type"); return; }
    PASS();
}

static void test_bottom_miss() {
    TEST("updateBallPhysics bottom miss");
    GameState state;
    initGame(state);
    state.ball.active = true;
    state.ball.x = 100;
    state.ball.y = SCREEN_HEIGHT;
    state.ball.vx = 3;
    state.ball.vy = 5;
    BallEvent ev = updateBallPhysics(state);
    if (state.lives != 2) { FAIL("lives decremented"); return; }
    if (state.ball.active) { FAIL("ball reset"); return; }
    if (ev != BallEvent::BottomMiss) { FAIL("event type"); return; }
    PASS();
}

static void test_paddle_collision() {
    TEST("checkPaddleCollision angle reflection");
    GameState state;
    initGame(state);
    state.paddle.x = 400;
    state.ball.active = true;
    state.ball.x = state.paddle.x + PADDLE_WIDTH / 2.0f - BALL_SIZE / 2.0f;
    state.ball.y = PADDLE_Y - BALL_SIZE + 1;
    state.ball.vx = 2;
    state.ball.vy = 5;
    bool hit = checkPaddleCollision(state);
    if (!hit) { FAIL("should hit paddle"); return; }
    if (state.ball.vy >= 0) { FAIL("vy should be up"); return; }
    if (state.ball.vx != 0.0f) { FAIL("center hit vx=0"); return; }
    PASS();
}

static void test_cell_collision() {
    TEST("checkCellCollisions destroy and reflect (horizontal side)");
    GameState state;
    initGame(state);
    state.cells = {{1, 1}};
    state.ball.active = true;
    // Approach from the left so |dx| > |dy| => vx reflection
    state.ball.x = 1 * CELL_SIZE - 4;
    state.ball.y = 1 * CELL_SIZE + 10;
    state.ball.vx = 5;
    state.ball.vy = 1;
    float vxBefore = state.ball.vx;
    bool hit = checkCellCollisions(state);
    if (!hit) { FAIL("should hit cell"); return; }
    if (state.score != 10) { FAIL("score +10"); return; }
    if (!state.cells.empty()) { FAIL("cell destroyed"); return; }
    if (state.ball.vx != -vxBefore) { FAIL("vx reflected"); return; }
    PASS();
}

static void test_stage_clear() {
    TEST("checkStageClear respawns cells");
    GameState state;
    initGame(state);
    state.cells.clear();
    checkStageClear(state);
    if (state.cells.size() != 20) { FAIL("cells respawned"); return; }
    if (state.ball.active) { FAIL("ball reset"); return; }
    PASS();
}

static void test_life_tick() {
    TEST("tickLifeGeneration at frame 20");
    GameState state;
    initGame(state);
    Cells initial = state.cells;
    state.frameCounter = 19;
    tickLifeGeneration(state);
    if (state.frameCounter != 0) { FAIL("counter reset"); return; }
    if (cellsEqual(state.cells, initial)) { FAIL("cells should evolve"); return; }
    PASS();
}

int main() {
    printf("Running game logic unit tests...\n");
    test_init_cells();
    test_alive_and_neighbors();
    test_next_generation_blinker();
    test_next_generation_glider_step();
    test_rect_overlap();
    test_reset_ball_and_init_game();
    test_shoot_ball();
    test_wall_bounce();
    test_bottom_miss();
    test_paddle_collision();
    test_cell_collision();
    test_stage_clear();
    test_life_tick();

    printf("\n%d tests run, %d failed\n", testsRun, testsFailed);
    return testsFailed == 0 ? 0 : 1;
}