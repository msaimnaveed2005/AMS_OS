# AMS OS - Atomic Management System

## Final Project Report

- **Course:** Operating Systems Lab, Spring 2026
- **Project:** Mini Operating System Simulator
- **Project Phase:** Phase 3 / Final Submission Codebase
- **Operating System Name:** AMS OS (Atomic Management System)
- **Target Platform:** Ubuntu Linux
- **Primary Language:** C++17
- **Build Tool:** GNU Make

## 1. Executive Summary

AMS OS is a Linux terminal application that simulates the responsibilities of a
small operating system. It accepts hardware resources at startup, boots with a
named loading screen, creates task processes with `fork()` and `exec()`, grants
or denies resources through IPC, tracks processes in a PCB table, schedules
ready tasks with multilevel queues, and exposes both User Mode and Kernel Mode
controls.

The implementation is organized into kernel subsystems and independent task
executables. Each task has its own source file in `tasks/` and is launched as a
separate process. Runtime state is recorded in a system log, and file-oriented
tasks use a virtual disk directory under `data/virtual_disk/`.

## 2. Project Scope

AMS OS demonstrates the following operating system concepts:

- Multitasking through multiple child processes and ready queues
- Process creation using `fork()`
- Program replacement using `exec()`
- Inter-process communication using pipes
- Resource allocation for RAM, hard drive, and CPU cores
- Process lifecycle management through PCB state transitions
- Context switching with scheduler dispatch and POSIX signals
- Multilevel queue scheduling with FCFS and Round Robin behavior
- Priority aging to prevent starvation
- Synchronization with semaphore, mutex, condition variable, and thread
- User Mode and Kernel Mode separation
- Interrupt simulation through minimize, block, resume, and close operations
- Deadlock detection through a circular wait graph
- Runtime logging for process and resource events

## 3. System Requirements

### 3.1 Target Environment

- Ubuntu 22.04 or newer recommended
- GNU Make
- `g++` with C++17 support
- GNOME Terminal, `x-terminal-emulator`, or `xfce4-terminal`

### 3.2 Installation Commands

```bash
sudo apt update
sudo apt install build-essential gnome-terminal
```

Optional lightweight terminal fallback:

```bash
sudo apt install xfce4-terminal
```

### 3.3 Build and Run

```bash
make check
make
./OS 2 256 8
```

The command above starts AMS OS with the project-recommended instance:

- RAM: 2 GB
- Hard drive: 256 GB
- CPU cores: 8

## 4. System Architecture

```text
                         User Terminal
                              |
                              v
+---------------------------------------------------------------+
|                         AMS OS Kernel                         |
|                         kernel/main.cpp                       |
|                                                               |
|  +----------------+   +----------------+   +----------------+ |
|  | Task Catalog   |   | Process Mgr    |   | Resource Mgr   | |
|  | 16 executables |   | PCB table      |   | RAM/HDD/CPU    | |
|  +----------------+   +----------------+   +----------------+ |
|          |                    |                    |           |
|          v                    v                    v           |
|  +----------------+   +----------------+   +----------------+ |
|  | Ready Queues   |   | Scheduler      |   | Sync Manager   | |
|  | MLQ + aging    |   | FCFS/RR        |   | sem/mutex/cv   | |
|  +----------------+   +----------------+   +----------------+ |
|          |                    |                    |           |
|          v                    v                    v           |
|  +----------------+   +----------------+   +----------------+ |
|  | Deadlock Mgr   |   | Logger         |   | Terminal UI    | |
|  | wait graph     |   | system_log.txt |   | GNOME style    | |
|  +----------------+   +----------------+   +----------------+ |
+---------------------------------------------------------------+
                              |
                   fork(), pipes, SIGSTOP/SIGCONT
                              |
                              v
+---------------------------------------------------------------+
|                 Child Task Processes via exec()               |
|  Calculator | Notepad | Clock | Calendar | File Tasks | Games |
+---------------------------------------------------------------+
```

