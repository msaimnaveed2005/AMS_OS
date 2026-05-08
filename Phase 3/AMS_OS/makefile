CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread
UI_HEADER = kernel/ui.h

KERNEL_SRC = kernel/main.cpp \
             kernel/resource_manager.cpp \
             kernel/process_manager.cpp \
             kernel/task_catalog.cpp \
             kernel/ready_queue.cpp \
             kernel/scheduler.cpp \
             kernel/logger.cpp \
             kernel/deadlock_manager.cpp \
             kernel/sync_manager.cpp

OS_OUTPUT = OS

TASKS = build/calculator \
        build/calendar \
        build/notepad \
        build/clock \
        build/music_player \
        build/file_copy \
        build/create_file \
        build/delete_file \
        build/download_simulator \
        build/file_info \
        build/minesweeper \
        build/move_file \
        build/process_killer \
        build/snake \
        build/system_info \
        build/task_manager

.PHONY: all folders run clean

all: folders $(OS_OUTPUT) $(TASKS)

folders:
	mkdir -p build
	mkdir -p data
	mkdir -p data/virtual_disk

$(OS_OUTPUT): $(KERNEL_SRC) $(UI_HEADER)
	$(CXX) $(CXXFLAGS) $(KERNEL_SRC) -o $(OS_OUTPUT)

build/%: tasks/%.cpp $(UI_HEADER) | folders
	$(CXX) $(CXXFLAGS) $< -o $@

run:
	./OS 2 256 8

clean:
	rm -f OS
	rm -f $(TASKS)
