CFLAGS := -Wall -fpermissive -Wno-unused-function
CXXFLAGS := -std=c++20

# Backend Files
backend ?= raylib
ifeq ($(backend), raylib)
	CORE_FILES := ${CORE_FILES} core/backend/RaylibUpdateList.o
	LDFLAGS := ${LDFLAGS} -lglfw -lGL -ldl -lm -lpthread -lX11 -ldl -lrt
	WLDFLAGS := --static -lglfw3 -lgdi32 -lwinmm -lws2_32 -static-libstdc++ -static-libgcc
else ifeq ($(backend), sokol)
	CORE_FILES := ${CORE_FILES} core/backend/SokolUpdateList.o core/backend/SokolAudio.o
	LDFLAGS := ${LDFLAGS} -lglfw -lGL -ldl -lm -lpthread -lX11 -ldl -lasound -lXi -lXcursor
	WLDFLAGS := --static -lglfw3 -lgdi32 -lwinmm -lws2_32 -static-libstdc++ -static-libgcc
endif

# Platform Args
ifndef platform
	CC=gcc
	CXX = g++
	CFLAGS := ${CFLAGS} -O3 -DPLATFORM_DESKTOP
	CORE_FILES := ${CORE_FILES} core/backend/nbnetClient.o

	ifeq ($(OS),Windows_NT)
		LDFLAGS = $(WLDFLAGS)
		EXEC = exe
		PLATFORM = Windows
		BUILD_DIR = build/windows
	else
		EXEC = out
		PLATFORM = Linux
		BUILD_DIR = build/linux
	endif
else ifeq ($(platform), windows)
	CC=x86_64-w64-mingw32-gcc
	CXX = x86_64-w64-mingw32-g++
	CFLAGS := ${CFLAGS} -O3 -DPLATFORM_DESKTOP
	CORE_FILES := ${CORE_FILES} core/backend/nbnetClient.o
	LDFLAGS = $(WLDFLAGS)

	EXEC = exe
	PLATFORM = Windows
	BUILD_DIR = build/windows
else ifeq ($(platform), web)
	CC=emcc
	CXX = em++
	CFLAGS := ${CFLAGS} -Os -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
	CORE_FILES := ${CORE_FILES} core/backend/nullClient.o
	LDFLAGS := ${LDFLAGS} -s USE_GLFW=3 -s ASYNCIFY --shell-file src/Skyrmion/include/raylib/src/minshell.html --preload-file res

	EXEC = html
	PLATFORM = Web
	BUILD_DIR = build/web
endif

# Debug addons
ifdef debug
	LDFLAGS := ${LDFLAGS} -lbfd
	CXXFLAGS := ${CXXFLAGS} -g -D _DEBUG -DBACKWARD_HAS_BFD=1
	CORE_FILES := ${CORE_FILES} include/backward-cpp/backward.o debug/DebugTools.o
	SERVER_FILES := ${SERVER_FILES} include/backward-cpp/backward.o

	BUILD_DIR := ${BUILD_DIR}-debug
	VERSION := ${VERSION}d
endif

# Skyrmion File List
CORE_FILES := ${CORE_FILES} core/Node.o core/RenderComponents.o core/Vector.o
INPUT_FILES := input/InputHandler.o input/Keymap.o input/MovementSystems.o input/Settings.o
TILING_FILES := tiling/GridMaker.o tiling/LightMap.o tiling/SquareTiles.o
SKYRMION_FILES := $(CORE_FILES) $(INPUT_FILES) $(TILING_FILES)

SERVER_FILES := ${SERVER_FILES} core/backend/nbnetServer.o

# Dependency File Lists
IMGUI_FILES := imgui.o imgui_demo.o imgui_draw.o imgui_tables.o imgui_widgets.o
RAYLIB_FILES := rcore.o rshapes.o rtextures.o rtext.o rmodels.o raudio.o
LIBNOISE_FILES1 := noisegen.o latlon.o model/line.o model/plane.o model/sphere.o model/cylinder.o
LIBNOISE_FILES2 := $(wildcard src/Skyrmion/include/libnoise/src/module/*.cpp)

IMGUI_OBJS := $(IMGUI_FILES:%=include/imgui/%)
RAYLIB_OBJS := $(RAYLIB_FILES:%=include/raylib/src/%)
LIBNOISE_OBJS := $(LIBNOISE_FILES1:%=include/libnoise/src/%) $(LIBNOISE_FILES2:src/Skyrmion/%.cpp=%.o)
INCLUDE_FILES := include/rlImGui/rlImGui.o $(IMGUI_OBJS) $(RAYLIB_OBJS) $(LIBNOISE_OBJS)

INCLUDE_PATHS := ${INCLUDE_PATHS} -I. -Isrc/Skyrmion/include/raylib/src/ -Isrc/Skyrmion/include/imgui -Isrc/Skyrmion/include/libnoise/src/noise

# Compilation
SKYRMION_OBJS := $(SKYRMION_FILES:%=$(BUILD_DIR)/src/Skyrmion/%) $(INCLUDE_FILES:%=$(BUILD_DIR)/src/Skyrmion/%)
SERVER_OBJS := $(SERVER_FILES:%=$(BUILD_DIR)/src/Skyrmion/%)
GAME_OBJS := $(GAME_FILES:%=$(BUILD_DIR)/src/%)
OBJS = $(GAME_OBJS) $(SKYRMION_OBJS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $^ -o $@ $(CFLAGS) $(INCLUDE_PATHS)

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $^ -o $@ $(CFLAGS) $(CXXFLAGS) $(INCLUDE_PATHS)

game: $(OBJS)
	$(CXX) $(OBJS) -o $(BUILD_DIR)/$(GAME_NAME).$(EXEC) $(LDFLAGS) $(INCLUDE_PATHS)

server: $(SERVER_OBJS)
	$(CXX) $(SERVER_OBJS) -o $(BUILD_DIR)/$(GAME_NAME)-server.$(EXEC) $(LDFLAGS)

# Final targets
TARGET_NAME = $(GAME_NAME)_$(PLATFORM)_$(VERSION)
TARGET_DIR = $(BUILD_DIR)/$(TARGET_NAME)

all: game

zip: game
	mkdir -p $(TARGET_DIR)
	cp -r res $(TARGET_DIR)
	cp $(BUILD_DIR)/$(GAME_NAME).* $(TARGET_DIR)
	cd $(BUILD_DIR) ; zip -r $(TARGET_NAME).zip $(TARGET_NAME)

clean:
	rm -r $(BUILD_DIR)

clean-all:
	rm -r build

# Usage:
# make zip
# make all debug=1
# make all platform=web

# Example Project Makefile:
#	GAME_NAME = ShaderToys
#	GAME_FILES = main.o
#	include src/Skyrmion/Makefile