## 5. Source Organization

```text
kernel/
|-- main.cpp                Boot, menus, IPC, process controls
|-- resource_manager.*      RAM, HDD, CPU counters and memory layout
|-- process_manager.*       PCB table and lifecycle operations
|-- task_catalog.*          Metadata for all task executables
|-- ready_queue.*           Multilevel ready queue implementation
|-- scheduler.*             FCFS/RR scheduler and context switching
|-- sync_manager.*          Semaphore, mutex, condition variable, thread
|-- deadlock_manager.*      Circular wait detection
|-- logger.*                System, process, and resource logging
`-- ui.h                    Terminal UI helpers

tasks/
|-- calculator.cpp
|-- calendar.cpp
|-- clock.cpp
|-- create_file.cpp
|-- delete_file.cpp
|-- download_simulator.cpp
|-- file_copy.cpp
|-- file_info.cpp
|-- minesweeper.cpp
|-- move_file.cpp
|-- music_player.cpp
|-- notepad.cpp
|-- process_killer.cpp
|-- snake.cpp
|-- system_info.cpp
`-- task_manager.cpp
```

## 6. Kernel Module Design

### 6.1 Main Kernel Controller

`kernel/main.cpp` controls startup, boot display, hardware resource input,
menu routing, task launch, IPC, process control, mode switching, and shutdown.
It creates required runtime directories with C++17 filesystem APIs instead of
shell-specific commands.

Task launch workflow:

1. User selects a task from the catalog.
2. Kernel creates request and response pipes.
3. Kernel forks a child process.
4. Child sends `IPCResourceRequest` to the kernel.
5. Kernel checks RAM, HDD, and CPU availability.
6. Kernel grants or denies the request through the response pipe.
7. If granted, the kernel allocates a simulated RAM block and creates a PCB.
8. The child waits with `SIGSTOP`.
9. Scheduler resumes the child with `SIGCONT`.
10. Child replaces itself with the task executable using `exec()`.

### 6.2 Resource Manager

The Resource Manager tracks:

- Total and available RAM in MB
- Total and available hard drive space in MB
- Total and available CPU cores
- Simulated RAM layout using first-fit block allocation

Memory allocation uses contiguous blocks with `startAddress` and `endAddress`.
When a process terminates, the manager releases its block and merges adjacent
free blocks to reduce fragmentation.

### 6.3 Process Manager

The Process Manager stores each process in a PCB record:

- PID
- Process name
- Process type
- Lifecycle state
- Priority
- RAM, HDD, and CPU requirements
- Waiting time
- Turnaround time
- Assigned CPU core
- Ready queue label
- RAM block start and end addresses

Supported lifecycle states:

```text
NEW -> READY -> RUNNING -> TERMINATED
              |      |
              |      +-> BLOCKED
              |      +-> MINIMIZED
              +<----- resume/unblock
```

### 6.4 Task Catalog

The Task Catalog stores static metadata for each task, including ID, name,
type, priority, RAM requirement, HDD requirement, CPU requirement, executable
path, and description. Registration is centralized through a single helper
method to enforce consistent metadata layout and reduce duplication when
maintaining the 16-task catalog.

### 6.5 Ready Queue Manager

AMS OS implements a multilevel ready queue:

| Queue | Priority | Scheduling Policy | Purpose |
| --- | --- | --- | --- |
| System Queue | 1 or lower | FCFS | System and kernel tasks |
| Interactive Queue | 2 | Round Robin | Calculator, Notepad, games, auto tasks |
| Background Queue | 3 or higher | FCFS | Copy, move, music, download tasks |

Priority aging is applied before each scheduler selection. If a process waits
too long, its numeric priority improves and it may move to a higher queue.

### 6.6 Scheduler

The scheduler selects the next process from System, Interactive, then
Background queues. It demonstrates context switching by:

