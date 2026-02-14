# OS Process Scheduler

A comprehensive Operating System Process Scheduler simulation implementing multiple scheduling algorithms with memory management capabilities.

## 📋 Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Scheduling Algorithms](#scheduling-algorithms)
- [Memory Management](#memory-management)
- [Installation](#installation)
- [Usage](#usage)
- [Input Format](#input-format)
- [Output](#output)
- [Project Structure](#project-structure)

## 🎯 Overview

This project simulates an operating system process scheduler that manages process execution using different scheduling algorithms. It includes a process generator, scheduler, clock simulator, and memory management system using the Buddy allocation algorithm.

## ✨ Features

- **Multiple Scheduling Algorithms**: HPF (Highest Priority First), SRTN (Shortest Remaining Time Next), and RR (Round Robin)
- **Memory Management**: Buddy system memory allocation with 1024 bytes total memory
- **Process Control Block (PCB)**: Complete process state tracking
- **Inter-Process Communication**: Message queues and shared memory
- **Comprehensive Logging**: Detailed logs for scheduler events and memory allocation
- **Performance Metrics**: Calculation of CPU utilization, average waiting time, and weighted turnaround time

## 📊 Scheduling Algorithms

### 1. HPF (Highest Priority First)
- Non-preemptive priority-based scheduling
- Processes with lower priority number have higher priority
- Processes are executed based on their priority value

### 2. SRTN (Shortest Remaining Time Next)
- Preemptive scheduling algorithm
- Always executes the process with the shortest remaining time
- Dynamic process switching when a new process with shorter remaining time arrives

### 3. RR (Round Robin)
- Preemptive time-slice based scheduling
- Each process gets a fixed time quantum
- Processes are executed in circular queue fashion
- User-defined time quantum

## 💾 Memory Management

The scheduler implements a **Buddy System** memory allocation algorithm:
- Total memory: 1024 bytes
- Binary tree structure for memory blocks
- Dynamic memory allocation and deallocation
- Memory fragmentation handling
- Blocked process queue for processes waiting for memory

### Memory Allocation Process
1. Process requests memory based on its size
2. Memory manager searches for suitable block using buddy algorithm
3. If memory available, process is allocated and starts execution
4. If memory unavailable, process is placed in blocked queue
5. Upon process completion, memory is freed and merged with buddy blocks

## 🔧 Installation

### Prerequisites
- GCC compiler
- Linux/Unix environment (for IPC features)
- Make utility

### Build Instructions

```bash
# Build all components
make build

# Or build and clean in one command
make all

# For debug build
make debug

# Clean build artifacts
make clean
```

This will compile:
- `process_generator.out` - Generates and manages processes
- `clk.out` - Clock simulator for synchronization
- `scheduler.out` - Main scheduler implementation
- `process.out` - Individual process execution
- `test_generator.out` - Test case generator

## 🚀 Usage

### Step 1: Generate Test Processes (Optional)

```bash
./Compiled/test_generator.out
```
This will prompt you for the number of processes to generate and create a `processes.txt` file.

### Step 2: Run the Scheduler

```bash
make run
# Or directly:
./Compiled/process_generator.out
```

### Step 3: Select Scheduling Algorithm

When prompted, enter:
- `0` for HPF (Highest Priority First)
- `1` for SRTN (Shortest Remaining Time Next)
- `2` for RR (Round Robin) - will prompt for time quantum

## 📝 Input Format

The `processes.txt` file should follow this format:

```
#id arrival runtime priority memory
1    0    10    0    256
2    2    6     1    512
3    3    4     0    128
```

**Fields:**
- `id`: Unique process identifier
- `arrival`: Arrival time of the process
- `runtime`: Total execution time required
- `priority`: Priority level (lower number = higher priority)
- `memory`: Memory size required in bytes

## 📄 Output

The scheduler generates several output files:

### 1. Scheduler.log
Contains detailed process execution log:
```
At time X process Y started arr A total T remain R wait W
At time X process Y stopped arr A total T remain R wait W
At time X process Y resumed arr A total T remain R wait W
At time X process Y finished arr A total T remain R wait W TA X WTA X.XX
```

### 2. Scheduler.pref
Performance metrics:
- CPU utilization %
- Average Weighted Turnaround Time (Avg WTA)
- Average Waiting Time (Avg Waiting)
- Standard Deviation of WTA

### 3. memory.log
Memory allocation and deallocation events:
```
At time X allocated Y bytes for process Z from A to B
At time X freed Y bytes from process Z from A to B
```

## 📁 Project Structure

```
OS-Scheduler/
├── headers.h              # Common header file with data structures
├── process_generator.c    # Process generator and manager
├── scheduler.c           # Main scheduler implementation
├── process.c             # Individual process behavior
├── clk.c                 # Clock simulator
├── test_generator.c      # Test case generator
├── Makefile              # Build configuration
├── processes.txt         # Input file with process definitions
├── Compiled/             # Compiled binaries directory
├── Scheduler.log         # Scheduler execution log
├── Scheduler.pref        # Performance metrics
└── memory.log            # Memory allocation log
```

## 🔑 Key Components

### Process Generator
- Reads processes from input file
- Creates scheduler and clock processes
- Manages inter-process communication
- Sends processes to scheduler at their arrival time

### Scheduler
- Implements selected scheduling algorithm
- Manages ready queue and process states
- Handles memory allocation and deallocation
- Maintains blocked process queue
- Generates performance logs

### Clock
- Simulates system clock using shared memory
- Provides synchronization for all processes
- Increments time every second

### Process
- Represents individual process execution
- Handles signals for preemption
- Reports completion to scheduler

## 📊 Performance Metrics

The scheduler calculates and reports:

- **CPU Utilization**: `(Total Execution Time / Total Simulation Time) × 100%`
- **Turnaround Time (TA)**: `Finish Time - Arrival Time`
- **Weighted Turnaround Time (WTA)**: `Turnaround Time / Execution Time`
- **Average Waiting Time**: `Sum of Waiting Times / Number of Processes`
- **Average WTA**: `Sum of WTAs / Number of Processes`
- **Standard Deviation of WTA**: Variance in weighted turnaround times

## 🛠️ Technical Details

### Inter-Process Communication
- **Message Queues**: For sending process information
- **Shared Memory**: For clock synchronization
- **Signals**: For process control and synchronization
  - `SIGUSR1`: Process completion notification
  - `SIGCONT`: Process ready/resume signal
  - `SIGTSTP`: Process suspension

### Data Structures
- **Priority Queue**: For HPF scheduling
- **Min-Heap**: For SRTN scheduling
- **Circular Queue**: For RR scheduling
- **Binary Tree**: For buddy system memory management
- **Linked List**: For blocked processes

## 🤝 Contributing

Feel free to contribute to this project by:
1. Forking the repository
2. Creating a feature branch
3. Making your changes
4. Submitting a pull request

## 📄 License

This project is developed for educational purposes.
