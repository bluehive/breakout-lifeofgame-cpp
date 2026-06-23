# Breakout Game of Life (C++ / raylib)

**ブロック崩し × ライフゲーム（C++ / raylib 版）**

A C++ port of [breakout-lifeofgame.rkt](https://github.com/bluehive/lifeofgame-racket/blob/main/breakout-lifeofgame.rkt). This hybrid game combines classic Breakout with Conway's Game of Life (B3/S23).

[lifeofgame-racket](https://github.com/bluehive/lifeofgame-racket) の [breakout-lifeofgame.rkt](https://github.com/bluehive/lifeofgame-racket/blob/main/breakout-lifeofgame.rkt) を C++ に移植したハイブリッドゲームです。クラシックなブロック崩しとライフゲーム（B3/S23 ルール）を組み合わせています。

---

## Features / 機能

| English | 日本語 |
|---------|--------|
| 40×30 grid of living cells (four gliders at start) | 40×30 の生存セルグリッド（初期配置は4つのグライダー） |
| Conway's Game of Life (B3/S23) every 20 frames | 20フレームごとにライフゲームの世代交代（B3/S23） |
| Orange paddle, red ball, AABB collision | オレンジのパドル、赤いボール、AABB 衝突判定 |
| Score +10 per cell, 3 lives, stage clear respawns gliders | セル破壊で+10点、残機3、全破壊でグライダー再配置 |
| Synthesized sound effects | 合成トーンによる効果音（バウンド・ヒット・破壊・ミス） |

---

## Controls / 操作方法

| Key / キー | Action / 操作 |
|------------|---------------|
| Left Arrow / 左矢印 | Move paddle left / パドルを左へ |
| Right Arrow / 右矢印 | Move paddle right / パドルを右へ |
| Space / スペース | Shoot ball / Restart after game over / 発射・ゲームオーバー後リスタート |

---

## Requirements / 動作環境

| English | 日本語 |
|---------|--------|
| C++11 compiler (`g++`) | C++11 対応コンパイラ（`g++`） |
| [raylib](https://www.raylib.com/) 4.x–6.x | [raylib](https://www.raylib.com/) 4.x〜6.x |
| Linux: OpenGL, X11, PulseAudio | Linux: OpenGL、X11、PulseAudio（音声） |

---

## Build / ビルド

### Quick start (prebuilt raylib 6.0) / クイックスタート

```bash
mkdir -p vendor/raylib-prebuilt
curl -sL https://github.com/raysan5/raylib/releases/download/6.0/raylib-6.0_linux_amd64.tar.gz \
  | tar xz -C vendor/raylib-prebuilt --strip-components=1
make
```

### System raylib / システムの raylib を使う場合

```bash
# Debian/Ubuntu (if available / 利用可能な場合)
sudo apt install libraylib-dev
g++ -std=c++11 -o breakout-lifeofgame-cpp main.cpp -lraylib -lm -lpthread -ldl -lGL -lX11
```

### Tests / テスト

```bash
make test-all    # logic + integration / ロジック＋統合テスト
make test        # logic only / ロジックのみ
```

### Headless self-test / ヘッドレス自己テスト

```bash
./breakout-lifeofgame-cpp --self-test
```

---

## Run / 実行

```bash
./breakout-lifeofgame-cpp
```

Window: 960×720 px, 60 FPS  
画面サイズ: 960×720 ピクセル、60 FPS

---

## Prebuilt binaries / ビルド済みバイナリ

Every push to `main` triggers a [GitHub Actions](.github/workflows/release.yml) workflow that publishes a Linux amd64 binary.

`main` ブランチへの push ごとに [GitHub Actions](.github/workflows/release.yml) が Linux amd64 バイナリを [Releases](https://github.com/bluehive/breakout-lifeofgame-cpp/releases) に公開します。

1. Open [Releases](https://github.com/bluehive/breakout-lifeofgame-cpp/releases) / [Releases](https://github.com/bluehive/breakout-lifeofgame-cpp/releases) を開く
2. Download `breakout-lifeofgame-cpp-linux-amd64.tar.gz` / 最新の tar.gz をダウンロード
3. Extract and run / 展開して実行:

```bash
tar xzf breakout-lifeofgame-cpp-linux-amd64.tar.gz
./breakout-lifeofgame-cpp
```

---

## Project structure / ファイル構成

| File | Purpose / 役割 |
|------|----------------|
| `game_logic.hpp` | Pure game rules (Life, physics, collisions) — testable without raylib / 純粋ロジック（テスト可能） |
| `game_app.hpp` | Drawing, input, gameTick loop / 描画・入力・ゲームループ |
| `main.cpp` | Raylib window and main entry / raylib ウィンドウとエントリポイント |
| `audio_helper.hpp` | Synthesized tone sounds / 合成トーン効果音 |
| `test_logic.cpp` | Unit tests / ユニットテスト |
| `test_integration.cpp` | Integration tests / 統合テスト |

---

## License / ライセンス

MIT License — see [LICENSE](LICENSE).  
MIT ライセンス — 詳細は [LICENSE](LICENSE) を参照してください。