- Acquiring CPU core slots through the Sync Manager
- Assigning a core ID to the PCB
- Moving the selected process to RUNNING
- Sending `SIGCONT`
- Waiting for completion or time quantum expiry
- Sending `SIGSTOP` when an interactive process exhausts its quantum
- Re-queueing unfinished tasks
- Releasing resources when a task terminates

### 6.7 Sync Manager

Synchronization concepts are implemented as follows:

| Primitive | Implementation |
| --- | --- |
| Semaphore | `SimpleSemaphore` controls CPU core slots |
| Mutex | Protects the CPU core ID pool |
| Condition variable | Notifies scheduler when a ready process exists |
| Thread | Resource monitor logs available resources periodically |
| Atomic flag | Stops the monitor thread safely |

### 6.8 Deadlock Manager

The deadlock module simulates circular wait between two selected processes. It
creates a wait-for graph, detects cycles with DFS, displays the required
message `Deadlock detected among processes`, selects a victim, and terminates
that victim to recover resources.

### 6.9 Logger

`Logger` appends timestamped events to `data/system_log.txt`. It records:

- Process creation
- Resource allocation
- RAM block assignment
- Scheduler dispatch
- Interrupts and state changes
- Process termination
- Kernel mode access
- Deadlock detection
- Shutdown cleanup

## 7. Task Inventory

| ID | Task | Source File | Type | Priority | RAM | HDD | CPU |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Create File | `tasks/create_file.cpp` | Interactive | 2 | 80 MB | 20 MB | 1 |
| 2 | Delete File | `tasks/delete_file.cpp` | Interactive | 2 | 70 MB | 10 MB | 1 |
| 3 | Copy File | `tasks/file_copy.cpp` | Background | 3 | 200 MB | 100 MB | 1 |
| 4 | Move File | `tasks/move_file.cpp` | Background | 3 | 150 MB | 60 MB | 1 |
| 5 | File Info | `tasks/file_info.cpp` | Interactive | 2 | 90 MB | 20 MB | 1 |
| 6 | Notepad | `tasks/notepad.cpp` | Interactive | 2 | 150 MB | 50 MB | 1 |
| 7 | Calculator | `tasks/calculator.cpp` | Interactive | 2 | 100 MB | 20 MB | 1 |
| 8 | Digital Clock | `tasks/clock.cpp` | Auto-running | 2 | 50 MB | 10 MB | 1 |
| 9 | System Info | `tasks/system_info.cpp` | Interactive | 2 | 80 MB | 10 MB | 1 |
| 10 | Snake Game | `tasks/snake.cpp` | Interactive | 2 | 250 MB | 40 MB | 1 |
| 11 | Minesweeper | `tasks/minesweeper.cpp` | Interactive | 2 | 220 MB | 40 MB | 1 |
| 12 | Music Player | `tasks/music_player.cpp` | Background | 3 | 120 MB | 30 MB | 1 |
| 13 | Download Simulator | `tasks/download_simulator.cpp` | Background | 3 | 180 MB | 200 MB | 1 |
| 14 | Task Manager | `tasks/task_manager.cpp` | System | 1 | 100 MB | 20 MB | 1 |
| 15 | Process Killer | `tasks/process_killer.cpp` | Kernel | 1 | 100 MB | 20 MB | 1 |
| 16 | Calendar | `tasks/calendar.cpp` | Auto-running | 2 | 50 MB | 10 MB | 1 |

## 8. Ready Queue Design

```text
Priority 1: System Queue      [FCFS]
Priority 2: Interactive Queue [Round Robin, 8 second quantum]
Priority 3: Background Queue  [FCFS]

Before each dispatch:
    increment waiting time
    if waiting time >= aging threshold:
        improve priority
        move process to the appropriate queue
```

This design satisfies the manual requirement for multilevel queues with
different algorithms at different levels.

## 9. Resource Allocation Strategy

Resources are checked before a child process is allowed to run.

