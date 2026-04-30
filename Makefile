CXX = g++
QT_CFLAGS = $(shell pkg-config --cflags Qt5Core)
QT_LIBS = $(shell pkg-config --libs Qt5Core)
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -fPIC $(QT_CFLAGS)
LDFLAGS = $(QT_LIBS)

SRC_DIR = src
BUILD_DIR = build
TARGET = ecu_smart_home

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

MOC_SRC = $(BUILD_DIR)/moc_AutomationRuleEngine.cpp
MOC_OBJ = $(BUILD_DIR)/moc_AutomationRuleEngine.o

all: $(TARGET)

$(TARGET): $(OBJS) $(MOC_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(MOC_SRC): include/AutomationRuleEngine.h | $(BUILD_DIR)
	moc $< -o $@

$(MOC_OBJ): $(MOC_SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all clean
