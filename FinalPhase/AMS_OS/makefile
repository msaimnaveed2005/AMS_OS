# AMS OS - Atomic Management System Simulator
# Linux / Ubuntu build configuration

CXX ?= g++
CPPFLAGS ?= -I.
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -pthread -O3 -flto -march=native
LDFLAGS ?= -pthread -O3 -flto
.DEFAULT_GOAL := all

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib/ams-os

BUILD_DIR := build
DATA_DIR := data
TASK_DIR := tasks
KERNEL_DIR := kernel
GUI_DIR := gui

OS_BIN := OS
GUI_BIN := $(BUILD_DIR)/ams_desktop

# GTK3 flags (only evaluated when GUI target is built)
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0 2>/dev/null)
GTK_LIBS   = $(shell pkg-config --libs   gtk+-3.0 2>/dev/null)

KERNEL_SRC := $(wildcard $(KERNEL_DIR)/*.cpp)
KERNEL_HEADERS := $(wildcard $(KERNEL_DIR)/*.h)

# ── Terminal-based tasks (CMD mode) ──
TASK_SRCS := $(wildcard $(TASK_DIR)/*.cpp)
TASK_BINS := $(patsubst $(TASK_DIR)/%.cpp,$(BUILD_DIR)/%,$(TASK_SRCS))

# ── GUI task applications (GUI mode) ──
GUI_TASK_SRCS := $(wildcard $(GUI_DIR)/tasks/*.cpp)
GUI_TASK_BINS := $(patsubst $(GUI_DIR)/tasks/%.cpp,$(BUILD_DIR)/gui_%,$(GUI_TASK_SRCS))
GUI_THEME_H := $(GUI_DIR)/gui_theme.h

.PHONY: all cli dirs run run-cli gui gui-tasks run-gui check install uninstall clean distclean help

all: dirs $(OS_BIN) $(TASK_BINS) gui
	@echo "Build complete. Run './OS' for CMD mode or 'make run-gui' for GUI mode."

cli: dirs $(OS_BIN) $(TASK_BINS)
	@echo "CLI build complete. Run './OS' or 'make run-cli' to start."

dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(DATA_DIR)/virtual_disk

$(OS_BIN): $(KERNEL_SRC) $(KERNEL_HEADERS) | dirs
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(KERNEL_SRC) $(LDFLAGS) -o $@

$(BUILD_DIR)/%: $(TASK_DIR)/%.cpp $(KERNEL_DIR)/ui.h | dirs
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(LDFLAGS) -o $@

# ── GUI desktop + all GUI tasks (requires libgtk-3-dev) ──
gui: $(GUI_BIN) gui-tasks

gui-tasks: $(GUI_TASK_BINS)

$(GUI_BIN): $(GUI_DIR)/ams_desktop.cpp | dirs
	@if pkg-config --exists gtk+-3.0 2>/dev/null; then \
		echo "Building GUI desktop..."; \
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(GTK_CFLAGS) $< $(LDFLAGS) $(GTK_LIBS) -o $@; \
	else \
		echo "GTK3 not found. Install with: sudo apt install libgtk-3-dev"; \
		echo "Skipping GUI build. CMD mode (./OS) is still available."; \
	fi

# Pattern rule for GUI task apps: gui/tasks/X.cpp → build/gui_X
$(BUILD_DIR)/gui_%: $(GUI_DIR)/tasks/%.cpp $(GUI_THEME_H) | dirs
	@if pkg-config --exists gtk+-3.0 2>/dev/null; then \
		echo "  GTK  $@"; \
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(GUI_DIR) $(GTK_CFLAGS) $< $(LDFLAGS) $(GTK_LIBS) -o $@; \
	else \
		echo "GTK3 not found — skipping $@"; \
	fi

run: all
	./$(OS_BIN) 2 256 8

run-cli: cli
	./$(OS_BIN) 2 256 8

run-gui: all
	./$(GUI_BIN) 2 256 8

check:
	@command -v $(CXX) >/dev/null || { echo "Missing compiler: $(CXX)"; exit 1; }
	@command -v gnome-terminal >/dev/null || \
	 command -v x-terminal-emulator >/dev/null || \
	 command -v xfce4-terminal >/dev/null || \
	 { echo "Missing terminal emulator: install gnome-terminal or x-terminal-emulator"; exit 1; }
	@pkg-config --exists gtk+-3.0 2>/dev/null && echo "GTK3: OK" || echo "GTK3: Missing (install libgtk-3-dev for GUI mode)"
	@echo "Ubuntu build prerequisites look available."

install: check all
	install -d "$(DESTDIR)$(BINDIR)"
	install -d "$(DESTDIR)$(LIBDIR)/build"
	install -d "$(DESTDIR)$(LIBDIR)/data/virtual_disk"
	install -m 755 "$(OS_BIN)" "$(DESTDIR)$(LIBDIR)/OS"
	install -m 755 $(TASK_BINS) "$(DESTDIR)$(LIBDIR)/build/"
	-install -m 755 $(GUI_TASK_BINS) "$(DESTDIR)$(LIBDIR)/build/" 2>/dev/null
	-install -m 755 "$(GUI_BIN)" "$(DESTDIR)$(LIBDIR)/build/" 2>/dev/null
	printf '%s\n' '#!/bin/sh' 'cd "$(LIBDIR)" || exit 1' 'exec ./OS "$$@"' > "$(DESTDIR)$(BINDIR)/ams-os"
	chmod 755 "$(DESTDIR)$(BINDIR)/ams-os"
	@echo "Installed AMS OS to $(DESTDIR)$(LIBDIR)"
	@echo "Run with: $(DESTDIR)$(BINDIR)/ams-os 2 256 8"

uninstall:
	rm -f "$(DESTDIR)$(BINDIR)/ams-os"
	rm -rf "$(DESTDIR)$(LIBDIR)"
	@echo "Uninstalled AMS OS from $(DESTDIR)$(PREFIX)"

clean:
	rm -f "$(OS_BIN)"
	rm -f $(TASK_BINS)
	rm -f $(GUI_TASK_BINS)
	rm -f "$(GUI_BIN)"
	rm -f "$(DATA_DIR)/system_log.txt"
	@echo "Clean complete."

distclean: clean
	rm -rf "$(BUILD_DIR)"
	rm -rf "$(DATA_DIR)/virtual_disk"
	@echo "Distribution clean complete."

help:
	@echo "AMS OS Makefile targets"
	@echo "  make          Build kernel, CMD tasks, GUI desktop + GUI tasks"
	@echo "  make run      Build and start CMD mode (2 GB RAM, 256 GB HDD, 8 cores)"
	@echo "  make gui      Build GUI desktop + all 26 GUI task apps"
	@echo "  make gui-tasks Build only the 26 GUI task apps"
	@echo "  make run-gui  Build and start GUI mode"
	@echo "  make check    Verify compiler, terminal emulator, and GTK3 availability"
	@echo "  make install  Install under PREFIX=$(PREFIX)"
	@echo "  make clean    Remove binaries and runtime log"
	@echo "  make distclean Remove generated build and virtual disk directories"
	@echo "  make uninstall Remove installed files"
	@echo "  make help     Show this message"
