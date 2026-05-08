# AMS OS Final Project Report

## Project Overview

AMS OS is a terminal-based MiniOS simulator built for the Operating Systems Lab final project. It demonstrates process creation, fork/exec task loading, IPC resource approval, RAM/HDD/CPU resource management, ready queues, multilevel scheduling, synchronization, user mode, kernel mode, interrupt-style process controls, deadlock detection, and system logging.

## OS Name

AMS OS: Atomic Management System

## Target Platform

- Linux Xubuntu
- g++
- make
- xfce4-terminal

## Build And Run

```bash
make
./OS 2 256 8
```

The command above starts AMS OS with:

- RAM: 2 GB
- HDD: 256 GB
- CPU cores: 8

Default Kernel Mode password:

```text
admin
```

## Main Modules

| Module | Files | Purpose |
| --- | --- | --- |
| Kernel control center | `kernel/main.cpp` | Boot, shutdown, menu flow, user/kernel mode, task launching, process control |
| Resource manager | `kernel/resource_manager.*` | RAM, HDD, CPU allocation, release, RAM block layout |
| Process manager | `kernel/process_manager.*` | PCB creation, state transitions, priority, wait time, RAM block tracking |
| Task catalog | `kernel/task_catalog.*` | Metadata for all available task executables |
| Ready queues | `kernel/ready_queue.*` | Multilevel queues and priority aging |
| Scheduler | `kernel/scheduler.*` | FCFS, Round Robin, background queue dispatch, context switching |
| Synchronization | `kernel/sync_manager.*` | Semaphore, condition variable, mutex, resource monitor thread |
| Deadlock manager | `kernel/deadlock_manager.*` | Circular wait simulation and victim selection |
| Logger | `kernel/logger.*` | System, process, and resource logs in `data/system_log.txt` |
| UI helpers | `kernel/ui.h` | Terminal panels, status labels, tables, task control hints |

## Task List

AMS OS includes 16 separate task source files under `tasks/`:

| ID | Task | Type |
| --- | --- | --- |
| 1 | Create File | Interactive |
| 2 | Delete File | Interactive |
| 3 | Copy File | Background |
| 4 | Move File | Background |
| 5 | File Info | Interactive |
| 6 | Notepad | Interactive |
| 7 | Calculator | Interactive |
| 8 | Digital Clock | Auto-running |
| 9 | System Info | Interactive |
| 10 | Snake Game | Interactive |
| 11 | Minesweeper | Interactive |
| 12 | Music Player | Background |
| 13 | Download Simulator | Background |
| 14 | Task Manager | System |
| 15 | Process Killer | Kernel |
| 16 | Calendar | Auto-running |

## Requirement Mapping

| Manual Requirement | Implementation |
| --- | --- |
| Named OS with boot loading screen | `bootScreen()` in `kernel/main.cpp` |
| Hardware resources entered at startup | `./OS <RAM_GB> <HDD_GB> <CPU_CORES>` |
| Minimum 15 tasks | 16 task source files and executables |
| Separate code file per task | All tasks live under `tasks/` |
| Process creation | `fork()` in normal and auto-start task launch paths |
| Exec command usage | `executeTaskExecutable()` uses `execlp()` or `execl()` |
| New terminal per task | Default mode uses `xfce4-terminal --disable-server --execute` |
| Xubuntu process control | Child tasks use process groups so scheduler, close, minimize, and kill signals target the task terminal safely |
| IPC resource message | Child sends `IPCResourceRequest` through pipe |
| RAM/HDD/CPU approval | Parent kernel grants or denies request before task runs |
| RAM location and size | `allocateMemoryBlock()` assigns start/end MB block |
| Ready queue scheduling | `ReadyQueueManager` and `Scheduler` |
| Multilevel queues | System, interactive, and background queues |
| FCFS and Round Robin | System queue uses FCFS, interactive queue uses quantum-based Round Robin |
| Interrupt/minimize/resume | Menu options 17 and 18 move processes through BLOCKED/READY states |
| Close task and free RAM | Menu option 21 closes process and releases resources |
| Kernel mode process control | Menu option 13 unlocks Kernel Mode, option 16 kills processes |
| User mode and Kernel mode | `OSMode` in `kernel/main.cpp` |
| Threads | Resource monitor thread in `SyncManager` |
| Mutex, condition variable, semaphore | `sync_manager.*` |
| Process priority with aging | `ReadyQueueManager::applyAging()` |
| Deadlock detection | Menu option 19 runs circular wait simulation |
| System log file | `data/system_log.txt` |
| Auto-start time/calendar | Digital Clock and Calendar auto-start and are dispatched immediately after boot |
| Virtual hard disk storage | File tasks use `data/virtual_disk/` |

## Demo Sequence

1. Build with `make`.
2. Run `./OS 2 256 8`.
3. Show the boot screen and startup resource summary.
4. Open menu option 24, Instruction Guide.
5. Open menu option 1, Task Catalog.
6. Launch Calculator or Notepad through menu option 3.
7. Run menu option 11, Scheduler.
8. Show menu options 7, 12, 4, and 20 for PCB, ready queues, resources, and RAM layout.
9. Minimize a process with option 17, then resume it with option 18.
10. Switch to Kernel Mode using option 13 and password `admin`.
11. View logs with option 15.
12. Run deadlock detection with option 19.
13. Close a task with option 21 or Kernel Kill with option 16.
14. Shutdown with option 0.

## Suggested Screenshots For Final Word Report

- Boot screen showing AMS OS name.
- Startup hardware resource summary.
- Instruction guide and task catalog.
- Launch Task IPC request and grant.
- PCB table with PID, state, type, priority, wait time, and RAM block.
- Ready queues showing multilevel scheduling.
- Resource status and RAM memory layout.
- Scheduler dispatch/context switch output.
- Minimize and resume process output.
- Kernel Mode authentication.
- System log output.
- Deadlock detection message.
- Shutdown cleanup screen.

## Submission Notes

- Submit the full project folder as a zip file.
- Include the source code, `makefile`, `README.txt`, this report source, and task files.
- Record the demo on Linux Xubuntu so separate task terminals are visible.
- Capture screenshots after building and running the project on the target platform.
