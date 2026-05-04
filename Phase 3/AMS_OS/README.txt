AMS OS - MiniOS X Final Project
===============================

Target Platform
---------------
Linux Xubuntu with g++, make, and xfce4-terminal installed.

Build and Run
-------------
1. Open terminal in the project folder.
2. Build everything:
   make
3. Start AMS OS with hardware resources:
   ./OS 2 256 8

Default kernel password:
admin

Final Project Feature Checklist
-------------------------------
- Named OS with boot and shutdown screens.
- Hardware resources supplied at startup: RAM, HDD, CPU cores.
- 16 separate task executables under tasks/.
- Fork/exec process creation for tasks.
- IPC pipe request from child to kernel for RAM/HDD/CPU approval.
- RAM block assignment and memory layout table.
- Resource allocation and release for RAM, HDD, and CPU cores.
- Xubuntu separate-terminal task execution by default.
- User mode and kernel mode.
- Close, minimize, resume, and switch process controls.
- PCB table with process state, type, priority, wait time, and RAM block.
- Ready queues with multilevel queue scheduling.
- FCFS for system queue, Round Robin for interactive queue, background queue for low priority tasks.
- Priority aging to reduce starvation.
- Synchronization through mutex, condition variable, semaphore, and monitor thread.
- Auto-start Digital Clock and Calendar after boot.
- System log file at data/system_log.txt.
- Deadlock detection simulation with circular wait message.
- Virtual disk file tasks under data/virtual_disk.

Recommended Demo Sequence
-------------------------
1. Run make.
2. Start ./OS 2 256 8.
3. Open Instruction Guide from menu option 24.
4. Show Task Catalog with option 1.
5. Launch Calculator or Notepad with option 3.
6. Run Scheduler with option 11 to dispatch queued tasks.
7. Show PCB Table, Ready Queues, Resources, and RAM Layout.
8. Minimize a process with option 17, then resume with option 18.
9. Switch to Kernel Mode using option 13 and password admin.
10. View System Log with option 15.
11. Run Deadlock Detection with option 19.
12. Close a process with option 21 or Kernel Kill with option 16.
13. Shutdown with option 0.