```text
Child Process -> IPC request -> Kernel
Kernel checks RAM, HDD, CPU
    if available:
        allocate counters
        allocate RAM block
        create PCB
        add to ready queue
        grant response
    else:
        deny response
        terminate child
```

The number of active tasks is therefore limited by available RAM, hard drive
space, and CPU core availability.

## 10. User Interface Refactor

The UI was standardized around an Ubuntu terminal aesthetic:

- Royal blue is used as the primary accent.
- Neutral gray and white text are used for the base interface.
- Green, yellow, and red are reserved for success, warning, and error states.
- Fixed-width tables use plain state text to preserve alignment.
- The terminal launcher now prefers GNOME Terminal on Ubuntu and falls back to
  `x-terminal-emulator` or `xfce4-terminal`.

## 11. User Mode and Kernel Mode

AMS OS starts in User Mode. User Mode can view normal system state and launch
tasks. Kernel Mode is protected by password authentication and unlocks
privileged actions:

- View the full system log
- Kill a process
- Remove PCB entries
- Run resource allocation tests
- Run deadlock detection
- Manually update process state

Default Kernel Mode password:

```text
admin
```

## 12. Interrupt and Process Control

AMS OS supports manual process control:

| Control | Menu Option | Effect |
| --- | --- | --- |
| Minimize | 17 | Sends stop signal, removes from ready queue, keeps resources allocated |
| Resume | 18 | Moves MINIMIZED or BLOCKED process back to READY |
| Close | 21 | Terminates process and releases all resources |
| Switch | 22 | Shows and resumes a selected process state when applicable |
| Input interrupt | 27 | Moves task to BLOCKED state |
| Complete interrupt | 28 | Moves BLOCKED task back to READY |
| Kernel kill | 16 | Force terminates a task in Kernel Mode |

## 13. Manual Compliance Matrix

| Manual Requirement | Status | Implementation |
| --- | --- | --- |
| OS has its own name | Complete | AMS OS displayed on boot |
| Boot loading with sleep | Complete | `bootScreen()` uses `sleep(1)` |
| User provides RAM, HDD, cores | Complete | CLI args or interactive startup prompts |
| Resource-managed processes | Complete | Resource Manager checks RAM/HDD/CPU before grant |
| Minimum 15 tasks | Complete | 16 task executables |
| Separate code file per task | Complete | One `.cpp` file per task under `tasks/` |
| Process creation, not simple calls | Complete | `fork()` and `exec()` are used |
| IPC resource message | Complete | Child sends `IPCResourceRequest` through pipe |
| Deny if resources unavailable | Complete | Parent denies and child exits |
| Separate terminal per task | Complete | GNOME Terminal / Ubuntu fallback terminal launcher |
| Close or minimize task | Complete | Menu options 17, 18, 21, 22 |
| Multitasking | Complete | PCB table, ready queues, scheduler, separate processes |
| User Mode and Kernel Mode | Complete | `OSMode` with password-protected Kernel Mode |
| Hardware access in Kernel Mode | Complete | Kernel Mode process kill/resource tools |
| Time and calendar auto-start | Complete | Clock and Calendar auto-start after boot |
| Instruction guide | Complete | Menu option 24 |
| RAM location and size | Complete | Memory block start/end stored in PCB |
| Ready queue scheduling | Complete | `ReadyQueueManager` and `Scheduler` |
| Multilevel queue scheduling | Complete | System, Interactive, Background queues |
| Different algorithms per queue | Complete | FCFS and Round Robin |
| Interrupt moves task to blocked | Complete | Menu option 27 |
| Resume after interrupt | Complete | Menu option 28 |
| Process lifecycle removal | Complete | Scheduler and close/kill release resources |
| Data saved to hard disk | Complete | Virtual disk under `data/virtual_disk/` |
| Threads | Complete | Resource monitor thread |
| Semaphore | Complete | `SimpleSemaphore` |
| Mutex | Complete | CPU core pool mutex |
| Condition variable | Complete | Ready queue notification |
| Priority with aging | Complete | `ReadyQueueManager::applyAging()` |
| Deadlock detection | Complete | Wait-for graph cycle detection |
| Required deadlock message | Complete | Displays `Deadlock detected among processes` |
| System log file | Complete | `data/system_log.txt` |
| Graphics library bonus | Not included | Optional bonus, not part of mandatory scope |

