# 🖥️ C Programming Exam - PG3401 (Spring 2026)
### 🏆 Grade: A

This repository contains my final exam submission for the C Programming course at Kristiania University College. The exam focused on low-level system programming and protocol implementation using the C-89 standard.

---

## 🛠️ Technical Focus
The project is centered around the following core areas:

*   **System Programming:** Utilization of Linux system calls and POSIX standards.
*   **File and Memory Management:** Manipulation of binary data files and dynamic memory allocation.
*   **Multithreading:** Implementation of thread safety using `mutex` to handle race conditions in mathematical operations.
*   **Networking:** Implementation of raw TCP communication based on the Socket API, along with the development of a custom protocol ("TCP over TCP") for reliable file transfer.

---

## 🧩 Key Challenges
The submission required problem-solving in the following areas:

*   **Protocol Implementation:** Development of a client-server architecture capable of handling data transfer with CRC checking and ACK/NACK logic.
*   **Robustness:** Implementation of error handling to ensure the server remains stable when receiving unexpected or malformed input.
*   **Efficiency:** Optimization of resource usage through the use of threads for time-consuming mathematical tests.

---

## 🚀 How to Run the Project
1. Ensure you have `gcc` and `make` installed on a Linux-based system.
2. Navigate to the desired task directory: `cd taskX_candidate_number`
3. Compile the code: `make`
4. Run the program: `./program_name`

---

*Developed as part of the Cyber Security Bachelor's degree at Kristiania University College.*