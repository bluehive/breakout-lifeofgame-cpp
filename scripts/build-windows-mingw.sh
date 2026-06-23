#!/usr/bin/env bash
# MinGW-w64 静的リンクビルド（Win10/11 向け単体 exe）
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

RAYLIB_DIR="${RAYLIB_DIR:-vendor/raylib-prebuilt}"
RAYLIB_VERSION="${RAYLIB_VERSION:-6.0}"
RAYLIB_ZIP="raylib-${RAYLIB_VERSION}_win64_mingw-w64.zip"
RAYLIB_URL="https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/${RAYLIB_ZIP}"

if [[ ! -f "${RAYLIB_DIR}/lib/libraylib.a" ]]; then
  mkdir -p "${RAYLIB_DIR}"
  curl -sL "${RAYLIB_URL}" -o "/tmp/${RAYLIB_ZIP}"
  unzip -qo "/tmp/${RAYLIB_ZIP}" -d "${RAYLIB_DIR}"
  SUBDIR="${RAYLIB_DIR}/raylib-${RAYLIB_VERSION}_win64_mingw-w64"
  if [[ -d "${SUBDIR}" ]]; then
    mv "${SUBDIR}/include" "${SUBDIR}/lib" "${RAYLIB_DIR}/"
    rm -rf "${SUBDIR}"
  fi
fi

echo "Building breakout-lifeofgame-cpp.exe (static MinGW)..."
g++ -std=c++11 -Wall -Wextra -O2 \
  -I"${RAYLIB_DIR}/include" \
  -o breakout-lifeofgame-cpp.exe main.cpp \
  "${RAYLIB_DIR}/lib/libraylib.a" \
  -lopengl32 -lgdi32 -lwinmm -lole32 \
  -static-libgcc -static-libstdc++ -static

echo "Building test_logic.exe..."
g++ -std=c++11 -Wall -Wextra -O2 -o test_logic.exe test_logic.cpp
./test_logic.exe

echo "Running --self-test..."
./breakout-lifeofgame-cpp.exe --self-test

echo "Windows build OK"