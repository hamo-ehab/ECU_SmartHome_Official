# =============================================================================
# Makefile — ECU Smart Home System
# Builds core logic and GUI layer separately, links into one executable.
# =============================================================================

CXX      = g++
STD      = -std=c++17

# ---- Qt (for AutomationRuleEngine QObject signals) -------------------------
QT_CFLAGS = $(shell pkg-config --cflags Qt5Core)
QT_LIBS   = $(shell pkg-config --libs Qt5Core)

# ---- SDL2 + OpenGL (for Dear ImGui renderer) --------------------------------
SDL_CFLAGS = $(shell pkg-config --cflags sdl2)
SDL_LIBS   = $(shell pkg-config --libs sdl2) -lGL -ldl

# ---- ImGui vendored sources (bundled in gui/imgui/) -------------------------
IMGUI_DIR  = gui/imgui
IMGUI_SRCS = $(IMGUI_DIR)/imgui.cpp \
             $(IMGUI_DIR)/imgui_draw.cpp \
             $(IMGUI_DIR)/imgui_tables.cpp \
             $(IMGUI_DIR)/imgui_widgets.cpp \
             $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
             $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
IMGUI_OBJS = $(patsubst gui/%.cpp, $(BUILD_DIR)/gui/%.o, $(IMGUI_SRCS))

# ---- Directories ------------------------------------------------------------
SRC_DIR   = src
GUI_DIR   = gui
BUILD_DIR = build
TARGET    = ecu_smart_home

# ---- Compiler flags ---------------------------------------------------------
CORE_CFLAGS = $(STD) -Wall -Wextra -fPIC \
              -Iinclude \
              $(QT_CFLAGS)

GUI_CFLAGS  = $(STD) -Wall -Wextra -fPIC \
              -Iinclude \
              -Igui/imgui \
              -Igui/imgui/backends \
              $(QT_CFLAGS) \
              $(SDL_CFLAGS)

IMGUI_CFLAGS = $(STD) -fPIC \
               -Igui/imgui \
               -Igui/imgui/backends \
               $(SDL_CFLAGS)

# ---- Source → object mappings ----------------------------------------------
# Core: everything in src/ except main.cpp compiles with CORE_CFLAGS
CORE_SRCS = $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp))
CORE_OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/core/%.o, $(CORE_SRCS))

# Main entry point (needs GUI flags to see gui/GuiManager.h)
MAIN_OBJ  = $(BUILD_DIR)/core/main.o

# GUI: GuiManager.cpp
GUI_SRCS  = $(wildcard $(GUI_DIR)/*.cpp)
GUI_OBJS  = $(patsubst $(GUI_DIR)/%.cpp, $(BUILD_DIR)/gui/%.o, $(GUI_SRCS))

# Qt MOC
MOC_SRC   = $(BUILD_DIR)/moc_AutomationRuleEngine.cpp
MOC_OBJ   = $(BUILD_DIR)/moc_AutomationRuleEngine.o

# ---- All objects for final link --------------------------------------------
ALL_OBJS  = $(CORE_OBJS) $(MAIN_OBJ) $(GUI_OBJS) $(IMGUI_OBJS) $(MOC_OBJ)

# ============================================================================
# Targets
# ============================================================================
.PHONY: all clean imgui_check

all: imgui_check $(TARGET)

# ---- Dependency guard: abort early if ImGui has not been downloaded --------
imgui_check:
	@test -f $(IMGUI_DIR)/imgui.h || \
	  (echo "" && \
	   echo "  ╔══════════════════════════════════════════════════════════╗" && \
	   echo "  ║  ImGui sources not found. Run bootstrap script first:    ║" && \
	   echo "  ║                                                          ║" && \
	   echo "  ║      bash gui/setup_imgui.sh                             ║" && \
	   echo "  ╚══════════════════════════════════════════════════════════╝" && \
	   echo "" && exit 1)

# ---- Final link ------------------------------------------------------------
$(TARGET): $(ALL_OBJS)
	$(CXX) -o $@ $^ $(QT_LIBS) $(SDL_LIBS)
	@echo ""
	@echo "  ✔ Build complete → $(TARGET)"
	@echo "  Run:  ./$(TARGET)"

# ---- Core objects ----------------------------------------------------------
$(BUILD_DIR)/core/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)/core
	$(CXX) $(CORE_CFLAGS) -c $< -o $@

# ---- main.o needs GUI include paths ----------------------------------------
$(BUILD_DIR)/core/main.o: $(SRC_DIR)/main.cpp | $(BUILD_DIR)/core
	$(CXX) $(GUI_CFLAGS) -c $< -o $@

# ---- GUI objects (GuiManager etc.) ----------------------------------------
$(BUILD_DIR)/gui/%.o: $(GUI_DIR)/%.cpp | $(BUILD_DIR)/gui
	$(CXX) $(GUI_CFLAGS) -c $< -o $@

# ---- ImGui vendored objects ------------------------------------------------
$(BUILD_DIR)/gui/imgui/%.o: $(GUI_DIR)/imgui/%.cpp | $(BUILD_DIR)/gui/imgui
	$(CXX) $(IMGUI_CFLAGS) -c $< -o $@

$(BUILD_DIR)/gui/imgui/backends/%.o: $(GUI_DIR)/imgui/backends/%.cpp | $(BUILD_DIR)/gui/imgui/backends
	$(CXX) $(IMGUI_CFLAGS) -c $< -o $@

# ---- Qt MOC ----------------------------------------------------------------
$(MOC_SRC): include/AutomationRuleEngine.h | $(BUILD_DIR)
	moc $< -o $@

$(MOC_OBJ): $(MOC_SRC)
	$(CXX) $(CORE_CFLAGS) -c $< -o $@

# ---- Directory creation ----------------------------------------------------
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/core:
	mkdir -p $(BUILD_DIR)/core

$(BUILD_DIR)/gui:
	mkdir -p $(BUILD_DIR)/gui

$(BUILD_DIR)/gui/imgui:
	mkdir -p $(BUILD_DIR)/gui/imgui

$(BUILD_DIR)/gui/imgui/backends:
	mkdir -p $(BUILD_DIR)/gui/imgui/backends

# ---- Clean -----------------------------------------------------------------
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "  ✔ Cleaned build artefacts."
