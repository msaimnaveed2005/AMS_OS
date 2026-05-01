# AMS OS Phase 3 Core Implementation

## Project Name

AMS OS, Atomic Management System

## Phase 3 Completed Modules

This phase implements the core operating system simulation modules.

### 1. Process Creation Using fork

AMS OS creates child processes using the fork system call.  
When the user selects a task, the kernel creates a new child process.

### 2. IPC Resource Request

The child process sends a resource request to the parent kernel using pipes.

The request contains:

- Process name
- Process type
- Priority
- RAM required
- Hard drive space required
- CPU cores required

### 3. Resource Management

The kernel checks available resources before allowing the task to run.

If resources are available:

- Request is granted
- Resources are allocated
- PCB is created
- Process is added to ready queue

If resources are not available:

- Request is denied
- Child process terminates immediately

### 4. exec Based Task Loading

After resource approval, the child process loads the selected task executable using exec.

Implemented task executables:

- Calculator
- Notepad
- Digital Clock
- Music Player
- File Copy

### 5. PCB Table

Each approved process gets a Process Control Block containing:

- PID
- Process name
- Process type
- Process state
- Priority
- RAM required
- HDD required
- CPU cores required
- Waiting time

### 6. Multilevel Ready Queue

AMS OS uses three ready queues:

| Queue | Process Type | Scheduling Style |
|---|---|---|
| System Queue | System and kernel tasks | FCFS |
| Interactive Queue | Interactive and auto-running tasks | Round Robin |
| Background Queue | Background tasks | Low priority scheduling |

### 7. Basic Scheduler

The scheduler selects processes from the ready queues.

Selection order:

1. System Queue
2. Interactive Queue
3. Background Queue

The scheduler uses SIGSTOP and SIGCONT to simulate context switching.

### 8. Resource Release

When a process completes, the kernel:

- Updates state to TERMINATED
- Releases RAM
- Releases HDD
- Releases CPU cores
- Removes PCB from process table

## How to Compile

```bash
make