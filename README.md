# Concurrent Data Communication in C

This project provides a structured framework for data communication and shared memory management in C, using mutex locks to ensure thread-safe operations. It includes essential components for sending and receiving data, managing shared memory, and synchronizing access, making it suitable for concurrent programming environments.

## Project Structure

- **`send.c` and `send.h`**: Implement functions for data transmission.
  - These files handle data preparation and dispatching, likely providing functions to send structured data safely.

- **`recv.c` and `recv.h`**: Define the data reception logic.
  - They include functions to receive, process, and handle incoming data, structured to integrate with concurrent or multi-threaded systems.

- **`read.c` and `read.h`**: Define auxiliary read functions.
  - They handle reading data from specific sources, potentially with error handling and validation for robustness.

- **`sharedMutex.c` and `sharedMutex.h`**: Manage shared resources using mutex locks.
  - These files encapsulate mutex operations, ensuring that shared memory or resources are accessed in a thread-safe manner.

- **`main.c`**: The main entry point for the program.
  - It initializes the program, invokes send/receive operations, and orchestrates the use of shared memory resources in a concurrent environment.

- **`Makefile`**: Automates the build process. Running `make` compiles all necessary files, links them, and creates the executable.

## Getting Started

### Prerequisites
- **C Compiler**: Ensure you have GCC or an equivalent compiler.
- **POSIX Thread Library** (if applicable): Required for mutex and thread handling.

### Compilation
In the project directory, use:
```bash
make
```
This will:
  - Compile `send.c`, `recv.c`, `read.c`, `sharedMutex.c`, and `main.c`.
  - Link all components to create the main executable.

### Running the Program
After compilation, execute the program with:
```bash
./main
```

## Usage

This project provides a foundational structure for data communication in concurrent applications. By using `sharedMutex` for thread-safe memory management, `send` and `recv` for data transmission, and `read` for handling input, it supports a wide range of applications that require safe, efficient data processing.

## Technologies Used

- **C Language**: Provides low-level control for concurrency and memory management.
- **POSIX Threads** (if applicable): Enables multi-threaded functionality.
- **Makefile**: Simplifies the build process by managing dependencies and compilation flags.

## Example Use Cases

This project can be used in systems where data needs to be safely sent and received across multiple threads or processes, such as in server-client applications, real-time data processing, or multi-threaded simulations.