## 14. Build Configuration

The Makefile supports standard Linux build practices:

| Target | Purpose |
| --- | --- |
| `make` | Build kernel and all task executables |
| `make run` | Build and run with recommended resources |
| `make check` | Verify compiler and terminal emulator |
| `make install` | Run prerequisite checks, then install under `/usr/local` |
| `make uninstall` | Remove installed files |
| `make clean` | Remove generated binaries and runtime log |
| `make distclean` | Remove generated build and virtual disk directories |
| `make help` | Show usage |

The install target creates an `ams-os` launcher that changes into the installed
application directory before running the kernel, so relative task paths continue
to work after installation.

## 15. Testing Plan

Recommended test sequence:

1. Run `make check`.
2. Run `make`.
3. Start `./OS 2 256 8`.
4. Confirm the boot name and loading animation.
5. Confirm Clock and Calendar auto-start.
6. Launch Calculator, Notepad, Music Player, and File Copy.
7. Run the scheduler and observe context switch output.
8. Open PCB table and verify PID, state, priority, queue, and RAM block.
9. Open Ready Queues and verify multilevel assignment.
10. Minimize and resume an interactive task.
11. Trigger input interrupt and complete interrupt.
12. Enter Kernel Mode with `admin`.
13. View `data/system_log.txt` through menu option 15.
14. Run deadlock detection with two active processes.
15. Close tasks and verify resources are released.
16. Shutdown and confirm cleanup.

## 16. Screenshot and Demonstration Guide

The final academic submission should include screenshots or video frames for:

1. Hardware resource input screen.
2. AMS OS boot screen with loading animation.
3. Main dashboard in User Mode.
4. Instruction Guide.
5. Task Catalog.
6. IPC resource request and grant.
7. Separate task terminal.
8. PCB table.
9. Ready queue display.
10. Resource status and RAM memory layout.
11. Scheduler context switch.
12. Minimize/resume process.
13. Kernel Mode authentication.
14. System log.
15. Deadlock detection message.
16. Shutdown cleanup.

## 17. Known Limits

- AMS OS is a simulator; it does not manage real hardware.
- The terminal UI is not a graphical desktop. The graphics library requirement
  is listed as a bonus in the manual and is not implemented.
- Process execution depends on Ubuntu/POSIX APIs and is not portable to
  non-Linux platforms without replacing the process model.
- Some terminal emulators handle process groups differently, so GNOME Terminal
  is recommended for the cleanest Ubuntu experience.

## 18. Submission Checklist

- [x] Complete source under `kernel/` and `tasks/`
- [x] 16 task executables defined in the task catalog
- [x] `Makefile` with build, run, install, clean, and uninstall targets
- [x] Professional `README.md`
- [x] Professional project report
- [x] Manual compliance matrix
- [x] System log support
- [x] Virtual disk support
- [x] Ubuntu-focused terminal launcher
- [x] No non-Linux source dependencies

## 19. Role-Based Responsibilities

If submitted as a group project, responsibilities can be mapped as follows:

| Role | Responsibility |
| --- | --- |
| Kernel developer | Boot flow, menu, fork/exec, IPC, process controls |
| Scheduler developer | Ready queues, FCFS, Round Robin, aging, context switch |
| Resource developer | RAM/HDD/CPU allocation and memory layout |
| Task developer | Independent task executables under `tasks/` |
| Documentation lead | README, report, screenshots, demo workflow |
| QA lead | Ubuntu build/run testing and manual compliance checks |
