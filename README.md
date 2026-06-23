# Breakout Game of Life (C++ / raylib)

A C++ port of [breakout-lifeofgame.rkt](https://github.com/bluehive/lifeofgame-racket/blob/main/breakout-lifeofgame.rkt) from the lifeofgame-racket project. This hybrid game combines classic Breakout paddle-and-ball mechanics with Conway's Game of Life (B3/S23): black cell blocks evolve every 20 frames while you destroy them with a red ball.

## Features

- 40×30 grid of living cells (four gliders at start)
- Conway's Game of Life rules (B3/S23) applied every 20 frames
- Orange paddle, red ball, AABB collision detection
- Score (+10 per cell), 3 lives, stage clear respawns gliders
- Simple synthesized sound effects (bounce, paddle hit, cell destroy, life lost)

## Controls

| Key | Action |
|-----|--------|
| Left Arrow | Move paddle left |
| Right Arrow | Move paddle right |
| Space | Shoot ball (when ready) / Restart after game over |

## Requirements

- C++11 compiler (`g++`)
- [raylib](https://www.raylib.com/) 4.x or 5.x (or 6.0 prebuilt)
- Linux: OpenGL, X11, PulseAudio (for sound)

## Build

### Quick start (prebuilt raylib 6.0)

```bash
mkdir -p vendor/raylib-prebuilt
curl -sL https://github.com/raysan5/raylib/releases/download/6.0/raylib-6.0_linux_amd64.tar.gz \
  | tar xz -C vendor/raylib-prebuilt --strip-components=1
make
```

### System raylib

```bash
# Debian/Ubuntu (if available)
sudo apt install libraylib-dev
g++ -std=c++11 -o breakout-lifeofgame-cpp main.cpp -lraylib -lm -lpthread -ldl -lGL -lX11
```

### Unit tests (no raylib required)

```bash
make test
# or: g++ -std=c++11 -o test_logic test_logic.cpp && ./test_logic
```

## Run

```bash
./breakout-lifeofgame-cpp
```

Window size: 960×720 pixels at 60 FPS.

## Project structure

| File | Purpose |
|------|---------|
| `game_logic.hpp` | Pure game rules (Life, physics, collisions) — testable without raylib |
| `main.cpp` | Raylib window, input, rendering, game loop |
| `audio_helper.hpp` | Synthesized tone generation via raylib Wave/Sound |
| `test_logic.cpp` | Unit tests for core logic |

## License

MIT License — see [LICENSE](LICENSE).