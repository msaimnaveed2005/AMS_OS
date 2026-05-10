# AMS OS - Atomic Management System

AMS OS is a terminal-based Mini Operating System Simulator for the Operating
Systems Lab Spring 2026 final project. It demonstrates process creation,
resource allocation, multilevel scheduling, synchronization, interrupts,
deadlock detection, and user/kernel mode separation on Linux.

The project is intentionally Ubuntu-focused. It uses POSIX APIs such as
`fork()`, `exec()`, pipes, process signals, and `waitpid()`.

## Features

| Area | Implementation |
| --- | --- |
| Boot workflow | Named AMS OS boot screen with a `sleep()` loading sequence |
| Startup resources | RAM, hard drive, and CPU cores supplied before boot |
| Task catalog | 16 independent task executables under `tasks/` |
| Process creation | `fork()` plus `exec()` for each task |
| IPC | Child task sends a resource request to the kernel through pipes |
| Resource control | RAM, HDD, CPU counters plus first-fit RAM block layout |
| Scheduling | Multilevel ready queues with FCFS and Round Robin behavior |
| Priority aging | Waiting processes are promoted to prevent starvation |
| Synchronization | Semaphore, mutex, condition variable, and monitor thread |
| Interrupt handling | Minimize, resume, block, unblock, close, and kernel kill |
| User/kernel mode | Password-protected Kernel Mode for privileged operations |
| Deadlock detection | Circular-wait simulation with victim recovery |
| Runtime log | `data/system_log.txt` records process and resource events |
| Virtual disk | File tasks read/write under `data/virtual_disk/` |

## Requirements

- Ubuntu 22.04 or newer recommended
- `g++` with C++17 support
- GNU Make
- GNOME Terminal, `x-terminal-emulator`, or `xfce4-terminal`

Install the standard dependencies on Ubuntu:

```bash
sudo apt update
sudo apt install build-essential gnome-terminal
```

If you are using a lightweight Ubuntu flavor, `xfce4-terminal` is also
supported:

```bash
sudo apt install xfce4-terminal
```

## Build

```bash
make check
make
```

`make check` verifies that the compiler and at least one supported terminal
emulator are available. `make` builds the kernel binary and all 16 task
executables.

## Run

Interactive startup:

```bash
./OS
```

Quick start with the project-recommended hardware instance:

```bash
./OS 2 256 8
```

This means:

- 2 GB RAM
- 256 GB hard drive
- 8 CPU cores

You can also run:

```bash
make run
```

## Install

```bash
sudo make install
ams-os 2 256 8
```

The install target places the simulator under `/usr/local/lib/ams-os` and
creates `/usr/local/bin/ams-os` as a launcher script. Uninstall with:

```bash
sudo make uninstall
```

## Kernel Mode

Use menu option `13` to switch from User Mode to Kernel Mode.

Default password:

```text
admin
```

Kernel Mode unlocks system logs, resource tests, process kill, PCB removal,
deadlock detection, and other privileged controls.

## Main Demo Workflow

1. Build the project with `make`.
2. Start AMS OS with `./OS 2 256 8`.
3. Review the boot screen and startup hardware summary.
4. Open the Instruction Guide with menu option `24`.
5. View the Task Catalog with option `1`.
6. Launch a task with option `3`.
7. Dispatch ready tasks with option `11`.
8. Inspect PCB, queues, resources, and RAM layout with options `7`, `12`, `4`,
   and `20`.
9. Minimize and resume a task with options `17` and `18`.
10. Simulate a blocked interrupt with options `27` and `28`.
11. Switch to Kernel Mode with option `13`.
12. View logs with option `15`.
13. Run deadlock detection with option `19`.
14. Close or kill tasks with options `21` or `16`.
15. Shut down cleanly with option `0`.

## Task Catalog

| ID | Task | Type |
| --- | --- | --- |
| 1 | Create File | Interactive |
| 2 | Delete File | Interactive |
| 3 | Copy File | Background |
| 4 | Move File | Background |
| 5 | File Info | Interactive |
| 6 | Notepad | Interactive, auto-save |
| 7 | Calculator | Interactive |
| 8 | Digital Clock | Auto-start |
| 9 | System Info | Interactive |
| 10 | Snake Game | Interactive |
| 11 | Minesweeper | Interactive |
| 12 | Music Player | Background |
| 13 | Download Simulator | Background |
| 14 | Task Manager | System |
| 15 | Process Killer | Kernel |
| 16 | Calendar | Auto-start |

## Project Structure

```text
AMS_OS/
|-- kernel/
|   |-- main.cpp                # Boot, menu, IPC, process controls
|   |-- resource_manager.*      # RAM, HDD, CPU, memory blocks
|   |-- process_manager.*       # PCB table and lifecycle states
|   |-- ready_queue.*           # Multilevel ready queues and aging
|   |-- scheduler.*             # FCFS, Round Robin, context switching
|   |-- sync_manager.*          # Semaphore, mutex, condition variable, thread
|   |-- deadlock_manager.*      # Circular-wait detection
|   |-- logger.*                # Runtime system log
|   |-- task_catalog.*          # Task metadata
|   `-- ui.h                    # Terminal UI helpers
|-- tasks/                      # One source file per task executable
|-- build/                      # Generated task binaries
|-- data/
|   |-- system_log.txt          # Generated runtime log
|   `-- virtual_disk/           # Simulated hard drive
|-- Makefile
|-- README.md
`-- PROJECT_REPORT.md
```

## Make Targets

| Target | Purpose |
| --- | --- |
| `make` | Build the kernel and task executables |
| `make run` | Build and run with 2 GB RAM, 256 GB HDD, 8 cores |
| `make check` | Verify Ubuntu build prerequisites |
| `make install` | Install AMS OS under `PREFIX` |
| `make uninstall` | Remove installed files |
| `make clean` | Remove binaries and runtime log |
| `make distclean` | Remove generated build and virtual disk directories |
| `make help` | Print Makefile help |

## Notes

- The simulator is designed for Ubuntu/Linux process APIs.
- Task windows are launched with `gnome-terminal` first, then common Ubuntu
  fallbacks.
- Runtime files are generated under `data/`; they can be removed with
  `make clean` or `make distclean`.
- See `PROJECT_REPORT.md` for the full architecture, scheduling design, and
  manual compliance matrix.
