# AMS OS - Atomic Management System Simulator
# Linux / Ubuntu build configuration

CXX ?= g++
CPPFLAGS ?= -I.
CXXFLAGS ?= -std=c++17 -Wall -Wextra -pedantic -pthread
LDFLAGS ?= -pthread

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib/ams-os

BUILD_DIR := build
DATA_DIR := data
TASK_DIR := tasks
KERNEL_DIR := kernel

OS_BIN := OS

KERNEL_SRC := \
	$(KERNEL_DIR)/main.cpp \
	$(KERNEL_DIR)/resource_manager.cpp \
	$(KERNEL_DIR)/process_manager.cpp \
	$(KERNEL_DIR)/task_catalog.cpp \
	$(KERNEL_DIR)/ready_queue.cpp \
	$(KERNEL_DIR)/scheduler.cpp \
	$(KERNEL_DIR)/logger.cpp \
	$(KERNEL_DIR)/deadlock_manager.cpp \
	$(KERNEL_DIR)/sync_manager.cpp

KERNEL_HEADERS := \
	$(KERNEL_DIR)/resource_manager.h \
	$(KERNEL_DIR)/process_manager.h \
	$(KERNEL_DIR)/task_catalog.h \
	$(KERNEL_DIR)/ready_queue.h \
	$(KERNEL_DIR)/scheduler.h \
	$(KERNEL_DIR)/logger.h \
	$(KERNEL_DIR)/deadlock_manager.h \
	$(KERNEL_DIR)/sync_manager.h \
	$(KERNEL_DIR)/ui.h

TASK_NAMES := \
	calculator \
	calendar \
	clock \
	create_file \
	delete_file \
	download_simulator \
	file_copy \
	file_info \
	minesweeper \
	move_file \
	music_player \
	notepad \
	process_killer \
	snake \
	system_info \
	task_manager

TASK_BINS := $(addprefix $(BUILD_DIR)/,$(TASK_NAMES))

.PHONY: all dirs run check install uninstall clean distclean help

all: dirs $(OS_BIN) $(TASK_BINS)
	@echo "Build complete. Run './OS' or 'make run' to start AMS OS."

dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(DATA_DIR)/virtual_disk

$(OS_BIN): $(KERNEL_SRC) $(KERNEL_HEADERS) | dirs
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(KERNEL_SRC) $(LDFLAGS) -o $@

$(BUILD_DIR)/%: $(TASK_DIR)/%.cpp $(KERNEL_DIR)/ui.h | dirs
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< $(LDFLAGS) -o $@

run: all
	./$(OS_BIN) 2 256 8

check:
	@command -v $(CXX) >/dev/null || { echo "Missing compiler: $(CXX)"; exit 1; }
	@command -v gnome-terminal >/dev/null || \
	 command -v x-terminal-emulator >/dev/null || \
	 command -v xfce4-terminal >/dev/null || \
	 { echo "Missing terminal emulator: install gnome-terminal or x-terminal-emulator"; exit 1; }
	@echo "Ubuntu build prerequisites look available."

install: all
	install -d "$(DESTDIR)$(BINDIR)"
	install -d "$(DESTDIR)$(LIBDIR)/build"
	install -d "$(DESTDIR)$(LIBDIR)/data/virtual_disk"
	install -m 755 "$(OS_BIN)" "$(DESTDIR)$(LIBDIR)/OS"
	install -m 755 $(TASK_BINS) "$(DESTDIR)$(LIBDIR)/build/"
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
	rm -f "$(DATA_DIR)/system_log.txt"
	@echo "Clean complete."

distclean: clean
	rm -rf "$(BUILD_DIR)"
	rm -rf "$(DATA_DIR)/virtual_disk"
	@echo "Distribution clean complete."

help:
	@echo "AMS OS Makefile targets"
	@echo "  make          Build kernel and all task executables"
	@echo "  make run      Build and start with 2 GB RAM, 256 GB HDD, 8 cores"
	@echo "  make check    Verify compiler and terminal emulator availability"
	@echo "  make install  Install under PREFIX=$(PREFIX)"
	@echo "  make clean    Remove binaries and runtime log"
	@echo "  make distclean Remove generated build and virtual disk directories"
	@echo "  make uninstall Remove installed files"
	@echo "  make help     Show this message"
