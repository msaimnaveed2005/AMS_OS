# AMS OS — Atomic Management System

AMS OS is a terminal-based **Mini Operating System Simulator** built for the
Operating Systems Lab Spring 2026 final project. It demonstrates every core OS
concept — process creation, resource allocation, multilevel scheduling,
synchronization, interrupts, deadlock detection, and user/kernel mode
separation — using real POSIX APIs on Ubuntu Linux.

> **Platform:** Ubuntu 22.04+ (POSIX fork/exec, pipes, signals, pthreads)

---

## Features

| Area | Implementation |
|---|---|
| **Boot workflow** | Named boot screen with animated loading bar and sound cues |
| **Hardware resources** | RAM (GB), HDD (GB), CPU cores supplied before boot |
| **Task catalog** | 16 independent task executables under `tasks/` |
| **Process creation** | `fork()` + `exec()` for each task; no simple function calling |
| **IPC** | Child sends resource request to kernel through pipe; kernel grants/denies |
| **Resource control** | RAM, HDD, CPU counters + first-fit RAM block memory layout |
| **Separate terminals** | Each task opens in its own GNOME Terminal window |
| **Multitasking** | Multiple tasks run in parallel; scheduler manages time-sharing |
| **Scheduling** | Multilevel ready queue: System (FCFS), Interactive (Round Robin), Background (FCFS) |
| **Priority aging** | Waiting processes are promoted after threshold to prevent starvation |
| **Context switching** | SIGSTOP/SIGCONT-based preemption with time quantum tracking |
| **Synchronization** | Semaphore, mutex, condition variable, and background monitor thread |
| **Interrupt handling** | Minimize, resume, block, unblock, close, and kernel kill |
| **Terminal minimize detection** | `xdotool` detects when task terminal is minimized/restored |
| **User/Kernel mode** | Password-protected Kernel Mode for privileged operations |
| **Deadlock detection** | Circular-wait simulation with victim selection and recovery |
| **System log** | `data/system_log.txt` records process creation, resource allocation, termination |
| **Virtual disk** | File tasks read/write under `data/virtual_disk/` (simulated HDD) |
| **Graceful shutdown** | Terminates all children, releases resources, clears queues |

---

## Requirements

- **Ubuntu 22.04** or newer (any Linux with GNOME Terminal works)
- `g++` with C++17 support
- GNU Make
- GNOME Terminal (or `xfce4-terminal` / `x-terminal-emulator`)
- `xdotool` (for terminal minimize/restore detection)

```bash
sudo apt update
sudo apt install build-essential gnome-terminal xdotool
```

For lightweight Ubuntu flavors:

```bash
sudo apt install xfce4-terminal xdotool
```

---

## Build

```bash
make check    # Verify compiler + terminal emulator
make          # Build kernel + all 16 task executables
```

## Run

**Interactive startup** (prompts for RAM, HDD, cores):

```bash
./OS
```

**Quick start** with the project-recommended instance:

```bash
./OS 2 256 8    # 2 GB RAM, 256 GB HDD, 8 CPU cores
```

Or simply:

```bash
make run
```

---

## Install / Uninstall

```bash
sudo make install       # Installs to /usr/local
ams-os 2 256 8          # Run from anywhere
sudo make uninstall     # Remove
```

---

## Kernel Mode

Switch from User Mode to Kernel Mode using menu option **13**.

**Default password:** `admin`

Kernel Mode unlocks:
- System log viewer
- Process kill (force terminate any PID)
- Deadlock detection simulation
- Full resource diagnostics

---

## Demo Workflow

1. `make && ./OS 2 256 8` — Build and boot the OS
2. Clock and Calendar auto-start after boot
3. **Menu 1** — Browse Task Catalog
4. **Menu 3** — Launch a task (fork + IPC resource request)
5. **Menu 11** — Run Scheduler (dispatches ready tasks)
6. **Menu 28** — Start Multitasking (parallel execution)
7. **Menu 17/18** — Minimize / Resume a process
8. **Menu 26/27** — Simulate interrupt (block/unblock)
9. **Menu 22** — Switch to a running task
10. **Menu 7/12/4/20** — Inspect PCB, queues, resources, RAM layout
11. **Menu 13** → password `admin` → Kernel Mode
12. **Menu 15** — View system log
13. **Menu 19** — Run deadlock detection
14. **Menu 16/21** — Kill or close a process
15. **Menu 0** — Shutdown AMS OS

---

## Task Catalog

| ID | Task | Type | Description |
|---|---|---|---|
| 1 | Create File | Interactive | Creates a file in virtual disk |
| 2 | Delete File | Interactive | Deletes a file from virtual disk |
| 3 | Copy File | Background | Copies file with progress feedback |
| 4 | Move File | Background | Moves/renames file in virtual disk |
| 5 | File Info | Interactive | Displays file size, permissions, lines |
| 6 | Notepad | Interactive | Auto-save editor with commands |
| 7 | Calculator | Interactive | Basic arithmetic (add/sub/mul/div) |
| 8 | Digital Clock | Auto-start | Live clock with 60-second demo |
| 9 | System Info | Interactive | Process, host, and environment details |
| 10 | Snake Game | Interactive | Text-based snake with WASD controls |
| 11 | Minesweeper | Interactive | 5×5 grid with 4 hidden mines |
| 12 | Music Player | Background | Track selection with visual progress |
| 13 | Download Simulator | Background | Animated download with speed display |
| 14 | Task Manager | System | Virtual disk browser + process sample |
| 15 | Process Killer | Kernel | Signal sender for TERM/STOP/CONT |
| 16 | Calendar | Auto-start | Interactive calendar with date browser |

