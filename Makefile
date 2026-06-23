# Breakout Game of Life — ビルド設定 / Build configuration
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

# raylib プリビルド版（GitHub Releases から取得）またはシステムインストール
RAYLIB_PREBUILT ?= vendor/raylib-prebuilt
RAYLIB_INC = -I$(RAYLIB_PREBUILT)/include
RAYLIB_A = $(RAYLIB_PREBUILT)/lib/libraylib.a
SYS_LIBS = /lib/x86_64-linux-gnu/libGLX.so.0 /lib/x86_64-linux-gnu/libX11.so.6 /lib/x86_64-linux-gnu/libGLdispatch.so.0

LDFLAGS = -L$(RAYLIB_PREBUILT)/lib -Wl,-rpath,'$$ORIGIN/$(RAYLIB_PREBUILT)/lib' $(RAYLIB_A) -lm -lpthread -ldl $(SYS_LIBS)

.PHONY: all test test-all clean

all: breakout-lifeofgame-cpp

breakout-lifeofgame-cpp: main.cpp game_app.hpp game_logic.hpp audio_helper.hpp bgm_helper.hpp
	$(CXX) $(CXXFLAGS) $(RAYLIB_INC) -o $@ main.cpp $(LDFLAGS)

test: test_logic
	./test_logic

test_integration: test_integration.cpp game_app.hpp game_logic.hpp audio_helper.hpp
	$(CXX) $(CXXFLAGS) $(RAYLIB_INC) -o $@ test_integration.cpp $(LDFLAGS)

test-all: test_logic test_integration
	./test_logic
	./test_integration

test_logic: test_logic.cpp game_logic.hpp
	$(CXX) $(CXXFLAGS) -o $@ test_logic.cpp

clean:
	rm -f breakout-lifeofgame-cpp test_logic test_integration