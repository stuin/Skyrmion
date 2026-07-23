CFLAGS := -Wall -fpermissive -Wno-unused-function
CXXFLAGS := -std=c++20

# Backend Files
backend ?= raylib
ifeq ($(backend), raylib)
	CORE_FILES := ${CORE_FILES} core/backend/RaylibUpdateList.o core/backend/RaylibAudio.o
	LDFLAGS := ${LDFLAGS} -lglfw -lGL -ldl -lm -lpthread -lX11 -ldl -lrt
	WLDFLAGS := --static -lglfw3 -lgdi32 -lwinmm -lcomdlg32 -lole32 -lws2_32 -static-libstdc++ -static-libgcc
	WLDFLAGS += src/Skyrmion/include/raylib/src/raylib.rc.data
else ifeq ($(backend), sokol)
	CORE_FILES := ${CORE_FILES} core/backend/SokolUpdateList.o core/backend/SokolAudio.o
	LDFLAGS := ${LDFLAGS} -lglfw -lGL -ldl -lm -lpthread -lX11 -ldl -lasound -lXi -lXcursor
	WLDFLAGS := --static -lglfw3 -lgdi32 -lwinmm -lcomdlg32 -lole32 -lws2_32 -static-libstdc++ -static-libgcc
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

	ifndef debug
		LDFLAGS += -Wl,--subsystem,windows
	endif

	EXEC = exe
	PLATFORM = Windows
	BUILD_DIR = build/windows
else ifeq ($(platform), web)
	CC=emcc
	CXX = em++
	CFLAGS := ${CFLAGS} -Os -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
	CORE_FILES := ${CORE_FILES} core/backend/nullClient.o
	LDFLAGS := ${LDFLAGS} -s USE_GLFW=3 --shell-file src/Skyrmion/include/raylib/src/minshell.html --preload-file res

	EXEC = html
	PLATFORM = Web
	BUILD_DIR = build/web
endif

# Debug addons
ifdef debug
	ifeq ($(platform), linux)
		LDFLAGS := ${LDFLAGS} -lbfd
		CXXFLAGS := ${CXXFLAGS} -DBACKWARD_HAS_BFD=1
		CORE_FILES := ${CORE_FILES} include/backward-cpp/backward.o
		SERVER_FILES := ${SERVER_FILES} include/backward-cpp/backward.o
	else ifeq ($(platform), web)
		LDFLAGS := ${LDFLAGS} -sNO_DISABLE_EXCEPTION_CATCHING
	endif

	CXXFLAGS := ${CXXFLAGS} -g -D_DEBUG=1
	CORE_FILES := ${CORE_FILES} debug/DebugTools.o

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

IMGUI_OBJS := $(IMGUI_FILES:%=include/imgui/%)
RAYLIB_OBJS := $(RAYLIB_FILES:%=include/raylib/src/%)
INCLUDE_FILES := include/rlImGui/rlImGui.o $(IMGUI_OBJS) $(RAYLIB_OBJS)

INCLUDE_PATHS := ${INCLUDE_PATHS} -I. -Isrc/Skyrmion/include/raylib/src/ -Isrc/Skyrmion/include/imgui

# Full lists
SKYRMION_OBJS := $(SKYRMION_FILES:%=$(BUILD_DIR)/src/Skyrmion/%)
SERVER_OBJS := $(SERVER_FILES:%=$(BUILD_DIR)/src/Skyrmion/%)
GAME_OBJS := $(GAME_FILES:%=$(BUILD_DIR)/src/%)
INCLUDE_OBJS := $(INCLUDE_FILES:%=$(BUILD_DIR)/src/Skyrmion/%)
OBJS = $(GAME_OBJS) $(SKYRMION_OBJS) $(INCLUDE_OBJS)

# .h Dependencies
SKYRMION_DEPENDS := $(patsubst %.o,%.d,$(SKYRMION_OBJS)) $(patsubst %.o,%.d,$(SERVER_OBJS))
GAME_DEPENDS := $(patsubst %.o,%.d,$(GAME_OBJS))
DEPENDS := $(SKYRMION_DEPENDS) $(GAME_DEPENDS)

# Linking execs
game: $(OBJS)
	$(CXX) $(OBJS) -o $(BUILD_DIR)/$(GAME_NAME).$(EXEC) $(LDFLAGS) $(INCLUDE_PATHS)

server: $(SERVER_OBJS)
	$(CXX) $(SERVER_OBJS) -o $(BUILD_DIR)/$(GAME_NAME)-server.$(EXEC) $(LDFLAGS)

# Compilation
$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS) -MMD -MP $(INCLUDE_PATHS)

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ $(CFLAGS) $(CXXFLAGS) -MMD -MP $(INCLUDE_PATHS)

-include $(DEPENDS)

# Zip Release
TARGET_NAME = $(GAME_NAME)_$(PLATFORM)_$(VERSION)
TARGET_DIR = $(BUILD_DIR)/$(TARGET_NAME)

zip: game
	mkdir -p $(TARGET_DIR)
	cp -r res $(TARGET_DIR)
	cp $(BUILD_DIR)/$(GAME_NAME).* $(TARGET_DIR)
ifeq ($(platform), web)
	mv $(TARGET_DIR)/$(GAME_NAME).html $(TARGET_DIR)/index.html
endif
	cd $(BUILD_DIR) ; zip -r $(TARGET_NAME).zip $(TARGET_NAME)

# Test run
run: game
ifeq ($(platform), web)
	cd $(BUILD_DIR) ; python -m http.server 12345
else
	$(BUILD_DIR)/$(GAME_NAME).$(EXEC)
endif

# Other
.PHONY: all clean game

all: game

clean:
	rm -r $(BUILD_DIR)

clean-all:
	rm -r build

# Usage:
# make zip
# make debug=1
# make platform=web
# make platform=web run

# Example Project Makefile:
#	GAME_NAME = ShaderToys
#	GAME_FILES = main.o
#	VERSION = 1.0
#	include src/Skyrmion/Makefile