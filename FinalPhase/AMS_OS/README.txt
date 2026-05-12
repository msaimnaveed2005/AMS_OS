AMS OS - Atomic Management System
==================================

A terminal-based Mini Operating System Simulator for the Operating Systems Lab
Spring 2026 final project. Demonstrates process creation (fork/exec), IPC
(pipes), resource allocation, multilevel scheduling (FCFS + Round Robin),
synchronization (semaphore, mutex, condition variable, threads), interrupt
handling, deadlock detection, and user/kernel mode separation.

Platform: Ubuntu 22.04+ with g++ (C++17), GNU Make, and GNOME Terminal.

Build and Run
-------------
    make check          # verify prerequisites
    make                # compile kernel + 16 task executables
    ./OS 2 256 8        # boot with 2 GB RAM, 256 GB HDD, 8 cores

Kernel Mode password: admin

See README.md for detailed usage, demo workflow, and project structure.
See PROJECT_REPORT.md for architecture documentation and compliance matrix.
