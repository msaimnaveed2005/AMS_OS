In Phase 3, AMS OS implements the core operating system simulation modules. 
The parent process acts as the OS kernel. When a user launches a task, the kernel creates a child process using fork(). 
Before execution, the child process sends an IPC resource request to the kernel through pipes. 
The kernel checks available RAM, hard drive space, and CPU core availability. 
If resources are available, the request is granted, a PCB is created, and the process is added to the ready queue. 
The child process then loads the task executable using exec().

Basic multitasking is simulated through a multi-level ready queue system. 
System tasks are given higher priority, interactive tasks use time quantum based execution, and background tasks run with lower priority. 
Context switching is simulated using SIGSTOP and SIGCONT signals. 
When a process completes, its allocated resources are released and returned to the resource pool.