![hero](https://anime-hanabi.com/wp-content/uploads/2023/02/trap3.png?w=636)
# 🛜 Custom UDP Communication Protocol Implementation
> Project 1 IF2230 Computer Networks
## 💫 Overview

This repository contains a C/C++ implementation of a custom communication protocol that operates over UDP (User Datagram Protocol) while incorporating essential features of TCP (Transmission Control Protocol), such as:

- Reliable data transfer
- Error detection and correction
- Sequence numbering
- Acknowledgements
- Sliding window for flow control

The program is designed to showcase a hybrid protocol capable of ensuring reliable communication in an unreliable network environment, simulating the robustness of TCP while maintaining UDP's simplicity.


## 😎 Features

1. **Custom Protocol over UDP**  
   Implements a reliable, ordered, and error-checked data transfer protocol over UDP, including the core TCP features.

2. **Dual Functionality**  
   - **Text Mode**: Sends text messages from the client to the server.  
   - **File Mode**: Transfers raw binary files between the client and server.

3. **Interactive User Selection**  
   The program can switch roles (sender/receiver) and modes (text/file) dynamically based on user input.

4. **Verbose Logging**  
   Logs detailed operational messages for debugging and analysis, including:  
   - Current protocol status and TCP state  
   - SYN/ACK sequence numbers  
   - Sliding window activity (e.g., waiting for availability)  
   - Timeout or retransmission events  
   - Completed transmission statuses  

5. **Linux Compatibility**  
   Fully compatible with the Linux operating system and designed to operate seamlessly in a Linux network environment.



## Usage

Compile the program using a C/C++ compiler and run it from the command line. Below is an example of typical usage:

```
# For removing the compiled file
make clean

# For compiling and running the program
make run host=[DESIRED_IP] port=[DESIRED_PORT]
```

## Configuration

The program is tested under simulated poor network conditions to validate its reliability under packet loss, delays, and reordering. You can configure network behavior using tools such as `tc` (Traffic Control) to emulate such scenarios.



## Logging Output

The logs provide detailed insights into program execution, including:
- **Status Indicators**: Indicates ongoing and completed operations.
- **TCP State**: Displays the current state of the custom protocol.
- **SYN/ACK Numbers**: Tracks sequence numbers for handshake and data acknowledgment.
- **Sliding Window Events**: Shows queued, timed-out, or successfully transmitted packets.
- **Failures and Retransmissions**: Logs timeout events and retransmission attempts.
- **Completion Status**: Verifies the successful completion of data transfer.



## Requirements

- Makefile
- Linux operating system


## Notes

- External libraries are not used in this project. All functionality is implemented using built-in C/C++ libraries.  
- The program's design ensures modularity and extensibility, allowing easy adaptation for other networking scenarios.  
- The network simulation settings for evaluation are adjustable as needed.
