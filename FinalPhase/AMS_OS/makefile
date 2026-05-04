CXX = g++
CXXFLAGS = -std=c++17 -Wall

KERNEL_SRC = kernel/main.cpp \
             kernel/resource_manager.cpp \
             kernel/process_manager.cpp \
             kernel/task_catalog.cpp \
             kernel/ready_queue.cpp \
             kernel/scheduler.cpp

OS_OUTPUT = OS

TASKS = build/calculator \
        build/notepad \
        build/clock \
        build/music_player \
        build/file_copy

all: folders $(OS_OUTPUT) $(TASKS)

folders:
	mkdir -p build
	mkdir -p data
	mkdir -p data/virtual_disk

$(OS_OUTPUT): $(KERNEL_SRC)
	$(CXX) $(CXXFLAGS) $(KERNEL_SRC) -o $(OS_OUTPUT)

build/calculator: tasks/calculator.cpp
	$(CXX) $(CXXFLAGS) tasks/calculator.cpp -o build/calculator

build/notepad: tasks/notepad.cpp
	$(CXX) $(CXXFLAGS) tasks/notepad.cpp -o build/notepad

build/clock: tasks/clock.cpp
	$(CXX) $(CXXFLAGS) tasks/clock.cpp -o build/clock

build/music_player: tasks/music_player.cpp
	$(CXX) $(CXXFLAGS) tasks/music_player.cpp -o build/music_player

build/file_copy: tasks/file_copy.cpp
	$(CXX) $(CXXFLAGS) tasks/file_copy.cpp -o build/file_copy

run:
	./OS 2 256 8

clean:
	rm -f OS
	rm -f build/calculator
	rm -f build/notepad
	rm -f build/clock
	rm -f build/music_player
	rm -f build/file_copy