---

## OS Concepts Covered

| Concept | Where |
|---|---|
| **Process creation** | `fork()` in `main.cpp` → child `exec()` task binary |
| **IPC** | Pipes for resource request/response between parent and child |
| **EXEC commands** | `execlp()` / `execl()` to load task executables |
| **Multitasking** | Parallel task dispatch, terminal window state monitoring |
| **Context switching** | `SIGSTOP` / `SIGCONT` signals, round-robin preemption |
| **Resource allocation** | RAM, HDD, CPU tracking; first-fit memory block allocator |
| **User / Kernel mode** | Password-gated access to privileged operations |
| **Threads** | `std::thread` resource monitor runs in background |
| **Semaphore** | `SimpleSemaphore` class in `sync_manager.*` |
| **Mutex** | `std::mutex` for CPU core pool and ready queue |
| **Condition variable** | `std::condition_variable` for ready queue notification |
| **Multilevel queue** | System (FCFS), Interactive (Round Robin), Background (FCFS) |
| **Priority aging** | Waiting time threshold promotes lower-priority processes |
| **Deadlock detection** | Circular wait simulation with victim recovery |
| **Interrupt handling** | Block/unblock, minimize/resume with queue management |
| **System log** | Logger records creation, allocation, termination events |

---

## Project Structure

```
AMS_OS/
├── kernel/
│   ├── main.cpp                 # Boot, menu, IPC, process controls
│   ├── resource_manager.*       # RAM, HDD, CPU, first-fit memory blocks
│   ├── process_manager.*        # PCB table and lifecycle state machine
│   ├── ready_queue.*            # Multilevel ready queues with aging
│   ├── scheduler.*              # FCFS, Round Robin, context switching
│   ├── sync_manager.*           # Semaphore, mutex, condition variable, thread
│   ├── deadlock_manager.*       # Circular-wait detection and recovery
│   ├── logger.*                 # Runtime system log writer
│   ├── task_catalog.*           # Task metadata registry
│   └── ui.h                    # Terminal UI framework (ANSI, panels, bars)
├── tasks/                       # 16 independent task source files
│   ├── calculator.cpp
│   ├── calendar.cpp
│   ├── clock.cpp
│   ├── create_file.cpp
│   ├── delete_file.cpp
│   ├── download_simulator.cpp
│   ├── file_copy.cpp
│   ├── file_info.cpp
│   ├── minesweeper.cpp
│   ├── move_file.cpp
│   ├── music_player.cpp
│   ├── notepad.cpp
│   ├── process_killer.cpp
│   ├── snake.cpp
│   ├── system_info.cpp
│   └── task_manager.cpp
├── build/                       # Compiled task binaries (generated)
├── data/
│   ├── sounds/                  # Optional .wav sound cue files
│   ├── system_log.txt           # Runtime log (generated)
│   └── virtual_disk/            # Simulated hard drive (generated)
├── makefile
├── README.md
├── README.txt
└── PROJECT_REPORT.md
```

---

## Make Targets

| Target | Purpose |
|---|---|
| `make` | Build kernel binary and all 16 task executables |
| `make run` | Build and run with 2 GB RAM, 256 GB HDD, 8 cores |
| `make check` | Verify Ubuntu build prerequisites |
| `make install` | Install AMS OS system-wide under `PREFIX` |
| `make uninstall` | Remove installed files |
| `make clean` | Remove binaries and runtime log |
| `make distclean` | Remove build directory and virtual disk |
| `make help` | Print available targets |

---

## Sound Cues (Optional)

Place `.wav` files in `data/sounds/` for audio feedback:

| File | Trigger |
|---|---|
| `boot.wav` | OS boot complete |
| `shutdown.wav` | OS shutdown |
| `error.wav` | Error events |
| `granted.wav` | Resource request granted |
| `denied.wav` | Resource request denied |
| `minimize.wav` | Process minimized |
| `resume.wav` | Process resumed |
| `close.wav` | Process closed |

If sound files are missing, the OS falls back to terminal bell (`\a`).

---

## Notes

- The simulator is designed exclusively for **Ubuntu/Linux** process APIs.
- Task windows launch via `gnome-terminal` with fallbacks to `xfce4-terminal`
  and `x-terminal-emulator`.
- Terminal minimize/restore detection requires `xdotool`.
- When a task's terminal is minimized via the window button, the scheduler
  automatically pauses it. When restored, the task auto-resumes and continues
  executing.
- Runtime files are generated under `data/`; remove with `make clean` or
  `make distclean`.
- See `PROJECT_REPORT.md` for the full architecture documentation and manual
  compliance matrix.
